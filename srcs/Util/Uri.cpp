#include "Util/Uri.hpp"
#include "Util/StringHelper.hpp"
#include <new>
#include <vector>

Uri::Uri()
	: value(),
	  domain(value, 0, 0), userinfo(value, 0, 0),
	  port(value, 0, 0),
	  path(value, 0, 0),
	  parameter(value, 0, 0),
	  fragment(value, 0, 0)
{
}

Uri::Uri(const Uri &right)
{
	*this = right;
}

Uri::~Uri()
{
}

Uri &Uri::operator=(const Uri &right)
{
	if (this != &right)
	{
		this->value = right.value;
		this->domain = right.domain.rebase(this->value);
		this->port = right.port.rebase(this->value);
		this->fragment = right.fragment.rebase(this->value);
		this->parameter = right.parameter.rebase(this->value);
		this->path = right.path.rebase(this->value);
		this->userinfo = right.userinfo.rebase(this->value);
	}
	return *this;
}

bool Uri::is_valid_port()
{
	unsigned long port_number = 0;
	for (Span::iterator itr = this->port.begin() + 1; itr != this->port.end(); itr++)
	{
		char c = *itr;
		if (!std::isdigit(c))
			return false;
		unsigned int digit = c - '0';
		port_number = port_number * 10 + digit;
		if (port_number > 65535)
			return false;
	}
	return true;
}

bool Uri::is_valid_domain()
{
	if (this->domain.size() == 0)
		return false;
	if (this->domain[0] == '[')
	{
		if (this->domain[this->domain.size() - 1] != ']')
			return false;
	}
	return true;
}

bool Uri::is_valid_path()
{
	return true;
}

bool Uri::is_valid_parameter()
{
	return true;
}

bool Uri::is_valid_fragment()
{
	return true;
}

bool Uri::try_parse_domain()
{
	std::string::size_type userinfo_found = this->domain.find('@');
	if (userinfo_found != std::string::npos)
	{
		new (&this->userinfo) Span(this->domain, 0, userinfo_found);
		this->domain = this->domain.slice(userinfo_found + 1);
		if (this->userinfo.size() == 0)
			return false;
	}
	std::string::size_type port_found = this->domain.find(':');
	if (port_found != std::string::npos)
	{
		new (&this->port) Span(this->domain, port_found);
		this->domain = this->domain.slice(0, port_found);
		if (!this->is_valid_port())
			return false;
	}
	return this->is_valid_domain();
}

bool Uri::try_parse_parameter_and_fragment()
{
	std::string::size_type parameter_found = this->path.find('?');
	if (parameter_found == std::string::npos)
	{
		std::string::size_type fragment_found = this->path.find('#');
		if (fragment_found == std::string::npos)
			return this->is_valid_path();
		new (&this->fragment) Span(this->path, fragment_found);
		new (&this->path) Span(this->path, 0, fragment_found);
		if (!this->is_valid_path())
			return false;
		return this->is_valid_fragment();
	}
	new (&this->parameter) Span(this->path, parameter_found);
	new (&this->path) Span(this->path, 0, parameter_found);
	if (!this->is_valid_path())
		return false;
	std::string::size_type fragment_found = this->parameter.find('#');
	if (fragment_found == std::string::npos)
		return this->is_valid_parameter();
	new (&this->fragment) Span(this->parameter, fragment_found);
	new (&this->parameter) Span(this->parameter, 0, fragment_found);
	if (!this->is_valid_parameter())
		return false;
	return this->is_valid_fragment();
}

bool Uri::try_parse(const std::string &input, Uri &output)
{
	output.value = input;
	bool is_http_start = output.value.find("http://") == 0;
	if (!is_http_start && output.value.find("https://") != 0)
		return false;
	std::string::size_type found = output.value.find('/', (is_http_start ? sizeof("http://") : sizeof("https://")) - 1);
	if (found == std::string::npos)
		new (&output.domain) Span(output.value, 0, output.value.size());
	else
		new (&output.domain) Span(output.value, 0, found);
	if (!output.try_parse_domain())
		return false;
	if (found == std::string::npos)
		return true;
	new (&output.path) Span(output.value, found);
	if (output.path.size() == 0)
		return true;
	return output.try_parse_parameter_and_fragment();
}

bool Uri::try_parse_start_from_path(const std::string &input, Uri &output)
{
	output.value = input;
	if (output.value.empty() || output.value[0] != '/')
		return false;
	output.domain = Span();
	new (&output.path) Span(output.value);
	return output.try_parse_parameter_and_fragment();
}

const std::string &Uri::get_value() const
{
	return this->value;
}

const Span &Uri::get_domain() const
{
	return this->domain;
}

const Span &Uri::get_port() const
{
	return this->port;
}

const Span &Uri::get_path() const
{
	return this->path;
}

const Span &Uri::get_parameter() const
{
	return this->parameter;
}

const Span &Uri::get_anchor() const
{
	return this->fragment;
}

bool is_c0_control(int c)
{
	return c >= 0 && c <= 0x1f;
}

bool is_c0_control_or_space(int c)
{
	return c >= 0 && c <= 0x20;
}

bool is_control(int c)
{
	return is_c0_control(c) || (c >= 0x7f && c <= 0x9f);
}

bool is_forbidden_host_codepoint(int c)
{
	switch (c)
	{
	case '\0':
	case '\x09':
	case '\x0a':
	case '\x0d':
	case '\x20':
	case '\x23':
	case '\x2f':
	case '\x3a':
	case '\x3c':
	case '\x3e':
	case '\x3f':
	case '\x40':
	case '\x5b':
	case '\x5c':
	case '\x5d':
	case '\x5e':
	case '\x7c':
		return true;
	}
	return false;
}

bool is_forbidden_domain_codepoint(int c)
{
	return is_forbidden_host_codepoint(c) || is_c0_control(c) || c == '%' || c == '\x7f';
}

bool Uri::is_match(const std::string &location) const
{
	if (location.empty())
		return false;
	if (location[0] == '/')
	{
		if (this->path == location)
			return true;
		if (!this->path.starts_with(location.c_str()))
			return false;
		if (this->path[location.size()] == '/')
			return true;
		return false;
	}
	else
	{
		Uri uri;
		if (!Uri::try_parse(location, uri))
			return false;
		return uri.is_match(location);
	}
}

int Uri::calculate_hex_digit(char c0)
{
	if (std::isdigit(c0))
	{
		return c0 - '0';
	}
	return (c0 | 32) - 'a' + 10;
}

int Uri::calculate_hex_digits(char c0, char c1)
{
	return (calculate_hex_digit(c0) << 4) | calculate_hex_digit(c1);
}

bool Uri::is_uri_reserved_char(int c)
{
	switch (c)
	{
	case ':':
	case '/':
	case '?':
	case '#':
	case '[':
	case ']':
	case '@':
	case '!':
	case '$':
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case ';':
	case '=':
		return true;
	}
	return false;
}

bool Uri::is_not_appropriate_percent_encoding(int c)
{
	switch (c)
	{
	case '-':
	case '.':
	case '_':
	case '~':
		return true;
	}
	return std::isalnum(c);
}

void Uri::push_char(std::vector<char> &buffer, int c)
{
	if (is_not_appropriate_percent_encoding(c))
	{
		buffer.push_back(static_cast<char>(c));
		return;
	}
	buffer.push_back('%');
	int higher = (c >> 4) & 0xf;
	buffer.push_back(static_cast<char>(higher + (higher >= 10 ? 'A' - 10 : '0')));
	int lower = c & 0xf;
	buffer.push_back(static_cast<char>(lower + (lower >= 10 ? 'A' - 10 : '0')));
}

bool Uri::is_single_dot_path_segment(const std::vector<char> &buffer)
{
	return StringHelper::equals(buffer.begin(), buffer.end(), "/.");
}

bool Uri::is_double_dot_path_segment(const std::vector<char> &buffer)
{
	return StringHelper::equals(buffer.begin(), buffer.end(), "/..");
}

void Uri::normalize_path_segment(std::deque<std::vector<char> > &buffer)
{
	while (!buffer.empty())
	{
		const std::vector<char> &first = buffer.front();
		if (Uri::is_double_dot_path_segment(first) || Uri::is_single_dot_path_segment(first))
		{
			buffer.pop_front();
			continue;
		}
		break;
	}
	for (std::size_t index = 0; index < buffer.size();)
	{
		if (Uri::is_single_dot_path_segment(buffer[index]))
		{
			buffer.erase(buffer.begin() + index);
			continue;
		}
		index++;
	}
	for (std::size_t index = buffer.size() - 1; index > 0;)
	{
		if (Uri::is_double_dot_path_segment(buffer[index]))
		{
			buffer.erase(buffer.begin() + (index - 1), buffer.begin() + (index + 1));
			index -= 2;
			continue;
		}
		index--;
	}
}

static void remove_double_slash(std::vector<char> &buffer)
{
	for (std::size_t index = buffer.size(); index-- > 1;)
	{
		if (buffer[index] == '/' && buffer[index - 1] == '/')
		{
			buffer.erase(buffer.begin() + index, buffer.begin() + index + 1);
		}
	}
}

bool Uri::append_normalize_path(std::vector<char> &buffer) const
{
	std::vector<char> tmp;
	if (!Uri::normalize_url_encode(this->path.begin(), this->path.end(), tmp))
		return false;
	remove_double_slash(tmp);
	std::deque<std::vector<char> > dequed;
	Uri::divide_path_segment(tmp.begin(), tmp.end(), dequed);
	Uri::normalize_path_segment(dequed);
	Uri::combine_path_segment(dequed.begin(), dequed.end(), buffer);
	return true;
}

void Uri::decode_path(std::vector<char> &buffer)
{
	for (std::size_t index = 0; index < buffer.size();)
	{
		char c = buffer[index++];
		if (c != '%')
			continue;
		if (index == buffer.size())
			continue;
		char c0 = buffer[index++];
		if (index == buffer.size() || !std::isxdigit(c0))
			continue;
		char c1 = buffer[index];
		if (!std::isxdigit(c1))
			continue;
		buffer[index - 2] = static_cast<char>(static_cast<unsigned char>(Uri::calculate_hex_digits(c0, c1)));
		buffer.erase(buffer.begin() + index - 1, buffer.begin() + index + 1);
		--index;
	}
}
