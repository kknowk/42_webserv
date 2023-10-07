#include "Util/HttpMethod.hpp"
#include <cstdio>

std::ostream &operator<<(std::ostream &stream, const HttpMethod &value)
{
	switch (value)
	{
	case HttpMethod_GET:
		return stream << "GET";
	case HttpMethod_DELETE:
		return stream << "DELETE";
	case HttpMethod_POST:
		return stream << "POST";
	case HttpMethod_PUT:
		return stream << "PUT";
	case HttpMethod_HEAD:
		return stream << "HEAD";
	case HttpMethod_CONNECT:
		return stream << "CONNECT";
	case HttpMethod_OPTIONS:
		return stream << "OPTIONS";
	case HttpMethod_TRACE:
		return stream << "TRACE";
	case HttpMethod_PATCH:
		return stream << "PATCH";
	default:
		return stream << static_cast<int>(value);
	}
}

std::string HttpMethod_to_string(HttpMethod value)
{
	switch (value)
	{
	case HttpMethod_GET:
		return "GET";
	case HttpMethod_DELETE:
		return "DELETE";
	case HttpMethod_POST:
		return "POST";
	case HttpMethod_PUT:
		return "PUT";
	case HttpMethod_HEAD:
		return "HEAD";
	case HttpMethod_CONNECT:
		return "CONNECT";
	case HttpMethod_OPTIONS:
		return "OPTIONS";
	case HttpMethod_TRACE:
		return "TRACE";
	case HttpMethod_PATCH:
		return "PATCH";
	default:
		char tmp[64];
		std::snprintf(tmp, sizeof(tmp), "%d", static_cast<int>(value));
		return std::string(tmp);
	}
}
