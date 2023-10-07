#ifndef RedirectionROUTINGCONFIG_HPP
#define RedirectionROUTINGCONFIG_HPP

#include "Config/Routing/RoutingConfig.hpp"
#include "Util/HttpRedirection.hpp"

class RedirectionRoutingConfig : public RoutingConfig
{

#ifdef TOOL
public:
#else
private:
#endif
	HttpRedirection redirection;

public:
	RedirectionRoutingConfig();
	virtual ~RedirectionRoutingConfig();

	virtual bool is_redirection_router() const;
	virtual bool is_cgi_router() const;
	virtual bool is_static_file_router() const;

	virtual ConfigMatchingResult is_match(const Span &path, const HttpMethod &method) const;

	virtual void validate() const;
	virtual bool prepare(const EnvironmentVariableDictionary &dictionary);

	const HttpRedirection &get_redirection() const;

#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

#endif