#include "Config/Routing/RoutingConfig.hpp"

RoutingConfig::RoutingConfig()
{
}

RoutingConfig::~RoutingConfig()
{
}

const std::string &RoutingConfig::get_location() const
{
	return this->location;
}

const std::set<HttpMethod> &RoutingConfig::get_methods() const
{
	return this->methods;
}

bool RoutingConfigRefLess::operator()(const RoutingConfigRef &left, const RoutingConfigRef &right) const
{
	if (left->get_location().size() > right->get_location().size())
		return true;
	return false;
}
