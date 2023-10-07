#ifndef HTTPMETHOD_HPP
#define HTTPMETHOD_HPP

#include <string>
#include <iostream>
#include "Util/StringHelper.hpp"

enum HttpMethod
{
	HttpMethod_GET,
	HttpMethod_POST,
	HttpMethod_DELETE,
	HttpMethod_PUT,
	HttpMethod_HEAD,
	HttpMethod_CONNECT,
	HttpMethod_OPTIONS,
	HttpMethod_TRACE,
	HttpMethod_PATCH,
};

std::ostream &operator<<(std::ostream &stream, const HttpMethod &value);
std::string HttpMethod_to_string(HttpMethod value);

template <typename TIterator>
HttpMethod HttpMethod_parse(TIterator start, TIterator end)
{
	if (StringHelper::equals(start, end, "GET"))
		return HttpMethod_GET;
	if (StringHelper::equals(start, end, "POST"))
		return HttpMethod_POST;
	if (StringHelper::equals(start, end, "DELETE"))
		return HttpMethod_DELETE;
	if (StringHelper::equals(start, end, "PUT"))
		return HttpMethod_PUT;
	if (StringHelper::equals(start, end, "HEAD"))
		return HttpMethod_HEAD;
	if (StringHelper::equals(start, end, "CONNECT"))
		return HttpMethod_CONNECT;
	if (StringHelper::equals(start, end, "OPTIONS"))
		return HttpMethod_OPTIONS;
	if (StringHelper::equals(start, end, "TRACE"))
		return HttpMethod_TRACE;
	if (StringHelper::equals(start, end, "PATCH"))
		return HttpMethod_PATCH;
	return static_cast<HttpMethod>(-1);
}

#endif