#include "Server/HttpResponseUtility.hpp"
#include <sys/types.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>
#include <cstdio>
#include <sstream>

static void get_from_config_static_file(const StaticRoutingConfig &routing, const std::vector<char> &path, const HttpResponseUtility::References &references)
{
	int resource_fd = ::open(path.data(), O_RDONLY | O_NONBLOCK | O_CLOEXEC);
	if (resource_fd < 0)
	{
		int error_code = errno;
		switch (error_code)
		{
		case EACCES:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 403);
			break;
		case ENOENT:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 404);
			break;
		default:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
			break;
		}
		return;
	}
	references.handler.add_read_resource_file(resource_fd, references.connection);
	std::vector<char>::const_iterator end = path.end() - 1; // additional '\0' removal
	const std::string &found_media_type = routing.get_dictionary().find_by_path(path.begin(), end);
	references.response.register_header("media-type", found_media_type);
	references.response.set_status_code(200);
	references.response.make_chunked();
	references.start_response_sending();
}

void HttpResponseUtility::get_from_config_enumerate_directory(const std::vector<char> &path, const HttpResponseUtility::References &references)
{
	DIR *directory = ::opendir(path.data());
	if (directory == NULL)
	{
		switch (errno)
		{
		case EACCES:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 403);
			return;
		case ENOENT:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 404);
			return;
		default:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 500);
			return;
		}
	}
	references.response.get_content() = std::vector<char>();
	std::vector<char> &buffer = references.response.get_content().value;
	StringHelper::append(buffer, "<!DOCTYPE html>\n<html>\n");
	StringHelper::append(buffer, "<head>\n<title>Directory</title>\n</head>\n<body>\n<ul>\n");
	for (dirent *entry = ::readdir(directory); entry != NULL; entry = ::readdir(directory))
	{
		StringHelper::append(buffer, "  <li><a href=\"./");
		StringHelper::append_html(buffer, entry->d_name);
		if (entry->d_type == DT_DIR)
			buffer.push_back('/');
		StringHelper::append(buffer, "\">");
		StringHelper::append(buffer, entry->d_name);
		if (entry->d_type == DT_DIR)
			buffer.push_back('/');
		StringHelper::append(buffer, "</a></li>\n");
	}
	StringHelper::append(buffer, "</ul>\n</body>\n</html>");
	::closedir(directory);
	references.response.register_header("media-type", "text/html");
	references.response.set_status_code(200);
	references.start_response_sending();
}

static void delete_from_config_static_file(const std::vector<char> &path, const HttpResponseUtility::References &references)
{
	if (std::remove(path.data()))
	{
		switch (errno)
		{
		case ENOENT:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 404);
			return;
		default:
			HttpResponseUtility::from_error_status_code(references.to_Error(), 403);
			return;
		}
	}
	references.response.set_status_code(204);
	references.start_response_sending();
}

static void create_from_config_static_file(const std::vector<char> &path, const std::string &original_path, const HttpResponseUtility::References &references)
{
	int write_fd = ::open(path.data(), O_WRONLY | O_CLOEXEC | O_NONBLOCK | O_CREAT | O_TRUNC, S_IRUSR | S_IWUSR);
	if (write_fd < 0)
	{
		int status_code;
		switch (errno)
		{
		case EACCES:
			status_code = 409;
			break;
		default:
			status_code = 500;
			break;
		}
		HttpResponseUtility::from_error_status_code(references.to_Error(), status_code);
		return;
	}
	references.handler.add_write_resource_file(write_fd, references.connection);
	references.response.set_status_code(201);
	std::string location;
	{
		std::ostringstream stream;
		stream << "http://" << references.request.get_header().get_host() << original_path;
		location = stream.str();
	}
	references.response.register_header("location", location);
	references.response.register_header("content-type", references.request.get_header().get_content_type());
	references.connection.wait_for_resource_completion();
}

static void to_c_str(std::vector<char> &buffer)
{
	Uri::decode_path(buffer);
	if (buffer.empty() || buffer.back() != '\0')
		buffer.push_back('\0');
}

void HttpResponseUtility::from_static_config(const StaticRoutingConfig &routing, const HttpResponseUtility::References &references)
{
	const std::string &path = references.request.get_header().get_normalized_resource_location();
	Span span(path);
	std::vector<char> routed_path = routing.get_routed_path(span);
	switch (references.request.get_header().get_method())
	{
	case HttpMethod_GET:
		if (routed_path.back() == '/')
		{
			if (routing.get_either_index_or_listing().is_item0)
			{
				const std::string &append_path = routing.get_either_index_or_listing().item0;
				routed_path.insert(routed_path.end(), append_path.begin(), append_path.end());
				to_c_str(routed_path);
				get_from_config_static_file(routing, routed_path, references);
			}
			else
			{
				to_c_str(routed_path);
				HttpResponseUtility::get_from_config_enumerate_directory(routed_path, references);
			}
		}
		else
		{
			to_c_str(routed_path);
			get_from_config_static_file(routing, routed_path, references);
		}
		break;
	case HttpMethod_DELETE:
	{
		if (routed_path.back() == '/')
			HttpResponseUtility::from_error_status_code(references.to_Error(), 403);
		else
		{
			to_c_str(routed_path);
			delete_from_config_static_file(routed_path, references);
		}
	}
	break;
	case HttpMethod_POST:
	{
		if (routed_path.back() == '/')
		{
			if (routing.get_either_index_or_listing().is_item0)
			{
				const std::string &append_path = routing.get_either_index_or_listing().item0;
				routed_path.insert(routed_path.end(), append_path.begin(), append_path.end());
				to_c_str(routed_path);
				create_from_config_static_file(routed_path, path, references);
			}
			else
				HttpResponseUtility::from_error_status_code(references.to_Error(), 403);
		}
		else
		{
			to_c_str(routed_path);
			create_from_config_static_file(routed_path, path, references);
		}
	}
	break;
	default:
		HttpResponseUtility::from_error_405(references.to_Error(), routing.get_methods());
		break;
	}
}
