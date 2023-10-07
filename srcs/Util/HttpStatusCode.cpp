#include "Util/HttpStatusCode.hpp"

std::map<int, std::string> HttpStatusCode::message_dictionary;

HttpStatusCode::HttpStatusCode()
{
}

HttpStatusCode::HttpStatusCode(int value)
	: value(value)
{
}

HttpStatusCode::HttpStatusCode(const HttpStatusCode &right)
	: value(right.value)
{
}

HttpStatusCode::~HttpStatusCode()
{
}

HttpStatusCode &HttpStatusCode::operator=(const HttpStatusCode &right)
{
	if (this != &right)
	{
		this->value = right.value;
	}
	return *this;
}

int HttpStatusCode::get_value(void) const
{
	return this->value;
}

bool HttpStatusCode::operator==(const HttpStatusCode &right) const
{
	return this->value == right.value;
}

bool HttpStatusCode::operator!=(const HttpStatusCode &right) const
{
	return this->value != right.value;
}

bool HttpStatusCode::operator<(const HttpStatusCode &right) const
{
	return this->value < right.value;
}

bool HttpStatusCode::operator>(const HttpStatusCode &right) const
{
	return this->value > right.value;
}

bool HttpStatusCode::operator<=(const HttpStatusCode &right) const
{
	return this->value <= right.value;
}

bool HttpStatusCode::operator>=(const HttpStatusCode &right) const
{
	return this->value >= right.value;
}

const std::string &HttpStatusCode::get_message() const
{
	return HttpStatusCode::message_dictionary.at(this->value);
}

void HttpStatusCode::init()
{
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(100, "Continue"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(101, "Switching Protocols"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(102, "Processing"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(103, "Early Hints"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(200, "OK"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(201, "Created"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(202, "Accepted"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(203, "Non-Authoritative Information"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(204, "No Content"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(205, "Reset Content"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(206, "Partial Content"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(207, "Multi-Status"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(208, "Already Reported"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(226, "IM Used"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(300, "Multiple Choices"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(301, "Moved Permanently"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(302, "Found"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(303, "See Other"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(304, "Not Modified"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(305, "Use Proxy"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(306, "Switch Proxy"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(307, "Temporary Redirect"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(308, "Permanent Redirect"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(400, "Bad Request"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(401, "Unauthorized"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(402, "Payment Required"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(403, "Forbidden"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(404, "Not Found"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(405, "Method Not Allowed"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(406, "Not Acceptable"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(407, "Proxy Authentication Required"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(408, "Request Timeout"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(409, "Conflict"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(410, "Gone"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(411, "Length Required"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(412, "Precondition Failed"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(413, "Payload Too Large"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(414, "URI Too Long"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(415, "Unsupported Media Type"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(416, "Range Not Satisfiable"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(417, "Expectation Failed"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(418, "I'm a teapot"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(421, "Misdirected Request"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(422, "Unprocessable Entity"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(423, "Locked"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(424, "Failed Dependency"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(425, "Too Early"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(426, "Upgrade Required"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(428, "Precondition Required"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(429, "Too Many Requests"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(431, "Request Header Fields Too Large"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(451, "Unavailable For Legal Reasons"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(500, "Internal Server Error"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(501, "Not Implemented"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(502, "Bad Gateway"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(503, "Service Unavailable"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(504, "Gateway Timeout"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(505, "HTTP Version Not Supported"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(506, "Variant Also Negotiates"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(507, "Insufficient Storage"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(508, "Loop Detected"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(510, "Not Extended"));
	HttpStatusCode::message_dictionary.insert(std::pair<int, std::string>(511, "Network Authentication Required"));
}

std::ostream &operator<<(std::ostream &stream, const HttpStatusCode &value)
{
	stream << value.get_value() << ' ' << value.get_message();
	return stream;
}
