#ifndef ROUTINGCONFIG_HPP
#define ROUTINGCONFIG_HPP

#include <set>
#include <fstream>
#include <iostream>
#include "Util/HttpMethod.hpp"
#include "Util/Span.hpp"
#include "Util/EnvironmentVariableDictionary.hpp"
#include "Config/ServerConfig.hpp"

class RoutingConfig
{
#ifdef TOOL
public:
#else
protected:
#endif
	std::string location;
	std::set<HttpMethod> methods;

	RoutingConfig();

public:
	virtual ~RoutingConfig();
	const std::string &get_location() const;
	const std::set<HttpMethod> &get_methods() const;

	virtual bool is_redirection_router() const = 0;
	virtual bool is_cgi_router() const = 0;
	virtual bool is_static_file_router() const = 0;

	virtual ConfigMatchingResult is_match(const Span &path, const HttpMethod &method) const = 0;

	virtual void validate() const = 0;

	virtual bool prepare(const EnvironmentVariableDictionary &dictionary) = 0;
#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

struct RoutingConfigRefLess
{
	bool operator()(const RoutingConfigRef &left, const RoutingConfigRef &right) const;
};

#endif