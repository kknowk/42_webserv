#include "Util/StreamReader.hpp"
#include "Util/Uri.hpp"
#include "Util/IpAddress.hpp"
#include <cstring>
#include <algorithm>

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Config &config)
{
	StreamReader::read(itr, end, config.wait_time);
	StreamReader::read(itr, end, config.servers);
	StreamReader::read(itr, end, config.request_body_size_limit);
	StreamReader::read(itr, end, config.keep_alive);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, ServerConfig &config)
{
	StreamReader::read(itr, end, config._is_ipv4);
	StreamReader::read(itr, end, config.ip);
	if (!config._is_ipv4)
	{
		std::vector<char> tmp;
		IpAddress::append_normalize_v6(tmp, config.ip);
		config.ip = std::string(tmp.begin(), tmp.end());
	}
	StreamReader::read(itr, end, config.port);
	StreamReader::read_to_lower(itr, end, config.server_names);
	StreamReader::read(itr, end, config.error_pages);
	StreamReader::read(itr, end, config.default_error_page);
	StreamReader::read(itr, end, config.routings);
	config.match_routings = config.routings;
	std::sort(config.match_routings.begin(), config.match_routings.end(), RoutingConfigRefLess());
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, ErrorPage &value)
{
	StreamReader::read(itr, end, value.path);
	StreamReader::read(itr, end, value.media_type);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Nullable<ErrorPage> &value)
{
	StreamReader::read(itr, end, value.has_value);
	if (value.has_value)
		StreamReader::read(itr, end, value.value);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, unsigned long long int &value)
{
	value = 0;
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read64 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, unsigned long int &value)
{
    if (sizeof(unsigned long int) == sizeof(unsigned int))
    {
        unsigned int tmp;
        StreamReader::read(itr, end, tmp);
        std::memcpy(&value, &tmp, sizeof(value));
    }
    else
    {
        unsigned long long int tmp;
        StreamReader::read(itr, end, tmp);
        std::memcpy(&value, &tmp, sizeof(value));
    }
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, unsigned int &value)
{
	value = 0;
	if (itr == end)
		throw std::out_of_range("read32 is out of range.");
	value = static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read32 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read32 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
	if (itr == end)
		throw std::out_of_range("read32 is out of range.");
	value = (value << 8) | static_cast<unsigned char>(*itr++);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, uint8_t &value)
{
	if (itr == end)
		throw std::out_of_range("read8 is out of range.");
	value = *itr++;
}

static std::vector<char>::const_iterator next(std::vector<char>::const_iterator itr, std::size_t length, const std::vector<char>::const_iterator &end)
{
	while (length)
	{
		if (itr++ == end)
			throw std::out_of_range("next is out of range.");
		--length;
	}
	return itr;
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, bool &value)
{
	if (itr == end)
		throw std::out_of_range("read bool is out of range.");
	value = *itr++ != 0;
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, char &value)
{
	if (itr == end)
		throw std::out_of_range("read char is out of range.");
	value = *itr++;
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::string &value)
{
	uint32_t size;
	StreamReader::read(itr, end, size);
	std::vector<char>::const_iterator text_end = next(itr, size, end);
	value = std::string(itr, text_end);
	itr = text_end;
}

void StreamReader::read_to_lower(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::string &value)
{
	StreamReader::read(itr, end, value);
	for (std::string::iterator itr = value.begin(); itr != value.end(); itr++)
	{
		*itr = std::tolower(*itr);
	}
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Nullable<std::string> &value)
{
	StreamReader::read(itr, end, value.has_value);
	if (value.has_value)
		StreamReader::read(itr, end, value.value);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, TimeSpan &value)
{
	uint64_t temp;
	StreamReader::read(itr, end, temp);
	int64_t temp2;
	std::memcpy(&temp2, &temp, sizeof(uint64_t));
	if (temp2 < 0)
		value = TimeSpan::infinity();
	else
		value = TimeSpan::from_milliseconds(temp);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, MediaTypeDictionary &value)
{
	std::string default_value;
	std::size_t count;
	StreamReader::read(itr, end, default_value);
	value.set_default(default_value);
	StreamReader::read(itr, end, count);
	std::string _key, _value;
	for (std::size_t index = 0; index < count; index++)
	{
		StreamReader::read(itr, end, _key);
		StreamReader::read(itr, end, _value);
		value.set(_key, _value);
	}
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpRedirection &value)
{
	StreamReader::read(itr, end, value.converter);
	StreamReader::read(itr, end, value.code);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpRedirection::Variable &value)
{
	uint8_t tmp;
	StreamReader::read(itr, end, tmp);
	value = static_cast<HttpRedirection::Variable>(tmp);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpStatusCode &config)
{
	uint32_t tmp;
	StreamReader::read(itr, end, tmp);
	config.value = tmp;
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpMethod &value)
{
	uint8_t tmp;
	StreamReader::read(itr, end, tmp);
	value = static_cast<HttpMethod>(tmp);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, RoutingConfigRef &value)
{
	uint8_t kind;
	StreamReader::read(itr, end, kind);
	switch (kind)
	{
	case 0:
	{
		StaticRoutingConfig *tmp = new StaticRoutingConfig();
		StreamReader::read(itr, end, *tmp);
		value = tmp;
	}
	break;
	case 1:
	{
		CgiRoutingConfig *tmp = new CgiRoutingConfig();
		StreamReader::read(itr, end, *tmp);
		value = tmp;
	}
	break;
	case 2:
	{
		RedirectionRoutingConfig *tmp = new RedirectionRoutingConfig();
		StreamReader::read(itr, end, *tmp);
		value = tmp;
	}
	break;
	default:
		throw std::out_of_range("RoutingConfig kind is out of range.");
	}
}

static void read_location(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::string &value)
{
	uint32_t size;
	StreamReader::read(itr, end, size);
	if (size == 0)
	{
		value = "/";
		return;
	}
	std::vector<char>::const_iterator text_end = next(itr, size, end);
	value = std::string(itr, text_end) + "/";
	itr = text_end;
	Uri uri;
	if (!Uri::try_parse_start_from_path(value, uri))
		throw std::invalid_argument("invalid url");
	std::vector<char> tmp;
	if (!uri.append_normalize_path(tmp))
		throw std::invalid_argument("invalid url");
	value = std::string(tmp.begin(), tmp.end());
}

static void read_root_directory(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::string &value)
{
	uint32_t size;
	StreamReader::read(itr, end, size);
	std::vector<char>::const_iterator text_end = next(itr, size, end);
	if (size > 1 && *(itr + (size - 1)) == '/')
		value = std::string(itr, text_end - 1);
	else
		value = std::string(itr, text_end);
	itr = text_end;
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, StaticRoutingConfig &value)
{
	read_location(itr, end, value.location);
	StreamReader::read(itr, end, value.methods);
	StreamReader::read(itr, end, value.dictionary);
	read_root_directory(itr, end, value.root_directory);
	StreamReader::read(itr, end, value.either_index_or_listing);
	StreamReader::read(itr, end, value._accept_file_editing);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, CgiRoutingConfig &value)
{
	read_location(itr, end, value.location);
	StreamReader::read(itr, end, value.methods);
	read_root_directory(itr, end, value.root_directory);
	StreamReader::read(itr, end, value.either_index_or_listing);
	StreamReader::read(itr, end, value.location_suffix);
	StreamReader::read(itr, end, value.executor);
}

void StreamReader::read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, RedirectionRoutingConfig &value)
{
	read_location(itr, end, value.location);
	StreamReader::read(itr, end, value.methods);
	StreamReader::read(itr, end, value.redirection);
}
