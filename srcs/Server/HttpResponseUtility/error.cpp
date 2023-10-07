#include "Server/HttpResponseUtility.hpp"
#include "Config/Routing/RoutingConfig.hpp"
#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Config/Routing/StaticRoutingConfig.hpp"
#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include "Util/Span.hpp"
#include "Util/Logger.hpp"
#include <sstream>

Nullable<std::size_t> HttpResponseUtility::try_get_server_id(const Config &config, const std::string &host, const std::string &port, const HttpRequest &request)
{
	if (request.is_parse_success())
	{
		const HttpRequestHeader::HostHeader &header = request.get_header().get_host();
		if (header.is_valid())
			return config.find_server_id(host, port, header.get_host());
		else
			return config.find_server_id(host, port);
	}
	else
		return config.find_server_id(host, port);
}

void HttpResponseUtility::from_error_status_code(const HttpResponseUtility::ErrorReferences &references, int status_code)
{
	Connection &connection = references.connection;
	HttpResponse &response = connection.get_response();
	response.set_status_code(HttpStatusCode(status_code));
	response.get_status().clear();
	EventHandler &handler = references.handler;
	const ListenerSocket &socket = handler.get_listener_by_socket_fd(connection.get_base_socket_fd());
	const Config &config = handler.get_config();
	Nullable<std::size_t> server_id = HttpResponseUtility::try_get_server_id(config, socket.get_ip(), socket.get_port(), connection.get_request());
	std::map<std::string, std::string> &headers = response.get_header();
	bool process_error = !server_id.has_value;
	if (server_id.has_value)
	{
		try
		{
			const std::vector<char> &found = config.get_error_page_content(response.get_status_code().value, server_id.value, headers["content-type"]);
			response.add_content(found.begin(), found.end());
		}
		catch (const std::exception &e)
		{
			process_error = true;
		}
	}
	if (process_error)
	{
		const std::string &resource_path = references.connection.get_request().get_header().get_normalized_resource_location();
		std::vector<char> found = config.get_default_error_page_content(resource_path, HttpStatusCode(status_code));
		response.add_content(found.begin(), found.end());
		headers["content-type"] = "text/html";
	}
	switch (status_code)
	{
	case 408:
		headers.operator[]("connection") = "close";
		break;
	}
	handler.remove_read_resource(connection);
	handler.remove_write_resource(connection);
	references.start_response_sending();
}

void HttpResponseUtility::from_error_status_code(EventHandler *handler, Connection &connection, int status_code)
{
	if (handler == NULL)
		return;
	HttpResponseUtility::ErrorReferences refs(*handler, connection);
	HttpResponseUtility::from_error_status_code(refs, status_code);
}

static std::string to_string(const std::set<HttpMethod> &allowed_method)
{
	std::ostringstream stream;
	for (std::set<HttpMethod>::const_iterator itr = allowed_method.begin(), start = allowed_method.begin(), end = allowed_method.end(); itr != end; itr++)
	{
		if (itr != start)
			stream << ", ";
		stream << *itr;
	}
	return stream.str();
}

void HttpResponseUtility::from_error_405(const ErrorReferences &references, const std::set<HttpMethod> &allowed_method)
{
	Connection &connection = references.connection;
	HttpResponse &response = connection.get_response();
	response.set_status_code(HttpStatusCode(405));
	response.get_status().clear();
	EventHandler &handler = references.handler;
	const ListenerSocket &socket = handler.get_listener_by_socket_fd(connection.get_base_socket_fd());
	const Config &config = handler.get_config();
	Nullable<std::size_t> server_id = HttpResponseUtility::try_get_server_id(config, socket.get_ip(), socket.get_port(), connection.get_request());
	std::map<std::string, std::string> &headers = response.get_header();
	bool process_error = !server_id.has_value;
	if (!process_error)
	{
		try
		{
			const std::vector<char> &found = config.get_error_page_content(405, server_id.value, headers["content-type"]);
			response.add_content(found.begin(), found.end());
		}
		catch (const std::exception &e)
		{
			process_error = true;
		}
	}
	if (process_error)
	{
		const std::string &resource_path = references.connection.get_request().get_header().get_normalized_resource_location();
		std::vector<char> found = config.get_default_error_page_content(resource_path, HttpStatusCode(405));
		response.add_content(found.begin(), found.end());
		headers["content-type"] = "text/html";
	}
	headers.operator[]("allow") = to_string(allowed_method);
	handler.remove_read_resource(connection);
	handler.remove_write_resource(connection);
	references.start_response_sending();
}
