#include "Server/HttpResponse.hpp"
#include "Server/Connection.hpp"
#include "Util/StringHelper.hpp"
#include <ctime>
#include <cstdio>
#include <cstdlib>

HttpResponse::HttpResponse()
	: status(), status_code(), header(), content(), _is_chunked_response(false), local_redirection()
{
}

const Nullable<HttpStatusCode> &HttpResponse::get_status_code() const
{
	return this->status_code;
}

void HttpResponse::set_status_code(const HttpStatusCode &value)
{
	this->status_code = value;
}

void HttpResponse::set_status_code(int value)
{
	this->status_code = HttpStatusCode(value);
}

const std::map<std::string, std::string> &HttpResponse::get_header() const
{
	return this->header;
}

std::map<std::string, std::string> &HttpResponse::get_header()
{
	return this->header;
}

const Nullable<std::vector<char> > &HttpResponse::get_content() const
{
	return this->content;
}

Nullable<std::vector<char> > &HttpResponse::get_content()
{
	return this->content;
}

bool HttpResponse::is_chunked_response() const
{
	return this->_is_chunked_response && !this->content.has_value;
}

bool HttpResponse::is_single_content_response() const
{
	return this->content.has_value;
}

void HttpResponse::make_chunked()
{
	this->_is_chunked_response = true;
	this->content.has_value = false;
}

const Nullable<Uri> &HttpResponse::get_local_redirection() const
{
	return this->local_redirection;
}

Nullable<Uri> &HttpResponse::get_local_redirection()
{
	return this->local_redirection;
}

void HttpResponse::encode_response_header()
{
	std::vector<char> &buffer = this->status.read_resource_buffer;
	StringHelper::append(buffer, "HTTP/1.1 ");
	StringHelper::append(buffer, this->status_code.value.get_value());
	buffer.push_back(' ');
	StringHelper::append(buffer, this->status_code.value.get_message());
	StringHelper::append_line(buffer);
	if (this->_is_chunked_response)
	{
		StringHelper::append(buffer, "Transfer-Encoding: chunked");
		StringHelper::append_line(buffer);
	}
	else if (this->content.has_value)
	{
		StringHelper::append(buffer, "Content-Length: ");
		StringHelper::append(buffer, this->content.value.size());
		StringHelper::append_line(buffer);
	}
	else
	{
		StringHelper::append(buffer, "Content-Length: 0");
		StringHelper::append_line(buffer);
	}
	StringHelper::append(buffer, "Date: ");
	std::time_t now;
	std::time(&now);
	StringHelper::append(buffer, now);
	StringHelper::append_line(buffer);
	StringHelper::append(buffer, "Server: " SERVER_NAME);
	StringHelper::append_line(buffer);
	for (std::map<std::string, std::string>::const_iterator itr = this->header.begin(); itr != this->header.end(); itr++)
	{
		StringHelper::append(buffer, itr->first);
		StringHelper::append(buffer, ": ");
		StringHelper::append(buffer, itr->second);
		StringHelper::append_line(buffer);
	}
	StringHelper::append_line(buffer);
}

void HttpResponse::clear()
{
	this->status = HttpResponseStatus();
	this->status_code.has_value = false;
	this->header.clear();
	this->content.has_value = false;
	this->_is_chunked_response = false;
	this->local_redirection.has_value = false;
}

bool HttpResponse::is_construction_completed() const
{
	return this->status.is_construction_completed();
}

void HttpResponse::construction_complete()
{
	this->status.sending_target = HttpResponseStatus::Header;
	this->encode_response_header();
	this->status.current_length = this->status.read_resource_buffer.size();
	this->status.current_offset = 0;
}

void HttpResponse::register_header(const std::string &name, const std::string &value)
{
	std::pair<std::string, std::string> pair;
	pair.first = name;
	for (std::string::iterator itr = pair.first.begin(); itr != pair.first.end(); itr++)
	{
		*itr = std::tolower(*itr);
	}
	std::map<std::string, std::string>::iterator found = this->header.find(pair.first);
	if (found == this->header.end())
	{
		pair.second = value;
		this->header.insert(pair);
	}
	else
	{
		found->second = found->second + ", " + value;
	}
}

const HttpResponseStatus &HttpResponse::get_status() const
{
	return this->status;
}

HttpResponseStatus &HttpResponse::get_status()
{
	return this->status;
}

bool HttpResponseStatus::is_construction_completed() const
{
	return this->sending_target != DuringConstruction;
}

std::size_t HttpResponseStatus::current_rest_length() const
{
	return this->current_length - this->current_offset;
}

void HttpResponseStatus::prepare_for_single_content_sending(const Nullable<std::vector<char> > &buffer)
{
	this->sending_target = HttpResponseStatus::SingleContent;
	this->current_offset = 0;
	this->current_length = buffer.has_value ? buffer.value.size() : 0;
}

void HttpResponseStatus::prepare_for_first_or_last_chunk(bool is_read_fd_alive)
{
	this->current_offset = 0;
	this->sending_target = HttpResponseStatus::FirstChunk;
	if (this->current_length != 0)
	{
		std::vector<char>::iterator itr = this->read_resource_buffer.begin();
		this->read_resource_buffer.erase(itr, itr + this->current_length);
	}
	this->current_length = this->read_resource_buffer.size();
	if (this->current_length)
	{
		int size = std::snprintf(this->chunk_length_buffer, sizeof(this->chunk_length_buffer), "%zX\r\n", this->current_length);
		this->read_resource_buffer.insert(this->read_resource_buffer.begin(), this->chunk_length_buffer, this->chunk_length_buffer + size);
		this->current_length += static_cast<std::size_t>(size);
		if (is_read_fd_alive)
			return;
	}
	if (!is_read_fd_alive)
	{
		const char last[] = "\r\n0\r\n\r\n";
		this->read_resource_buffer.insert(this->read_resource_buffer.end(), last, last + sizeof(last) - 1);
		this->current_length += sizeof(last) - 1U;
		this->sending_target = HttpResponseStatus::LastChunk;
	}
}

void HttpResponseStatus::prepare_for_middle_or_last_chunk(bool is_read_fd_alive)
{
	this->sending_target = HttpResponseStatus::MiddleChunk;
	this->current_offset = 0;
	if (this->current_length != 0)
	{
		std::vector<char>::iterator itr = this->read_resource_buffer.begin();
		this->read_resource_buffer.erase(itr, itr + this->current_length);
	}
	this->current_length = this->read_resource_buffer.size();
	if (this->current_length)
	{
		int size = std::snprintf(this->chunk_length_buffer, sizeof(this->chunk_length_buffer), "\r\n%zX\r\n", this->current_length);
		this->read_resource_buffer.insert(this->read_resource_buffer.begin(), this->chunk_length_buffer, this->chunk_length_buffer + size);
		this->current_length += static_cast<std::size_t>(size);
		if (is_read_fd_alive)
			return;
	}
	if (!is_read_fd_alive)
	{
		const char last[] = "\r\n0\r\n\r\n";
		this->read_resource_buffer.insert(this->read_resource_buffer.end(), last, last + sizeof(last) - 1);
		this->current_length += sizeof(last) - 1U;
		this->sending_target = HttpResponseStatus::LastChunk;
	}
}

HttpResponseStatus::HttpResponseStatus()
	: sending_target(HttpResponseStatus::DuringConstruction), chunk_length_buffer(), read_resource_buffer(), current_length(), current_offset(), is_reading_done(false)
{
}

void HttpResponseStatus::clear()
{
	this->read_resource_buffer.clear();
	this->current_length = 0;
	this->current_offset = 0;
	this->sending_target = HttpResponseStatus::DuringConstruction;
}

bool HttpResponse::should_connection_be_closed() const
{
	if (!this->is_construction_completed())
		return false;
	std::map<std::string, std::string>::const_iterator found = this->header.find("connection");
	if (found == this->header.end())
		return false;
	return found->second == "close";
}
