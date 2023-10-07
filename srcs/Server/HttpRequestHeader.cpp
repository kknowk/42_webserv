#include "Server/HttpRequestHeader.hpp"
#include "Util/StringHelper.hpp"
#include "Util/Uri.hpp"

HttpRequestHeader::HttpRequestHeader()
	: byte_size(0), method(HttpMethod_GET), resource_location(), normalized_resource_location(), content_length(), connection(ConnectionKind_KeepAlive), transfer_encoding(), host(), has_critical_error(false), dictionary()
{
}

void HttpRequestHeader::clear()
{
	*this = HttpRequestHeader();
}

void HttpRequestHeader::set_size(std::size_t size)
{
	this->byte_size = size;
}

HttpMethod HttpRequestHeader::get_method() const
{
	return this->method;
}

const Uri &HttpRequestHeader::get_resource_location() const
{
	return this->resource_location;
}

const std::string &HttpRequestHeader::get_normalized_resource_location() const
{
	return this->normalized_resource_location;
}

const Nullable<std::size_t> &HttpRequestHeader::get_content_length() const
{
	return this->content_length;
}

bool HttpRequestHeader::is_chunked() const
{
	return this->transfer_encoding.find(TransferEncodingKind_Chunked) != this->transfer_encoding.end();
}

const std::set<HttpRequestHeader::TransferEncodingKind> &HttpRequestHeader::get_transfer_encoding() const
{
	return this->transfer_encoding;
}

const HttpRequestHeader::HostHeader &HttpRequestHeader::get_host() const
{
	return this->host;
}

const std::string &HttpRequestHeader::get_content_type() const
{
	return this->content_type;
}

std::size_t HttpRequestHeader::get_size() const
{
	return this->byte_size;
}

HttpRequestHeader::HostHeader::HostHeader()
{
}

HttpRequestHeader::HostHeader::HostHeader(const std::string &host)
	: host(host), _is_valid(false)
{
	for (std::string::iterator itr = this->host.begin(); itr != this->host.end(); itr++)
	{
		*itr = std::tolower(*itr);
	}
}

HttpRequestHeader::HostHeader::HostHeader(const std::string &host, const std::string &port)
	: host(host), port(port), _is_valid(false)
{
	for (std::string::iterator itr = this->host.begin(); itr != this->host.end(); itr++)
	{
		*itr = std::tolower(*itr);
	}
}

void HttpRequestHeader::HostHeader::clear()
{
	this->host = std::string();
	this->port = Nullable<std::string>();
	this->_is_valid = false;
}

const std::string &HttpRequestHeader::HostHeader::get_host() const
{
	return this->host;
}

const Nullable<std::string> &HttpRequestHeader::HostHeader::get_port() const
{
	return this->port;
}

bool HttpRequestHeader::HostHeader::is_valid() const
{
	return this->_is_valid;
}

void HttpRequestHeader::HostHeader::set_valid()
{
	this->_is_valid = true;
}

int HttpRequestHeader::parse_request_line(std::vector<char>::const_iterator start, std::vector<char>::const_iterator end)
{
	std::vector<char>::const_iterator found_first_space = StringHelper::find_first_match_any(start, end, " \t", " \t" + 2);
	if (found_first_space == end)
		return 400;
	this->method = HttpMethod_parse(start, found_first_space);
	if (static_cast<int>(this->method) < 0)
		return 400;
	std::vector<char>::const_iterator found_second_space = StringHelper::find_first_match_any(found_first_space + 1, end, " \t", " \t" + 2);
	if (found_second_space == end)
		return 505;
	if (!StringHelper::equals(found_second_space + 1, end, "HTTP/1.1"))
		return 505;
	this->resource_location = Uri();
	if (!Uri::try_parse_start_from_path(std::string(found_first_space + 1, found_second_space), this->resource_location))
		return 400;
	std::vector<char> tmp;
	if (!this->resource_location.append_normalize_path(tmp))
		return 400;
	this->normalized_resource_location = std::string(tmp.begin(), tmp.end());
	return 0;
}

static std::vector<std::string> to_vector(std::vector<char>::const_iterator start, std::vector<char>::const_iterator end)
{
	std::vector<std::string> answer;
	std::vector<char>::const_iterator comma = StringHelper::find_first_match(start, end, ',');
	std::vector<char>::const_iterator before_comma_start = StringHelper::trim_start_any(start, comma, " \t", " \t" + 2);
	std::vector<char>::const_iterator before_comma_end = StringHelper::trim_end_any(before_comma_start, comma, " \t", " \t" + 2);
	while (comma != end)
	{
		answer.push_back(std::string(before_comma_start, before_comma_end));
		start = comma + 1;
		comma = StringHelper::find_first_match(start, end, ',');
		before_comma_start = StringHelper::trim_start_any(start, comma, " \t", " \t" + 2);
		before_comma_end = StringHelper::trim_end_any(before_comma_start, comma, " \t", " \t" + 2);
	}
	answer.push_back(std::string(before_comma_start, before_comma_end));
	return answer;
}

int HttpRequestHeader::parse_header_line(std::vector<char>::const_iterator start, std::vector<char>::const_iterator end)
{
	std::vector<char>::const_iterator found = StringHelper::find_first_match(start, end, ':');
	if (found == end)
		return 400;
	std::vector<char>::const_iterator trimmed_start = StringHelper::trim_start_any(found + 1, end, " \t", " \t" + 2);
	std::vector<char>::const_iterator trimmed_end = StringHelper::trim_end_any(trimmed_start, end, " \t", " \t" + 2);
	std::pair<std::string, std::vector<std::string> > pair(StringHelper::to_lower_string(start, found), to_vector(trimmed_start, trimmed_end));
	std::map<std::string, std::vector<std::string> >::iterator exisiting_item = this->dictionary.find(pair.first);
	if (exisiting_item == this->dictionary.end())
	{
		this->dictionary.insert(pair);
	}
	else
	{
		for (std::vector<std::string>::iterator itr = pair.second.begin(), itr_end = pair.second.end(); itr != itr_end; itr++)
		{
			exisiting_item->second.push_back(*itr);
		}
	}
	return 0;
}

int HttpRequestHeader::fill_connection(std::map<std::string, std::vector<std::string> >::iterator not_found)
{
	std::map<std::string, std::vector<std::string> >::iterator found = this->dictionary.find("connection");
	if (found == not_found)
		this->connection = HttpRequestHeader::ConnectionKind_KeepAlive;
	else if (found->second.empty() || found->second.size() > 1)
		return 400;
	else if (found->second[0] == "keep-alive")
		this->connection = HttpRequestHeader::ConnectionKind_KeepAlive;
	else if (found->second[0] == "close")
		this->connection = HttpRequestHeader::ConnectionKind_Close;
	else
		return 400;
	return 0;
}

void HttpRequestHeader::fill_content_type(std::map<std::string, std::vector<std::string> >::iterator not_found)
{
	std::map<std::string, std::vector<std::string> >::iterator found = this->dictionary.find("content-type");
	if (found == not_found || found->second.empty())
		this->content_type.clear();
	else
		this->content_type = found->second.at(0);
}

int HttpRequestHeader::fill_transfer_encoding(std::map<std::string, std::vector<std::string> >::iterator transfer_encoding_found)
{
	for (std::vector<std::string>::iterator transfer_encoding_iterator = transfer_encoding_found->second.begin(),
											transfer_encoding_iterator_end = transfer_encoding_found->second.end();
		 transfer_encoding_iterator != transfer_encoding_iterator_end;
		 transfer_encoding_iterator++)
	{
		std::string &kind = *transfer_encoding_iterator;
		if (kind == "chunked")
			this->transfer_encoding.insert(HttpRequestHeader::TransferEncodingKind_Chunked);
		else if (kind == "compress")
			this->transfer_encoding.insert(HttpRequestHeader::TransferEncodingKind_Compress);
		else if (kind == "deflate")
			this->transfer_encoding.insert(HttpRequestHeader::TransferEncodingKind_Deflate);
		else if (kind == "gzip")
			this->transfer_encoding.insert(HttpRequestHeader::TransferEncodingKind_Gzip);
		else if (kind == "identity")
			this->transfer_encoding.insert(HttpRequestHeader::TransferEncodingKind_Identity);
		else
			return 400;
	}
	return 0;
}

int HttpRequestHeader::fill_content_length_or_transfer_encoding(std::size_t content_length_limit, std::map<std::string, std::vector<std::string> >::iterator not_found)
{
	std::map<std::string, std::vector<std::string> >::iterator content_length_found = this->dictionary.find("content-length");
	std::map<std::string, std::vector<std::string> >::iterator transfer_encoding_found = this->dictionary.find("transfer-encoding");
	if (content_length_found == not_found)
	{
		this->content_length = 0;
		if (transfer_encoding_found == not_found)
		{
			this->transfer_encoding.clear();
		}
		else
		{
			int return_code = this->fill_transfer_encoding(transfer_encoding_found);
			if (return_code)
				return return_code;
		}
	}
	else
	{
		if (content_length_found->second.empty() || content_length_found->second.size() > 1 || content_length_found->second[0].empty())
			return 400;
		char *endptr;
		this->content_length = static_cast<std::size_t>(std::strtoul(content_length_found->second[0].c_str(), &endptr, 10));
		if (*endptr != '\0')
			return 400;
		if (transfer_encoding_found != not_found)
			return 400;
		if (this->content_length.value > content_length_limit)
			return 413;
		this->transfer_encoding.clear();
	}
	return 0;
}

int HttpRequestHeader::fill_host(std::map<std::string, std::vector<std::string> >::iterator not_found)
{
	std::map<std::string, std::vector<std::string> >::iterator found = this->dictionary.find("host");
	if (found == not_found)
		return 400;
	if (found->second.empty() || found->second.size() > 1)
		return 400;
	std::string &host = found->second[0];
	std::size_t colon_index = host.find(':');
	if (colon_index == std::string::npos)
		this->host = HttpRequestHeader::HostHeader(host);
	else if (colon_index == 0 || colon_index == host.size() - 1)
		return 400;
	else
		this->host = HttpRequestHeader::HostHeader(std::string(host, 0, colon_index), std::string(host, colon_index + 1));
	this->host.set_valid();
	return 0;
}

int HttpRequestHeader::fill_special_settings(std::size_t content_length_limit)
{
	std::map<std::string, std::vector<std::string> >::iterator not_found = this->dictionary.end();
	this->fill_content_type(not_found);
	int return_code = this->fill_connection(not_found);
	if (return_code)
		return return_code;
	return_code = this->fill_content_length_or_transfer_encoding(content_length_limit, not_found);
	if (return_code)
		return return_code;
	return_code = this->fill_host(not_found);
	if (return_code)
		return return_code;
	return 0;
}

int HttpRequestHeader::parse(const std::vector<char> &buffer, std::size_t byte_size, std::size_t content_length_limit)
{
	this->byte_size = byte_size;
	std::vector<char>::const_iterator start = buffer.begin();
	std::vector<char>::const_iterator end = start + this->byte_size - 2;
	std::vector<char>::const_iterator found_newline = StringHelper::find_first_match(start, end, "\r\n", "\r\n" + 2);
	int return_code = this->parse_request_line(start, found_newline);
	if (return_code)
	{
		this->has_critical_error = true;
		return return_code;
	}
	for (start = found_newline + 2; start != end; start = found_newline + 2)
	{
		found_newline = StringHelper::find_first_match(start, end, "\r\n", "\r\n" + 2);
		return_code = this->parse_header_line(start, found_newline);
		if (return_code)
		{
			this->has_critical_error = true;
			return return_code;
		}
	}
	return_code = fill_special_settings(content_length_limit);
	if (return_code)
	{
		this->has_critical_error = true;
	}
	return return_code;
}

bool HttpRequestHeader::is_critical_error() const
{
	return this->has_critical_error;
}

std::map<std::string, std::vector<std::string> >::const_iterator HttpRequestHeader::at(const std::string &key) const
{
	return this->dictionary.find(key);
}

std::map<std::string, std::vector<std::string> >::const_iterator HttpRequestHeader::begin() const
{
	return this->dictionary.begin();
}

std::map<std::string, std::vector<std::string> >::const_iterator HttpRequestHeader::end() const
{
	return this->dictionary.end();
}

bool HttpRequestHeader::should_connection_be_closed() const
{
	if (this->is_critical_error())
		return true;
	return this->connection == HttpRequestHeader::ConnectionKind_Close;
}

std::ostream &operator<<(std::ostream &stream, const HttpRequestHeader::HostHeader &value)
{
	stream << value.get_host();
	const Nullable<std::string> &port = value.get_port();
	if (port.has_value && !port.value.empty() && port.value != "80")
		stream << ':' << port.value;
	return stream;
}
