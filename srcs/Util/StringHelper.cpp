#include "Util/StringHelper.hpp"
#include <cstring>
#include <cstdio>
#include <stdexcept>
#include <ctime>

StringHelper::StringHelper()
{
}

void StringHelper::append(std::vector<char> &buffer, const std::string &text)
{
	buffer.insert(buffer.end(), text.begin(), text.end());
}

void StringHelper::append_html(std::vector<char> &buffer, const std::string &text)
{
	StringHelper::append_html(buffer, text.begin(), text.end());
}

void StringHelper::append(std::vector<char> &buffer, const char *text)
{
	if (text == NULL)
		return;
	while (*text)
	{
		buffer.push_back(*text++);
	}
}

void StringHelper::append(std::vector<char> &buffer, const char *text, std::size_t count)
{
	if (text == NULL)
		return;
	for (std::size_t index = 0; index < count; index++)
	{
		if (text[index] == '\0')
			return;
		buffer.push_back(text[index]);
	}
}

void StringHelper::append(std::vector<char> &buffer, int value)
{
	char temp[64];
	int count = std::snprintf(temp, sizeof(temp), "%i", value);
	if (count < 0)
		throw std::invalid_argument("value encoding is failed.");
	std::size_t old_size = buffer.size();
	buffer.resize(old_size + static_cast<std::size_t>(count));
	std::memcpy(buffer.data() + old_size, temp, count);
}

void StringHelper::append(std::vector<char> &buffer, std::size_t value)
{
	char temp[64];
	int count = std::snprintf(temp, sizeof(temp), "%zu", value);
	if (count < 0)
		throw std::invalid_argument("value encoding is failed.");
	std::size_t old_size = buffer.size();
	buffer.resize(old_size + static_cast<std::size_t>(count));
	std::memcpy(buffer.data() + old_size, temp, count);
}

void StringHelper::append_line(std::vector<char> &buffer)
{
	buffer.push_back('\r');
	buffer.push_back('\n');
}

void StringHelper::append(std::vector<char> &buffer, const std::time_t &value)
{
	char temp[256];
	std::size_t count = static_cast<std::size_t>(std::strftime(temp, sizeof(temp), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&value)));
	StringHelper::append(buffer, temp, count);
}

char *StringHelper::strdup(const char *value)
{
	std::size_t size = std::strlen(value) + 1;
	char *answer = new char[size];
	std::memcpy(answer, value, size);
	return answer;
}