#define TOOL
#include "Config/Config.hpp"
#include "Config/Routing/StaticRoutingConfig.hpp"
#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include "Util/StreamWriter.hpp"
#include <iostream>
#include <fstream>
#include <ios>

int main(int argc, char **argv, char **envp)
{
	if (argc != 2)
		return 1;

	CgiRoutingConfig *cgi = new CgiRoutingConfig;
	cgi->location = "/cgi/";
	cgi->executor = "python3";
	cgi->either_index_or_listing.is_item0 = false;
	cgi->either_index_or_listing.item1 = false;
	cgi->location_suffix = ".py";
	cgi->root_directory = "./cgi/";
	cgi->methods.insert(HttpMethod_GET);
	cgi->methods.insert(HttpMethod_POST);

	RedirectionRoutingConfig *redirection301 = new RedirectionRoutingConfig;
	redirection301->methods.insert(HttpMethod_GET);
	redirection301->location = "/redirect/";
	redirection301->redirection.code = HttpStatusCode(301);
	redirection301->redirection.converter_add("http://127.0.0.1");
	redirection301->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_PortWithColon);
	redirection301->redirection.converter_add("/");
	redirection301->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_PathWithoutLocation);
	redirection301->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_HasParameter);
	redirection301->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_Parameter);
	redirection301->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_HasFragment);
	redirection301->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_Fragment);

	RedirectionRoutingConfig *redirection308 = new RedirectionRoutingConfig(*redirection301);
	redirection308->methods.clear();
	redirection308->methods.insert(HttpMethod_POST);
	redirection308->redirection.code = HttpStatusCode(308);

	StaticRoutingConfig *file_tools = new StaticRoutingConfig;
	file_tools->location = "/";
	file_tools->root_directory = "./tools/";
	file_tools->either_index_or_listing.is_item0 = true;
	file_tools->either_index_or_listing.item0 = "index.html";
	file_tools->methods.insert(HttpMethod_GET);
	file_tools->methods.insert(HttpMethod_POST);
	file_tools->dictionary = MediaTypeDictionary::create_default_dictionary();

	StaticRoutingConfig *file_image = new StaticRoutingConfig;
	file_image->location = "/image/";
	file_image->root_directory = "./images/";
	file_image->either_index_or_listing.is_item0 = false;
	file_image->either_index_or_listing.item1 = true;
	file_image->methods.insert(HttpMethod_GET);
	file_image->methods.insert(HttpMethod_POST);
	file_image->methods.insert(HttpMethod_DELETE);
	file_image->dictionary = MediaTypeDictionary::create_default_dictionary();

	ServerConfig serverConfig_127;
	serverConfig_127.routings.push_back(static_cast<RoutingConfig *>(redirection301));
	serverConfig_127.routings.push_back(static_cast<RoutingConfig *>(redirection308));
	serverConfig_127.routings.push_back(static_cast<RoutingConfig *>(file_tools));
	serverConfig_127.routings.push_back(static_cast<RoutingConfig *>(file_image));
	serverConfig_127.routings.push_back(static_cast<RoutingConfig *>(cgi));
	serverConfig_127.port = "8000";
	serverConfig_127.ip = "127.0.0.1";
	serverConfig_127.server_names.insert("127.0.0.1");
	serverConfig_127.error_pages.insert(std::pair<HttpStatusCode, ErrorPage>(HttpStatusCode(404), ErrorPage("./tools/404.html", "text/html")));

	RedirectionRoutingConfig *redirection301x = new RedirectionRoutingConfig;
	redirection301x->methods.insert(HttpMethod_GET);
	redirection301x->location = "/magic/";
	redirection301x->redirection.code = HttpStatusCode(301);
	redirection301x->redirection.converter_add("http://localhost");
	redirection301x->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_PortWithColon);
	redirection301x->redirection.converter_add("/");
	redirection301x->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_PathWithoutLocation);
	redirection301x->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_HasParameter);
	redirection301x->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_Parameter);
	redirection301x->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_HasFragment);
	redirection301x->redirection.converter_add(HttpRedirection::HttpRedirection_Variable_Fragment);

	RedirectionRoutingConfig *redirection308x = new RedirectionRoutingConfig(*redirection301x);
	redirection308x->methods.clear();
	redirection308x->methods.insert(HttpMethod_POST);
	redirection308x->redirection.code = HttpStatusCode(308);

	StaticRoutingConfig *file_toolsx = new StaticRoutingConfig(*file_tools);
	StaticRoutingConfig *file_imagex = new StaticRoutingConfig(*file_image);

	CgiRoutingConfig *cgix = new CgiRoutingConfig;
	cgix->executor.has_value = false;
	cgix->location = "/cgix/";
	cgix->root_directory = "./cgi/";
	cgix->methods.insert(HttpMethod_GET);
	cgix->either_index_or_listing.is_item0 = false;
	cgix->either_index_or_listing.item1 = true;

	ServerConfig serverConfig_local;
	serverConfig_local.routings.push_back(static_cast<RoutingConfig *>(redirection301x));
	serverConfig_local.routings.push_back(static_cast<RoutingConfig *>(redirection308x));
	serverConfig_local.routings.push_back(static_cast<RoutingConfig *>(file_toolsx));
	serverConfig_local.routings.push_back(static_cast<RoutingConfig *>(file_imagex));
	serverConfig_local.routings.push_back(static_cast<RoutingConfig *>(cgix));
	serverConfig_local.port = "8001";
	serverConfig_local.ip = "127.0.0.1";
	serverConfig_local.server_names.insert("localhost");
	serverConfig_local.error_pages.insert(std::pair<HttpStatusCode, ErrorPage>(HttpStatusCode(404), ErrorPage("./tools/404.html", "text/html")));

	EnvironmentVariableDictionary dictionary(envp);
	Config config(dictionary);
	config.servers.push_back(serverConfig_127);
	config.servers.push_back(serverConfig_local);
	config.make_servers_disposable();
	config.wait_time = TimeSpan::from_seconds(120);
	config.keep_alive = config.wait_time;
	config.request_body_size_limit = 1000000;

	std::ofstream stream(argv[1], std::ios::binary | std::ios::out | std::ios::trunc);
	StreamWriter::write(stream, config);
	return 0;
}
