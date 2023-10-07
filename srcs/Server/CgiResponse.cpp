#include "Server/CgiResponse.hpp"
#include "Util/StringHelper.hpp"
#include "Util/Uri.hpp"
#include <cstdio>
#include <ctime>

CgiResponse::CgiResponse()
	: header(), header_byte_count()
{
}

/// @return false: invalid
static bool parse_header_line(std::map<std::string, std::string> &headers, std::vector<char>::const_iterator start, std::vector<char>::const_iterator end)
{
	std::vector<char>::const_iterator colon = StringHelper::find_first_match(start, end, ':');
	if (colon == end || colon == start)
		return false;
	if (StringHelper::trim_end_any(start, colon, " \t", " \t" + 2) != colon)
		return false;
	std::string key = StringHelper::to_lower_string(start, colon);
	std::map<std::string, std::string>::iterator found_by_key = headers.find(key);
	if (found_by_key != headers.end())
		return false;
	std::vector<char>::const_iterator trim_start = StringHelper::trim_start_any(colon + 1, end, " \t", " \t" + 2);
	std::vector<char>::const_iterator trim_end = StringHelper::trim_end_any(trim_start, end, " \t\r", " \t\r" + 3);
	std::string value(trim_start, trim_end);
	headers.insert(std::pair<std::string, std::string>(key, value));
	return true;
}

/// @return false: invalid
bool CgiResponse::parse(const std::vector<char> &buffer)
{
	std::size_t newline_size = 0;
	std::vector<char>::const_iterator itr = buffer.begin(), end = buffer.end();
	do
	{
		std::vector<char>::const_iterator newline_start = StringHelper::find_first_newline(itr, end, newline_size);
		if (newline_size == 0)
			return false;
		if (newline_start == itr)
		{
			itr += newline_size;
			break;
		}
		if (!parse_header_line(this->header, itr, newline_start))
			return false;
		itr = newline_start + newline_size;
	} while (true);
	this->header_byte_count = itr - buffer.begin();
	this->document_byte_count = buffer.size() - this->header_byte_count;
	return true;
}

bool CgiResponse::is_document() const
{
	std::map<std::string, std::string>::const_iterator found = this->header.find("content-type");
	if (found == this->header.end())
		return false;
	return true;
}

bool CgiResponse::is_client_redirect() const
{
	std::map<std::string, std::string>::const_iterator not_found = this->header.end();
	bool content_type = this->header.find("content-type") != not_found;
	std::map<std::string, std::string>::const_iterator found = this->header.find("location");
	bool location = found != not_found;
	bool status = this->header.find("status") != not_found;
	if (this->document_byte_count != 0)
	{
		if (!content_type || !location || !status)
			return false;
	}
	else
	{
		if (this->header.size() != ((content_type ? 1UL : 0UL) + (location ? 1UL : 0UL) + (status ? 1UL : 0UL)))
			return false;
	}
	const std::string &value = found->second;
	Uri uri;
	if (!Uri::try_parse(value, uri))
		return false;
	return true;
}

bool CgiResponse::is_local_redirect() const
{
	if (this->document_byte_count != 0)
		return false;
	if (this->header.size() != 1)
		return false;
	std::map<std::string, std::string>::const_iterator location = this->header.find("location");
	if (location == this->header.end())
		return false;
	const std::string &value = location->second;
	if (value.empty())
		return false;
	if (value[0] != '/')
		return false;
	return true;
}

#define APPEND(text)                                                  \
	{                                                                 \
		buffer.insert(buffer.end(), text, text + (sizeof(text) - 1)); \
	}

#define INSERT(text)                                                   \
	{                                                                  \
		const std::string &tmp_text = (text);                          \
		buffer.insert(buffer.end(), tmp_text.begin(), tmp_text.end()); \
	}

void CgiResponse::write_document(HttpResponse &response) const
{
	response.get_content().has_value = true;
	std::vector<char> &buffer = response.get_content().value;
	buffer.clear();
	APPEND("HTTP/1.1 ");
	std::map<std::string, std::string>::const_iterator status_found = this->header.find("status");
	if (status_found == this->header.end())
	{
		APPEND("200 OK");
	}
	else
	{
		INSERT(status_found->second);
	}
	APPEND("\r\nContent-Type: ");
	INSERT(this->header.at("content-type"));
	for (std::map<std::string, std::string>::const_iterator itr = this->header.begin(); itr != this->header.end(); itr++)
	{
		if (itr->first == "status" || itr->first == "content-type" || itr->first == "transfer-encoding" || itr->first == "content-length")
			continue;
		APPEND("\r\n");
		INSERT(itr->first);
		APPEND(": ");
		INSERT(itr->second);
	}
	if (this->header.find("server") == this->header.end())
		APPEND("\r\nServer: webserv")
	if (this->header.find("date") == this->header.end())
	{
		std::time_t now;
		std::time(&now);
		char tmp[128];
		int length = std::strftime(tmp, sizeof(tmp), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
		buffer.insert(buffer.end(), tmp, tmp + length);
	}
	APPEND("\r\nContent-Length: ");
	{
		char tmp[64];
		int length = std::snprintf(tmp, sizeof(tmp), "%zu", this->document_byte_count);
		buffer.insert(buffer.end(), tmp, tmp + length);
	}
	APPEND("\r\n\r\n");
	HttpResponseStatus &status = response.get_status();
	std::vector<char> &chunk_buffer = status.read_resource_buffer;
	buffer.insert(buffer.end(), chunk_buffer.begin() + this->header_byte_count, chunk_buffer.end());
	chunk_buffer.clear();
	status.current_offset = 0;
	status.current_length = buffer.size();
	status.is_reading_done = true;
	status.sending_target = HttpResponseStatus::SingleContent;
}

void CgiResponse::write_client_redirect(HttpResponse &response) const
{
	response.get_content().has_value = true;
	std::vector<char> &buffer = response.get_content().value;
	buffer.clear();
	if (this->document_byte_count == 0)
	{
		APPEND("HTTP/1.1 302 Found\r\nLocation: ");
	}
	else
	{
		APPEND("HTTP/1.1 ");
		INSERT(this->header.at("status"));
		APPEND("\r\nContent-Type: ");
		INSERT(this->header.at("content-type"));
		APPEND("\r\nContent-Length: ");
		{
			char tmp[64];
			int length = std::snprintf(tmp, sizeof(tmp), "%zu", this->document_byte_count);
			buffer.insert(buffer.end(), tmp, tmp + length);
		}
		APPEND("\r\nLocation: ");
	}
	INSERT(this->header.at("location"));
	APPEND("\r\n\r\n");
	HttpResponseStatus &status = response.get_status();
	std::vector<char> &chunk_buffer = status.read_resource_buffer;
	buffer.insert(buffer.end(), chunk_buffer.begin() + this->header_byte_count, chunk_buffer.end());
	chunk_buffer.clear();
	status.current_offset = 0;
	status.current_length = buffer.size();
	status.is_reading_done = true;
	status.sending_target = HttpResponseStatus::SingleContent;
}

#undef APPEND
#undef INSERT

#define APPEND(text)                                                           \
	{                                                                          \
		buffer.insert(buffer.begin() + size, text, text + (sizeof(text) - 1)); \
		size += sizeof(text) - 1;                                              \
	}

#define INSERT(text)                                                            \
	{                                                                           \
		const std::string &tmp_text = (text);                                   \
		buffer.insert(buffer.begin() + size, tmp_text.begin(), tmp_text.end()); \
		size += tmp_text.size();                                                \
	}

void CgiResponse::write_local_redirect(std::vector<char> &buffer, const HttpRequest &request) const
{
	std::size_t size = request.size();
	APPEND("GET ");
	INSERT(this->header.at("location"));
	APPEND(" HTTP/1.1\r\nHost: ");
	const HttpRequestHeader &header = request.get_header();
	const HttpRequestHeader::HostHeader &host = header.get_host();
	INSERT(host.get_host());
	const Nullable<std::string> &port = host.get_port();
	if (port.has_value)
	{
		APPEND(":");
		INSERT(port.value);
	}
	for (std::map<std::string, std::vector<std::string> >::const_iterator itr = header.begin(); itr != header.end(); itr++)
	{
		if (itr->first == "host" || itr->first == "content-length" || itr->first == "transfer-encoding")
			continue;
		APPEND("\r\n");
		INSERT(itr->first);
		APPEND(": ");
		for (std::vector<std::string>::const_iterator elem_start = itr->second.begin(), elem_itr = itr->second.begin(), elem_end = itr->second.end(); elem_itr != elem_end; elem_itr++)
		{
			if (elem_itr != elem_start)
				APPEND(", ");
			INSERT(*elem_itr);
		}
	}
	APPEND("\r\n\r\n");
}

#undef APPEND
#undef INSERT