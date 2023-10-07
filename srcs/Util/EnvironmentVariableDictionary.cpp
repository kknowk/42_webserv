#include "Util/EnvironmentVariableDictionary.hpp"
#include <stdexcept>
#include <cstring>

void EnvironmentVariableDictionary::split(const std::string &text, std::pair<std::string, std::string> &pair)
{
	std::size_t index = text.find('=');
	if (index == std::string::npos)
	{
		pair.first = text;
		pair.second = std::string();
		return;
	}
	pair.first = text.substr(0, index);
	pair.second = text.substr(index + 1);
}

EnvironmentVariableDictionary::EnvironmentVariableDictionary(char const *const *pointer)
{
	if (pointer == NULL)
		return;
	std::pair<std::string, std::string> pair;
	while (*pointer)
	{
		EnvironmentVariableDictionary::split(*pointer++, pair);
		this->insert(pair);
	}
}

EnvironmentVariableDictionaryCharArray EnvironmentVariableDictionary::to_string_vector() const
{
	EnvironmentVariableDictionaryCharArray answer;
	for (const_iterator itr = this->begin(); itr != this->end(); itr++)
	{
		std::string text = itr->first + "=" + itr->second;
		char *output = new char[text.size() + 1];
		std::memcpy(output, text.c_str(), text.size());
		output[text.size()] = '\0';
		answer.push_back(output);
	}
	answer.push_back(NULL);
	return answer;
}

EnvironmentVariableDictionaryCharArray::EnvironmentVariableDictionaryCharArray()
	: std::vector<char *>(), should_dispose(false)
{
}

EnvironmentVariableDictionaryCharArray::~EnvironmentVariableDictionaryCharArray()
{
	if (should_dispose)
	{
		for (std::vector<char *>::iterator itr = this->begin(); itr != this->end(); itr++)
		{
			delete[] (*itr);
		}
	}
}

void EnvironmentVariableDictionaryCharArray::force_dispose()
{
	this->should_dispose = true;
}
