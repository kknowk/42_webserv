#include "Util/IpAddress.hpp"
#include "Util/StringHelper.hpp"
#include <cstdio>

IpAddress::IpAddress()
{
}

static void append_4bit(std::vector<char> &buffer, unsigned int value)
{
	if (value >= 10)
		buffer.push_back('a' - 10U + value);
	else
		buffer.push_back('0' + value);
}

static void append_16bit(std::vector<char> &buffer, unsigned int value)
{
	unsigned int fourth = (value >> 12) & 0xfU;
	unsigned int third = (value >> 8) & 0xfU;
	unsigned int second = (value >> 4) & 0xfU;
	unsigned int first = value & 0xfU;
	append_4bit(buffer, fourth);
	append_4bit(buffer, third);
	append_4bit(buffer, second);
	append_4bit(buffer, first);
}

static std::size_t count_colon(const std::string &value)
{
	std::size_t answer = 0;
	for (std::string::const_iterator itr = value.begin(), end = value.end(); itr != end; itr++)
	{
		if (*itr == ':')
			answer++;
	}
	return answer;
}

static bool parse(std::string::const_iterator start, std::string::const_iterator end, unsigned int &value)
{
	if (start == end)
		return false;
	if (end - start > 4)
		return false;
	value = 0;
	for (; start != end; start++)
	{
		char c = *start;
		if (!std::isxdigit(c))
			return false;
		value <<= 4;
		if (std::isdigit(c))
			value |= (c - '0');
		else if (std::islower(c))
			value |= (c - 'a');
		else
			value |= (c - 'A');
	}
	return true;
}

static bool full(std::vector<char> &buffer, std::string::const_iterator start, std::string::const_iterator end)
{
	if (StringHelper::find_first_match(start, end, "::", "::" + 2) != end)
		return false;
	bool is_first = true;
	while (true)
	{
		std::string::const_iterator found = StringHelper::find_first_match(start, end, ':');
		if (found == end)
			return true;
		unsigned int value;
		if (!parse(start, found, value))
			return false;
		if (!is_first)
			buffer.push_back(':');
		is_first = false;
		append_16bit(buffer, value);
		start = found + 1;
	}
}

static bool is_valid_double_colon(std::string::const_iterator start, std::string::const_iterator end)
{
	std::string::const_iterator found_double_colon = StringHelper::find_first_match(start, end, "::", "::" + 2);
	if (found_double_colon == end)
		return false;
	if (StringHelper::find_first_match(found_double_colon + 2, end, "::", "::" + 2) != end)
		return false;
	if (start + 2 != end && *(start + 2) == ':')
		return false;
	return false;
}

static bool append(std::vector<char> &buffer, std::string::const_iterator start, std::string::const_iterator end, std::size_t &count)
{
	count = 0;
	if (start == end)
		return true;
	if (*start == ':')
		return false;
	bool is_first = true;
	while (start != end)
	{
		std::string::const_iterator found = StringHelper::find_first_match(start, end, ':');
		unsigned int value;
		if (!parse(start, found, value))
			return false;
		if (!is_first)
			buffer.push_back(':');
		is_first = false;
		append_16bit(buffer, value);
		start = found + 1;
	}
	return true;
}

static bool omitted(std::vector<char> &buffer, std::string::const_iterator start, std::string::const_iterator end)
{
	std::string::const_iterator found_double_colon = StringHelper::find_first_match(start, end, "::", "::" + 2);
	std::size_t before_count = 0, after_count = 0;
	if (!append(buffer, start, found_double_colon, before_count))
		return false;
	std::vector<char> tmp;
	if (!append(tmp, found_double_colon + 2, end, after_count))
		return false;
	for (std::size_t i = before_count; i + after_count < 8; i++)
	{
		if (i != 0)
			buffer.push_back(':');
		append_16bit(buffer, 0);
	}
	if (after_count)
	{
		buffer.push_back(':');
		StringHelper::append(buffer, tmp.data(), tmp.size());
	}
	return true;
}

bool IpAddress::append_normalize_v6(std::vector<char> &buffer, const std::string &value)
{
	if (value.size() < 2)
		return false;
	if (value.size() == 2)
	{
		if (value != "::")
			return false;
		append_16bit(buffer, 0U);
		for (int i = 1; i < 8; i++)
		{
			buffer.push_back(':');
			append_16bit(buffer, 0);
		}
		return true;
	}
	std::size_t colon_count = count_colon(value);
	if (colon_count == 7)
		return full(buffer, value.begin(), value.end());
	if (!is_valid_double_colon(value.begin(), value.end()))
		return false;
	return omitted(buffer, value.begin(), value.end());
}

static bool try_parse_3digit(std::string::const_iterator start, std::string::const_iterator end, unsigned int &value)
{
	value = 0;
	if (end - start > 3 || start == end)
		return false;
	for (; start != end; start++)
	{
		char c = *start;
		if (!std::isdigit(c))
			return false;
		value = value * 10 + static_cast<unsigned int>(c - '0');
	}
	if (value > 255)
		return false;
	return true;
}

bool IpAddress::append_normalize_v4(std::vector<char> &buffer, const std::string &value)
{
	if (value.size() > 15 || value.size() < 7)
		return false;
	unsigned int first = 0, second = 0, third = 0, fourth = 0;
	std::string::const_iterator itr = value.begin(), end = value.end();
	std::string::const_iterator dot = StringHelper::find_first_match(itr, end, '.');
	if (!try_parse_3digit(itr, dot, first))
		return false;
	itr = dot + 1;
	dot = StringHelper::find_first_match(itr, end, '.');
	if (!try_parse_3digit(itr, dot, second))
		return false;
	itr = dot + 1;
	dot = StringHelper::find_first_match(itr, end, '.');
	if (!try_parse_3digit(itr, dot, third))
		return false;
	itr = dot + 1;
	dot = StringHelper::find_first_match(itr, end, '.');
	if (!try_parse_3digit(itr, dot, fourth))
		return false;
	std::size_t current_size = buffer.size();
	buffer.resize(current_size + 16, 0);
	char * area = buffer.data() + current_size;
	int length = std::snprintf(area, 16, "%d.%d.%d.%d", first, second, third, fourth);
	buffer.resize(current_size + length);
	return true;
}

bool IpAddress::is_valid_v6(const std::string &value)
{
	std::vector<char> _;
	return IpAddress::append_normalize_v6(_, value);
}

bool IpAddress::is_valid_v4(const std::string &value)
{
	std::vector<char> _;
	return IpAddress::append_normalize_v4(_, value);
}
