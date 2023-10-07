#ifndef ENVIRONMENTVARIABLEDICTIONARY_HPP
#define ENVIRONMENTVARIABLEDICTIONARY_HPP

#include <string>
#include <map>
#include <vector>

class EnvironmentVariableDictionaryCharArray : public std::vector<char *>
{
private:
	bool should_dispose;
public:
	EnvironmentVariableDictionaryCharArray();
	~EnvironmentVariableDictionaryCharArray();

	void force_dispose();
};

class EnvironmentVariableDictionary : public std::map<std::string, std::string>
{
private:
	static void split(const std::string &text, std::pair<std::string, std::string> &pair);

public:
	EnvironmentVariableDictionary(char const *const *pointer);
	EnvironmentVariableDictionaryCharArray to_string_vector() const;

	typedef std::map<std::string, std::string>::iterator iterator;
	typedef std::map<std::string, std::string>::const_iterator const_iterator;
};

#endif