#ifndef MEDIATYPEDICTIONARY_HPP
#define MEDIATYPEDICTIONARY_HPP

#include <vector>
#include <string>
#include "Util/StringHelper.hpp"

class MediaTypeDictionary
{
private:
	std::vector<std::string> key;
	std::vector<std::string> value;
	std::string default_value;

public:
	MediaTypeDictionary();
	void set_default(const std::string &value);
	/// @param key no preceding '.'
	void set(const std::string &key, const std::string &value);
	const std::string &operator[](const std::string &key) const;

	const std::vector<std::string> get_key_array() const;
	const std::vector<std::string> get_value_array() const;
	const std::string &get_default() const;

	MediaTypeDictionary clone() const;

	template <typename TIterator>
	const std::string &find_by_path(TIterator start, TIterator end) const
	{
		TIterator found = StringHelper::find_last(start, end, '.');
		if (found == end)
			return this->default_value;
		for (std::size_t index = 0; index < this->key.size(); index++)
		{
			if (StringHelper::equals_ignore_case(found + 1, end, this->key[index].c_str()))
				return this->value[index];
		}
		return this->default_value;
	}

	static MediaTypeDictionary create_default_dictionary();
};

#endif