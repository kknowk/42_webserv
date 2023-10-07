#include "Config/ServerConfig.hpp"
#include "Config/Routing/RoutingConfig.hpp"
#include "Util/Span.hpp"
#include "Util/Uri.hpp"
#include "Util/IpAddress.hpp"
#include <stdexcept>
#include <new>
#include <vector>

ServerConfig::ServerConfig()
	: _is_ipv4(true), should_dispose_routings(false)
{
}

ServerConfig::~ServerConfig()
{
	if (this->should_dispose_routings)
	{
		for (std::vector<RoutingConfig *>::iterator itr = this->routings.begin(); itr != this->routings.end(); itr++)
		{
			RoutingConfig *pointer = *itr;
			delete pointer;
			*itr = NULL;
		}
	}
	this->routings.clear();
	this->should_dispose_routings = false;
}

bool ServerConfig::is_ipv4() const
{
	return this->_is_ipv4;
}

const Nullable<ErrorPage> &ServerConfig::get_default_error_page() const
{
	return this->default_error_page;
}

void ServerConfig::force_dispose()
{
	this->should_dispose_routings = true;
}

const std::string &ServerConfig::get_ip() const
{
	return this->ip;
}

const std::string &ServerConfig::get_port() const
{
	return this->port;
}

const std::set<std::string> &ServerConfig::get_server_names() const
{
	return this->server_names;
}

const std::map<HttpStatusCode, ErrorPage> &ServerConfig::get_error_pages() const
{
	return this->error_pages;
}

const std::vector<RoutingConfigRef> &ServerConfig::get_routings() const
{
	return this->routings;
}

void ServerConfig::validate() const
{
	{
		char *endptr;
		unsigned long long value = std::strtoull(this->port.c_str(), &endptr, 10);
		if (value == 0 || value >= 65536)
			throw std::runtime_error("port number is out of range(1-65535).");
		char tmp[256];
		std::snprintf(tmp, sizeof(tmp), "%llu", value);
		if (this->port != tmp)
			throw std::runtime_error("port number representation must be concrete.");
	}
	if (this->_is_ipv4)
	{
		if (!IpAddress::is_valid_v4(this->ip))
			throw std::runtime_error("ip address is not valid as IPv4.");
	}
	else
	{
		if (!IpAddress::is_valid_v6(this->ip))
			throw std::runtime_error("ip address is not valid as IPv6.");
	}
	if (this->port == "0")
		throw std::runtime_error("port number 0 is not allowed.");
	if (this->server_names.empty())
		throw std::runtime_error("server_names is empty.");
	if (this->routings.empty())
		throw std::runtime_error("routings is empty.");
	for (std::size_t index = 0; index < this->routings.size(); index++)
	{
		this->routings.at(index)->validate();
	}
}

const RoutingConfig &ServerConfig::find(const HttpMethod &method, const std::string &resource_location, ConfigMatchingResult &match_result, std::set<HttpMethod> &allowed) const
{
	Uri uri;
	match_result = ConfigMatchingResult_NotFound;
	allowed.clear();
	if (Uri::try_parse_start_from_path(resource_location, uri))
	{
		for (std::vector<RoutingConfigRef>::const_iterator itr = this->match_routings.begin(); itr != this->match_routings.end(); itr++)
		{
			const RoutingConfig &routing = *(*itr);
			ConfigMatchingResult result = routing.is_match(uri.get_path(), method);
			if (result == ConfigMatchingResult_Found)
			{
				match_result = ConfigMatchingResult_Found;
				return routing;
			}
			else if (result == ConfigMatchingResult_MethodNotMatch)
			{
				match_result = ConfigMatchingResult_MethodNotMatch;
				const std::set<HttpMethod> &set = routing.get_methods();
				allowed.insert(set.begin(), set.end());
			}
		}
	}
	return *(this->get_routings().at(0));
}

bool ServerConfig::prepare(const EnvironmentVariableDictionary &dictionary)
{
	for (std::vector<RoutingConfigRef>::iterator itr = this->routings.begin(); itr != this->routings.end(); itr++)
	{
		if (!(*itr)->prepare(dictionary))
			return false;
	}
	return true;
}
