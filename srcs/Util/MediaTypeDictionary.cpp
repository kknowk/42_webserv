#include "Util/MediaTypeDictionary.hpp"
#include "Util/StringHelper.hpp"
#include <stdexcept>

MediaTypeDictionary::MediaTypeDictionary()
{
}

static bool is_all_lower(const std::string &value)
{
	for (std::string::const_iterator itr = value.begin(); itr != value.end(); itr++)
	{
		if (std::isupper(*itr))
			return false;
	}
	return true;
}

void MediaTypeDictionary::set(const std::string &key, const std::string &value)
{
	for (std::size_t index = 0; index < this->key.size(); index++)
	{
		const std::string &item = this->key[index];
		if (StringHelper::equals_ignore_case(item.begin(), item.end(), key.c_str()))
		{
			this->value[index] = value;
			return;
		}
	}
	if (is_all_lower(key))
		this->key.push_back(key);
	else
		this->key.push_back(StringHelper::to_lower_string(key.begin(), key.end()));
	this->value.push_back(value);
}

const std::string &MediaTypeDictionary::operator[](const std::string &key) const
{
	for (std::size_t index = 0; index < this->key.size(); index++)
	{
		const std::string &item = this->key[index];
		if (StringHelper::equals_ignore_case(item.begin(), item.end(), key.c_str()))
		{
			return this->value[index];
		}
	}
	return this->default_value;
}

const std::vector<std::string> MediaTypeDictionary::get_key_array() const
{
	return this->key;
}

const std::vector<std::string> MediaTypeDictionary::get_value_array() const
{
	return this->value;
}

MediaTypeDictionary MediaTypeDictionary::clone() const
{
	MediaTypeDictionary answer;
	answer.key = this->key;
	answer.value = this->value;
	return answer;
}

const std::string &MediaTypeDictionary::get_default() const
{
	return this->default_value;
}

void MediaTypeDictionary::set_default(const std::string &value)
{
	this->default_value = value;
}

MediaTypeDictionary MediaTypeDictionary::create_default_dictionary()
{
	MediaTypeDictionary dictionary;
	dictionary.set_default("application/octet-stream");
	dictionary.set("html", "text/html");
	dictionary.set("htm", "text/html");
	dictionary.set("txt", "text/plain");
	dictionary.set("xml", "text/xml");
	dictionary.set("js", "text/javascript");
	dictionary.set("css", "text/css");
	dictionary.set("cbor", "application/cbor");
	dictionary.set("gz", "application/gzip");
	dictionary.set("pdf", "application/pdf");
	dictionary.set("zip", "application/zip");
	dictionary.set("sh", "application/x-sh");
	dictionary.set("wasm", "application/wasm");
	dictionary.set("mp3", "audio/mpeg");
	dictionary.set("m4a", "audio/aac");
	dictionary.set("ogg", "audio/ogg");
	dictionary.set("mid", "audio/midi");
	dictionary.set("midi", "audio/midi");
	dictionary.set("wav", "audio/wav");
	dictionary.set("mpeg", "video/mpeg");
	dictionary.set("mpg", "video/mpeg");
	dictionary.set("mp4", "video/mp4");
	dictionary.set("webm", "video/webm");
	dictionary.set("ogv", "video/ogg");
	dictionary.set("mov", "video/quicktime");
	dictionary.set("qt", "video/quicktime");
	dictionary.set("jxl", "image/jxl");
	dictionary.set("gif", "image/gif");
	dictionary.set("jpg", "image/jpeg");
	dictionary.set("jpeg", "image/jpeg");
	dictionary.set("png", "image/png");
	dictionary.set("svg", "image/svg+xml");
	dictionary.set("ico", "image/vnf.micrisoft.icon");
	return dictionary;
}
