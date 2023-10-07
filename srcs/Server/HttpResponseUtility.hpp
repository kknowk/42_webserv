#ifndef HTTPRESPONSEUTILITY_HPP
#define HTTPRESPONSEUTILITY_HPP

#include "Config/Config.hpp"
#include "Config/Routing/StaticRoutingConfig.hpp"
#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Server/HttpRequest.hpp"
#include "Server/HttpResponse.hpp"
#include "Util/EnvironmentVariableDictionary.hpp"
#include "Server/EventHandler.hpp"

class HttpResponseUtility
{
private:
	HttpResponseUtility();

	static Nullable<std::size_t> try_get_server_id(const Config &config, const std::string &host, const std::string &port, const HttpRequest &request);

public:
	struct ErrorReferences;

	struct References
	{
	public:
		HttpResponse &response;
		EventHandler &handler;
		Connection &connection;
		const HttpRequest &request;
		const Config &config;
		const ServerConfig &server;
		const EnvironmentVariableDictionary &dictionary;

		References(EventHandler &handler, Connection &connection, const ServerConfig &server);
		ErrorReferences to_Error() const;

		void start_response_sending() const;
	};

	struct ErrorReferences
	{
	public:
		EventHandler &handler;
		Connection &connection;

		ErrorReferences(EventHandler &handler, Connection &connection);
		void start_response_sending() const;
	};

	static void from_error_status_code(const ErrorReferences &references, int status_code);
	static void from_error_status_code(EventHandler *handler, Connection &connection, int status_code);
	static void from_error_405(const ErrorReferences &references, const std::set<HttpMethod> &allowed_method);
	static void from_config(EventHandler &handler, Connection &connection, const ListenerSocket &listener);

	static void from_static_config(const StaticRoutingConfig &routing, const References &references);
	static void from_redirection_config(const RedirectionRoutingConfig &routing, const References &references);
	static void from_cgi_config(const CgiRoutingConfig &routing, const References &references);

	static void from_cgi_response(EventHandler &handler, Connection &connection);

	static void get_from_config_enumerate_directory(const std::vector<char> &path, const References &references);
};

#endif