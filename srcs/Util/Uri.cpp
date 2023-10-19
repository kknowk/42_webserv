#include "Util/Uri.hpp"
#include "Util/StringHelper.hpp"
#include <new>
#include <vector>
#include <set>
#include <iostream>
#include <sstream>

Uri::Uri()
	:	value(),
	domain(value, 0, 0),
	userinfo(value, 0, 0),
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
		this->userinfo = right.userinfo.rebase(this->value);
		this->port = right.port.rebase(this->value);
		this->path = right.path.rebase(this->value);
		this->parameter = right.parameter.rebase(this->value);
		this->fragment = right.fragment.rebase(this->value);
	}
	return *this;
}

bool Uri::is_valid_port()
{
	// https://ja.linux-console.net/?p=7399
	const unsigned int BLOCKED_PORT_ARRAY[] = \
	{21, 22, 23, 53, 80, 161, 1080, 4444, 6660, 6661, 6662, 6663, 6664, 6665, 6666, 6667, 6668, 6669};
	const std::set<unsigned int> BLOCKED_PORTS(BLOCKED_PORT_ARRAY, BLOCKED_PORT_ARRAY + sizeof(BLOCKED_PORT_ARRAY) / sizeof(BLOCKED_PORT_ARRAY[0]));
	unsigned long port_num = 0;

	// : スタートのためbegin() + 1
	for (Span::iterator itr = this->port.begin() + 1; itr != this->port.end(); itr++)
	{
		char c = *itr;
		if (!std::isdigit(c))
			return false;
		unsigned int digit = c - '0';
		port_num = port_num * 10 + digit;
		// 16-bit の負でない整数の最大値
		if (port_num > 65535)
			return false;
	}
	if (BLOCKED_PORTS.find(port_num) != BLOCKED_PORTS.end())
		return false;
	return true;
}

// https://www.nic.ad.jp/ja/dom/system.html
bool Uri::is_valid_domain()
{
	if (this->domain.size() > 253)
		return false; 
	// https://www.ipentec.com/document/windows-internet-explorer-specified-ipv6-in-url
	// ipv6
	if (domain[0] == '[')
	{
		// '[' 分+1
		Span::iterator start = this->domain.begin() + 1;
		Span::iterator end = std::find(start, this->domain.end(), ':');
	
		while (end != this->domain.end())
		{
			std::string label(start, end);
			if (label.size() > 4)
				return false;
			for (std::string::iterator it = label.begin(); it != label.end(); ++it)
			{
				char c = *it;
				if (!std::isxdigit(c))
					return false;
			}
			start = end + 1;
			end = std::find(start, this->domain.end(), ':');
		}
		if (domain[domain.size() - 1] != ']')
			return false;
		return true;
	}
	else
	{
		Span::iterator start = this->domain.begin();
		Span::iterator end = std::find(start, this->domain.end(), '.');

		while (end != this->domain.end())
		{
			std::string label(start, end);
			if (label.size() > 63)
				return false;
			for (std::string::iterator it = label.begin(); it != label.end(); ++it)
			{
				char c = *it;
				if (!std::isalnum(c) && c != '-')
					return false;
			}
			start = end + 1;
			end = std::find(start, this->domain.end(), '.');
		}
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
	// ipv6
	if (this->domain[0] == '[')
	{
		size_t end = this->domain.find(']');
		if (end == std::string::npos)
			return false;
		if (end + 1 < this->domain.size() && this->domain[end + 1] == ':')
		{
			new (&this->port) Span(this->domain, end + 2);
			this->domain = this->domain.slice(1, end - 1);
		}
		return true;
	}
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
		new (&this->port) Span(this->domain, port_found + 1);
		this->domain = this->domain.slice(0, port_found);
		if (!this->is_valid_port())
			return false;
	}
	return is_valid_domain();
}

bool Uri::try_parse_parameter_and_fragment()
{
	// 検索クエリなど
	// 例 : https://example.com/page.html?param=value#section1
	std::string::size_type parameter_found = this->path.find('?');
	if (parameter_found == std::string::npos)
	{
		// 特定のページ内のセクションや位置を指定
		std::string::size_type fragment_found = this->path.find('#');
		if (fragment_found == std::string::npos)
			return this->is_valid_path();
		// '#'以降の位置を確保　例 : #section1
		new (&this->fragment) Span(this->path, fragment_found);
		// クエリがないので#までを確保またドメインとスキームもない　例 : /page.html
		new (&this->path) Span(this->path, 0, fragment_found);
	}
	new (&this->parameter) Span(this->path, parameter_found);
	// クエリがあるので?までをパスとする
	new (&this->path) Span(this->path, 0, parameter_found);
	if (!this->is_valid_path())
		return false;
	std::string::size_type fragment_found = this->path.find('#');
	if (fragment_found == std::string::npos)
			return this->is_valid_parameter();
	new (&this->fragment) Span(this->parameter, fragment_found);
	new (&this->parameter) Span(this->parameter, 0, fragment_found);
	if (!this->is_valid_parameter())
		return false;
	return this->is_valid_fragment();

}

// リダイレクトで使う
bool Uri::try_parse(const std::string &input, Uri &output)
{
	output.value = input;
	bool is_http_start = output.value.find("http://") == 0;
	bool is_https_start = output.value.find("https://") == 0;
	if (!is_http_start && !is_https_start)
		return false;
	// 終端のヌル文字含めず
	std::string::size_type found = output.value.find('/', (is_http_start ? sizeof("http://") : sizeof("https://")) - 1);
	// path がなかったら
	if (found == std::string::npos)
		new (&output.domain) Span(output.value, 0, output.value.size());
	else
		new (&output.domain) Span(output.value, 0, found);
	if (!output.try_parse_domain())
		return false;
	// path はなくとも良い
	if (found == std::string::npos)
		return true;
	// path を格納
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
	// NULL、BS、TAB 、改行　とか
	return c >= 0 && c <= 0x1f;
}

bool is_c0_control_or_space(int c)
{
	// space
	return c >= 0 && c <= 0x20;
}

bool is_control(int c)
{
	// is_space 的な
	return is_c0_control(c) || (c >= 0x7f && c <= 0x9f);
}

// https://triple-underscore.github.io/URL-ja.html#hosts-(domains-and-ip-addresses)
// 禁止されているホスト符号位置
bool is_forbidden_host_codepoint(int c)
{
	switch (c)
	{
	// NULL
	case '\0':
	// TAB
	case '\x09':
	// LF
	case '\x0a':
	// CR
	case '\x0d':
	// SPACE
	case '\x20':
	// #
	case '\x23':
	// /
	case '\x2f':
	// :
	case '\x3a':
	// <
	case '\x3c':
	// >
	case '\x3e':
	// ?
	case '\x3f':
	// @
	case '\x40':
	// [
	case '\x5b':
	// '\'
	case '\x5c':
	// ]
	case '\x5d':
	// ^
	case '\x5e':
	// |
	case '\x7c':
		return true;
	}
	return false;
}

bool is_forbidden_domain_codepoint(int c)
{
	// DELETE文字
	return is_forbidden_host_codepoint(c) || is_c0_control(c) || c == '%' || c == '\x7f';
}

int Uri::calculate_hex_digit(char c0)
{
	if (std::isdigit(c0))
		return c0 - '0';
	// 'A' = 65 = 0100 0001 (バイナリ)
	// 'a' = 97 = 0110 0001 (バイナリ)
	return (c0 | 32) - 'a' + 10;
}

int Uri::calculate_hex_digits(char c0, char c1)
{
	return (calculate_hex_digit(c0) << 4) | calculate_hex_digit(c1);
}

// 特定の役割がある
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

// to file path
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
