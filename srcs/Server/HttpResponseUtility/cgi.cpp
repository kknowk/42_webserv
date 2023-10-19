#include "Server/HttpResponseUtility.hpp"
#include "Util/FileUtility.hpp"
#include "Util/Logger.hpp"
#include "Util/Span.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <cstring>

static void close_many(int *fds, std::size_t count)
{
	for (std::size_t index = 0; index < count; index++)
	{
		::close(fds[index]);
	}
}

static void insert_http_header_dictionary_element(EnvironmentVariableDictionary &dictionary, const std::pair<std::string, std::vector<std::string> > &pair, std::vector<char> &buffer)
{
	buffer.clear();
	buffer.insert(buffer.end(), "HTTP_", "HTTP_" + 5);
	for (std::string::const_iterator itr = pair.first.begin(); itr != pair.first.end(); itr++)
	{
		if (*itr == '-')
			buffer.push_back('_');
		else
			buffer.push_back(std::toupper(*itr));
	}
	std::string key(buffer.begin(), buffer.end());
	buffer.clear();
	for (std::vector<std::string>::const_iterator itr = pair.second.begin(); itr != pair.second.end(); itr++)
	{
		if (!buffer.empty())
			buffer.push_back(',');
		buffer.insert(buffer.end(), itr->begin(), itr->end());
	}
	std::string value(buffer.begin(), buffer.end());
	dictionary.operator[](key).operator=(value);
}

static void create_environment_variable_dictionary(EnvironmentVariableDictionary &dictionary, const CgiRoutingConfig &config, const HttpResponseUtility::References &references, HttpMethod method)
{
	static std::string empty;
	dictionary.operator[]("AUTH_TYPE").operator=(empty);
	{
		std::size_t content_length = references.request.get_data().size();
		char text[32];
		std::snprintf(text, sizeof(text), "%zu", content_length);
		dictionary.operator[]("CONTENT_LENGTH").operator=(text);
	}
	const HttpRequestHeader &header = references.request.get_header();
	switch (method)
	{
	case HttpMethod_GET:
		dictionary.operator[]("REQUEST_METHOD").operator=("GET");
		dictionary.operator[]("CONTENT_TYPE").operator=(empty);
		break;
	case HttpMethod_POST:
	{
		dictionary.operator[]("REQUEST_METHOD").operator=("POST");
		std::map<std::string, std::vector<std::string> >::const_iterator found = header.at("content-type");
		if (found == header.end() || found->second.empty())
			dictionary.operator[]("CONTENT_TYPE").operator=(empty);
		else
			dictionary.operator[]("CONTENT_TYPE").operator=(found->second.at(0));
	}
	break;
	default:
		throw std::out_of_range("HTTP method is out of range.");
	}
	dictionary.operator[]("GATEWAY_INTERFACE").operator=("CGI/1.1");
	std::string path_info = config.get_path_info(Span(header.get_normalized_resource_location())).to_string();
	dictionary.operator[]("PATH_INFO").operator=(path_info);
	if (path_info.empty())
		dictionary.operator[]("PATH_TRANSLATED").operator=(empty);
	else
		dictionary.operator[]("PATH_TRANSLATED").operator=(FileUtility::path_combine(config.get_root_directory(), path_info));
	const Span &parameter = header.get_resource_location().get_parameter();
	if (parameter.size() == 0)
		dictionary.operator[]("QUERY_STRING").operator=(empty);
	else
	{
		// https://www2k.biglobe.ne.jp/~motoi/cdecgi/cdecgi02.html
		// Query String decoding should be done in CGI process.
		dictionary.operator[]("QUERY_STRING").operator=(parameter.slice(1).to_string());
	}
	dictionary.operator[]("REMOTE_ADDR").operator=(references.connection.get_counterpart_ip());
	dictionary.operator[]("REMOTE_HOST").operator=(empty);
	dictionary.operator[]("REMOTE_IDENT").operator=(empty);
	dictionary.operator[]("REMOTE_USER").operator=(empty);
	dictionary.operator[]("SCRIPT_NAME").operator=(config.get_script_name(header.get_normalized_resource_location()).to_string());
	dictionary.operator[]("SERVER_NAME").operator=(header.get_host().get_host());
	if (header.get_host().get_port().has_value)
		dictionary.operator[]("SERVER_PORT").operator=(header.get_host().get_port().value);
	else
		dictionary.operator[]("SERVER_PORT").operator=("80");
	dictionary.operator[]("SERVER_PROTOCOL").operator=("HTTP/1.1");
	dictionary.operator[]("SERVER_SOFTWARE").operator=("webserv");
	std::vector<char> buffer;
	for (std::map<std::string, std::vector<std::string> >::const_iterator itr = header.begin(); itr != header.end(); itr++)
	{
		insert_http_header_dictionary_element(dictionary, *itr, buffer);
	}
}

static void from_cgi_config_get(const CgiRoutingConfig &routing, const HttpResponseUtility::References &references, char *const *argv)
{
	int pipefds[2];
	if (::pipe(pipefds))
	{
		Logger::log_error("from_cgi_config pipe() failed.", errno);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	// 0: parent's read_end (CLOSE_ON_EXEC)
	if (!FileUtility::add_close_on_exec_and_nonblock_flag(pipefds[0]))
	{
		close_many(pipefds, 2);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	// 1: child's write_end (close after fork on the parent's side)
	if (!FileUtility::add_nonblock_flag(pipefds[1]))
	{
		close_many(pipefds, 2);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	EnvironmentVariableDictionary dictionary(references.dictionary);
	create_environment_variable_dictionary(dictionary, routing, references, HttpMethod_POST);
	EnvironmentVariableDictionaryCharArray env_vector = dictionary.to_string_vector();
	env_vector.force_dispose();
	char **envp = env_vector.data();
	const char *current_directory = routing.get_root_directory().c_str();
	pid_t process_id = ::fork();
	if (process_id < 0)
	{
		close_many(pipefds, 2);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	if (process_id == 0)
	{
		if (::chdir(current_directory) == 0 && ::dup2(pipefds[1], STDOUT_FILENO) >= 0)
		{
			::close(pipefds[1]);
			::execve(argv[0], argv, envp);
		}
		std::exit(1);
	}
	::close(pipefds[1]);
	references.handler.add_read_resource_pipe(pipefds[0], references.connection);
	references.connection.wait_for_resource_completion();
}

static void from_cgi_config_post(const CgiRoutingConfig &routing, const HttpResponseUtility::References &references, char *const *argv)
{
	int pipefds[4];
	if (::pipe(pipefds))
	{
		Logger::log_error("from_cgi_config pipe() failed.", errno);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	// 0: parent's read_end (CLOSE_ON_EXEC)
	if (!FileUtility::add_close_on_exec_and_nonblock_flag(pipefds[0]))
	{
		close_many(pipefds, 2);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	// 1: child's write_end (close after fork on the parent's side)
	if (!FileUtility::add_nonblock_flag(pipefds[1]))
	{
		close_many(pipefds, 2);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	if (::pipe(pipefds + 2))
	{
		Logger::log_error("from_cgi_config pipe() failed.", errno);
		close_many(pipefds, 2);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	// 2: child's read_end (close after fork on the parent's side)
	if (!FileUtility::add_nonblock_flag(pipefds[2]))
	{
		close_many(pipefds, 4);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	// 3: parent's write_end (CLOSE_ON_EXEC)
	if (!FileUtility::add_close_on_exec_and_nonblock_flag(pipefds[3]))
	{
		close_many(pipefds, 4);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	EnvironmentVariableDictionary dictionary(references.dictionary);
	create_environment_variable_dictionary(dictionary, routing, references, HttpMethod_POST);
	EnvironmentVariableDictionaryCharArray env_vector = dictionary.to_string_vector();
	env_vector.force_dispose();
	char **envp = env_vector.data();
	const char *current_directory = routing.get_root_directory().c_str();
	pid_t process_id = ::fork();
	if (process_id < 0)
	{
		close_many(pipefds, 4);
		HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	if (process_id == 0)
	{
		if (::chdir(current_directory) == 0 && ::dup2(pipefds[1], STDOUT_FILENO) >= 0 && ::dup2(pipefds[2], STDIN_FILENO) >= 0)
		{
			::close_many(pipefds + 1, 2);
			::execve(argv[0], argv, envp);
		}
		std::exit(1);
	}
	close_many(pipefds + 1, 2);
	references.handler.add_read_resource_pipe(pipefds[0], references.connection);
	references.handler.add_write_resource_pipe(pipefds[3], references.connection);
	references.connection.wait_for_resource_completion();
}

void HttpResponseUtility::from_cgi_config(const CgiRoutingConfig &routing, const HttpResponseUtility::References &references)
{
	Span location(references.request.get_header().get_normalized_resource_location());
	std::vector<char> target_file_path = routing.get_routed_path(location);
	if (target_file_path.back() == '/')
	{
		const Either<std::string, bool> &either = routing.get_either_index_or_listing();
		if (either.is_item0)
			FileUtility::path_combine(target_file_path, either.item0);
		else if (either.item1)
		{
			Uri::decode_path(target_file_path);
			target_file_path.push_back('\0');
			HttpResponseUtility::get_from_config_enumerate_directory(target_file_path, references);
			return;
		}
		else
		{
			HttpResponseUtility::from_error_status_code(references.to_Error(), 400);
			return;
		}
	}
	Uri::decode_path(target_file_path);
	target_file_path.push_back('\0');
	if (::access(target_file_path.data(), routing.handle_single_executable() ? X_OK : R_OK))
	{
		int error_code = errno;
		if (error_code == ENOENT)
			HttpResponseUtility::from_error_status_code(references.to_Error(), 404);
		else if (error_code == EACCES)
			HttpResponseUtility::from_error_status_code(references.to_Error(), 403);
		else
			HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
		return;
	}
	char *argv[3];
	std::string file_name = routing.get_file_name(location).to_string();
	if (routing.handle_single_executable())
	{
		argv[0] = StringHelper::strdup(file_name.c_str());
		argv[1] = NULL;
	}
	else
	{
		argv[0] = StringHelper::strdup(routing.get_executor().c_str());
		argv[1] = StringHelper::strdup(file_name.c_str());
	}
	argv[2] = NULL;
	switch (references.request.get_header().get_method())
	{
	case HttpMethod_GET:
		from_cgi_config_get(routing, references, argv);
		break;
	case HttpMethod_POST:
		from_cgi_config_post(routing, references, argv);
		break;
	default:
		std::set<HttpMethod> set;
		set.insert(HttpMethod_GET);
		set.insert(HttpMethod_POST);
		HttpResponseUtility::from_error_405(references.to_Error(), set);
		break;
	}
	delete[] argv[0];
	delete[] argv[1];
}
