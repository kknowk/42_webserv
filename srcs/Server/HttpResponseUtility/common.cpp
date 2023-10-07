#include "Server/HttpResponseUtility.hpp"
#include "Config/Routing/RoutingConfig.hpp"
#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Config/Routing/StaticRoutingConfig.hpp"
#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include "Util/Span.hpp"
#include "Util/Logger.hpp"

HttpResponseUtility::HttpResponseUtility()
{
}

void HttpResponseUtility::from_config(EventHandler &handler, Connection &connection, const ListenerSocket &listener)
{
	const HttpRequest &request = connection.get_request();
	Nullable<std::size_t> server_id = HttpResponseUtility::try_get_server_id(handler.get_config(), listener.get_ip(), listener.get_port(), request);
	if (!server_id.has_value)
	{
		HttpResponseUtility::ErrorReferences refs(handler, connection);
		HttpResponseUtility::from_error_status_code(refs, 400);
		return;
	}
	const HttpRequestHeader &request_header = request.get_header();
	const ServerConfig &server = handler.get_config().operator[](server_id.value);
	ConfigMatchingResult match_result;
	std::set<HttpMethod> allowed_methods;
	const RoutingConfig &routing = server.find(request_header.get_method(), request_header.get_normalized_resource_location(), match_result, allowed_methods);
	HttpResponseUtility::References references(handler, connection, server);
	if (match_result == ConfigMatchingResult_NotFound)
		HttpResponseUtility::from_error_status_code(references.to_Error(), 404);
	else if (match_result == ConfigMatchingResult_MethodNotMatch)
		HttpResponseUtility::from_error_405(references.to_Error(), allowed_methods);
	else if (routing.is_cgi_router())
		HttpResponseUtility::from_cgi_config(dynamic_cast<const CgiRoutingConfig &>(routing), references);
	else if (routing.is_redirection_router())
		HttpResponseUtility::from_redirection_config(dynamic_cast<const RedirectionRoutingConfig &>(routing), references);
	else if (routing.is_static_file_router())
		HttpResponseUtility::from_static_config(dynamic_cast<const StaticRoutingConfig &>(routing), references);
	else
		throw std::runtime_error("routing is invalid.");
}

HttpResponseUtility::References::References(EventHandler &handler, Connection &connection, const ServerConfig &server)
	: response(connection.get_response()), handler(handler), connection(connection), request(connection.get_request()), config(handler.get_config()), server(server), dictionary(handler.get_dictionary())
{
}

void HttpResponseUtility::References::start_response_sending() const
{
	this->response.construction_complete();
	this->connection.start_response_sending();
}

HttpResponseUtility::ErrorReferences HttpResponseUtility::References::to_Error() const
{
	return HttpResponseUtility::ErrorReferences(this->handler, this->connection);
}

HttpResponseUtility::ErrorReferences::ErrorReferences(EventHandler &handler, Connection &connection)
	: handler(handler), connection(connection)
{
}

void HttpResponseUtility::ErrorReferences::start_response_sending() const
{
	this->connection.get_response().construction_complete();
	this->connection.start_response_sending();
}
