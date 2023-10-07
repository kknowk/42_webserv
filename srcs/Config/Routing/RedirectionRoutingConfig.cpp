#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include <stdexcept>

RedirectionRoutingConfig::RedirectionRoutingConfig()
{
}

RedirectionRoutingConfig::~RedirectionRoutingConfig()
{
}

bool RedirectionRoutingConfig::is_redirection_router() const
{
	return true;
}

bool RedirectionRoutingConfig::is_cgi_router() const
{
	return false;
}

bool RedirectionRoutingConfig::is_static_file_router() const
{
	return false;
}

ConfigMatchingResult RedirectionRoutingConfig::is_match(const Span &path, const HttpMethod &method) const
{
	if (!path.starts_with(this->location.c_str()))
		return ConfigMatchingResult_NotFound;
	if (this->methods.find(method) == this->methods.end())
		return ConfigMatchingResult_MethodNotMatch;
	return ConfigMatchingResult_Found;
}

void RedirectionRoutingConfig::validate() const
{
	if (this->location.empty())
		throw std::runtime_error("RedirectionRoutingConfig::validate: location is empty.");
	if (this->methods.empty())
		throw std::runtime_error("RedirectionRoutingConfig::validate: methods is empty.");
}

const HttpRedirection &RedirectionRoutingConfig::get_redirection() const
{
	return this->redirection;
}

bool RedirectionRoutingConfig::prepare(const EnvironmentVariableDictionary &dictionary)
{
	(void)dictionary;
	return true;
}