#ifndef SERVERCONFIG_HPP
#define SERVERCONFIG_HPP

#include <vector>
#include <set>
#include <map>
#include <iterator>
#include "Config/Config.hpp"
#include "Util/EnvironmentVariableDictionary.hpp"
#include "Util/Uri.hpp"
#include "Util/HttpStatusCode.hpp"
#include "Util/TimeSpan.hpp"
#include "Util/Either.hpp"

class RoutingConfig;

typedef RoutingConfig *RoutingConfigRef;

enum ConfigMatchingResult
{
	ConfigMatchingResult_Found,
	ConfigMatchingResult_MethodNotMatch,
	ConfigMatchingResult_NotFound,
};

class ServerConfig
{
#ifdef TOOL
public:
#else
private:
#endif
	bool _is_ipv4;
	std::string ip;
	/// @brief no preceding :
	std::string port;
	std::set<std::string> server_names;
	std::map<HttpStatusCode, ErrorPage> error_pages;
	Nullable<ErrorPage> default_error_page;
	std::vector<RoutingConfigRef> routings;

	std::vector<RoutingConfigRef> match_routings;
	bool should_dispose_routings;

public:
	ServerConfig();
	~ServerConfig();

	void force_dispose();
	bool is_ipv4() const;
	const std::string &get_ip() const;
	const std::string &get_port() const;
	const Nullable<ErrorPage> &get_default_error_page() const;
	const std::set<std::string> &get_server_names() const;
	const std::map<HttpStatusCode, ErrorPage> &get_error_pages() const;
	const std::vector<RoutingConfigRef> &get_routings() const;

	const RoutingConfig &find(const HttpMethod &method, const std::string &resource_location, ConfigMatchingResult &success, std::set<HttpMethod> &allowed) const;

	bool prepare(const EnvironmentVariableDictionary &dictionary);

	void validate() const;
#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

#endif