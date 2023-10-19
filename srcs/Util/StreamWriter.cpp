#include "Util/StreamWriter.hpp"
#include "Util/Uri.hpp"
#include "Util/IpAddress.hpp"
#include <cstring>
#include <climits>

void StreamWriter::write(std::ostream &stream, const unsigned long long int &value)
{
	char bytes[sizeof(unsigned long long int)];
	for (size_t index = 0; index < sizeof(unsigned long long int); index++)
		bytes[index] = static_cast<char>(static_cast<unsigned char>(value >> ((sizeof(unsigned long long int) - index - 1) * CHAR_BIT)));
	stream.write(bytes, sizeof(unsigned long long int));
}

void StreamWriter::write(std::ostream &stream, const unsigned long int &value)
{
	char bytes[sizeof(unsigned long int)];
	for (size_t index = 0; index < sizeof(unsigned long int); index++)
		bytes[index] = static_cast<char>(static_cast<unsigned char>(value >> ((sizeof(unsigned long int) - index - 1) * CHAR_BIT)));
	stream.write(bytes, sizeof(unsigned long int));
}

void StreamWriter::write(std::ostream &stream, const unsigned int &value)
{
	char bytes[sizeof(unsigned int)];
	for (size_t index = 0; index < sizeof(unsigned int); index++)
		bytes[index] = static_cast<char>(static_cast<unsigned char>(value >> ((sizeof(unsigned int) - index - 1) * CHAR_BIT)));
	stream.write(bytes, sizeof(unsigned int));
}

void StreamWriter::write(std::ostream &stream, const uint8_t &value)
{
	stream.write(reinterpret_cast<const char *>(&value), 1);
}

void StreamWriter::write(std::ostream &stream, const char &value)
{
	stream.write(&value, 1);
}

void StreamWriter::write(std::ostream &stream, const bool &value)
{
	char c;
	if (value)
	{
		c = 1;
	}
	else
	{
		c = 0;
	}
	stream.write(&c, 1);
}

void StreamWriter::write(std::ostream &stream, const std::string &value)
{
	StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
	stream.write(value.c_str(), value.size());
}

void StreamWriter::write(std::ostream &stream, const char *value)
{
	std::size_t length = std::strlen(value);
	StreamWriter::write(stream, static_cast<uint32_t>(length));
	stream.write(value, length);
}

void StreamWriter::write_to_lower(std::ostream &stream, const std::string &value)
{
	StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
	char c;
	for (std::string::const_iterator itr = value.begin(); itr != value.end(); itr++)
	{
		c = std::tolower(*itr);
		stream.write(&c, 1);
	}
}

void StreamWriter::write(std::ostream &stream, const Nullable<std::string> &value)
{
	StreamWriter::write(stream, value.has_value);
	if (value.has_value)
		StreamWriter::write(stream, value.value);
}

void StreamWriter::write(std::ostream &stream, const HttpStatusCode &value)
{
	StreamWriter::write(stream, static_cast<uint32_t>(value.get_value()));
}

void StreamWriter::write(std::ostream &stream, const HttpMethod &value)
{
	StreamWriter::write(stream, static_cast<uint8_t>(value));
}

void StreamWriter::write(std::ostream &stream, const TimeSpan &value)
{
	int64_t tmp = value.to_milliseconds();
	uint64_t tmp2;
	std::memcpy(&tmp2, &tmp, sizeof(int64_t));
	StreamWriter::write(stream, tmp2);
}

void StreamWriter::write(std::ostream &stream, const MediaTypeDictionary &value)
{
	StreamWriter::write(stream, value.get_default());
	const std::vector<std::string> &keys = value.get_key_array();
	const std::vector<std::string> &values = value.get_value_array();
	StreamWriter::write(stream, keys.size());
	for (std::size_t index = 0; index < keys.size(); index++)
	{
		StreamWriter::write(stream, keys[index]);
		StreamWriter::write(stream, values[index]);
	}
}

void StreamWriter::write(std::ostream &stream, const HttpRedirection &value)
{
	StreamWriter::write(stream, value.get_converter());
	StreamWriter::write(stream, value.get_code());
}

void StreamWriter::write(std::ostream &stream, const HttpRedirection::Variable &value)
{
	StreamWriter::write(stream, static_cast<uint8_t>(value));
}

void StreamWriter::write(std::ostream &stream, const Config &value)
{
	StreamWriter::write(stream, value.wait_time);
	StreamWriter::write(stream, value.servers);
	StreamWriter::write(stream, value.request_body_size_limit);
	StreamWriter::write(stream, value.keep_alive);
}

void StreamWriter::write(std::ostream &stream, const ServerConfig &value)
{
	StreamWriter::write(stream, value._is_ipv4);
	if (value._is_ipv4)
		StreamWriter::write(stream, value.ip);
	else
	{
		std::vector<char> tmp;
		IpAddress::append_normalize_v6(tmp, value.ip);
		StreamWriter::write(stream, tmp);
	}
	if (value.port.empty())
		StreamWriter::write(stream, "80");
	else
		StreamWriter::write(stream, value.port);
	StreamWriter::write_to_lower(stream, value.server_names);
	StreamWriter::write(stream, value.error_pages);
	StreamWriter::write(stream, value.default_error_page);
	StreamWriter::write(stream, value.routings);
}

void StreamWriter::write(std::ostream &stream, const ErrorPage &value)
{
	StreamWriter::write(stream, value.path);
	StreamWriter::write(stream, value.media_type);
}

void StreamWriter::write(std::ostream &stream, const Nullable<ErrorPage> &value)
{
	StreamWriter::write(stream, value.has_value);
	if (value.has_value)
		StreamWriter::write(stream, value.value);
}

void StreamWriter::write(std::ostream &stream, RoutingConfig const *const &value)
{
	StaticRoutingConfig const *tmp_static = dynamic_cast<StaticRoutingConfig const *>(value);
	if (tmp_static != NULL)
	{
		StreamWriter::write(stream, *tmp_static);
		return;
	}
	CgiRoutingConfig const *tmp_cgi = dynamic_cast<CgiRoutingConfig const *>(value);
	if (tmp_cgi != NULL)
	{
		StreamWriter::write(stream, *tmp_cgi);
		return;
	}
	StreamWriter::write(stream, *dynamic_cast<RedirectionRoutingConfig const *>(value));
}

static void write_location(std::ostream &stream, const std::string &value)
{
	Uri uri;
	if (!Uri::try_parse_start_from_path(value, uri))
		throw std::invalid_argument("invalid url.");
	std::vector<char> tmp;
	if (!uri.append_normalize_path(tmp))
		throw std::invalid_argument("invalid url.");
	StreamWriter::write(stream, tmp);
}

void StreamWriter::write(std::ostream &stream, const StaticRoutingConfig &value)
{
	value.validate();
	StreamWriter::write(stream, static_cast<uint8_t>(0));
	write_location(stream, value.location);
	StreamWriter::write(stream, value.methods);
	StreamWriter::write(stream, value.dictionary);
	StreamWriter::write(stream, value.root_directory);
	StreamWriter::write(stream, value.either_index_or_listing);
	StreamWriter::write(stream, value._accept_file_editing);
}

void StreamWriter::write(std::ostream &stream, const CgiRoutingConfig &value)
{
	value.validate();
	StreamWriter::write(stream, static_cast<uint8_t>(1));
	write_location(stream, value.location);
	StreamWriter::write(stream, value.methods);
	StreamWriter::write(stream, value.root_directory);
	StreamWriter::write(stream, value.either_index_or_listing);
	StreamWriter::write(stream, value.location_suffix);
	StreamWriter::write(stream, value.executor);
}

void StreamWriter::write(std::ostream &stream, const RedirectionRoutingConfig &value)
{
	value.validate();
	StreamWriter::write(stream, static_cast<uint8_t>(2));
	write_location(stream, value.location);
	StreamWriter::write(stream, value.methods);
	StreamWriter::write(stream, value.redirection);
}
