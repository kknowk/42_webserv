#include "Server/HttpRequest.hpp"
#include "Util/StringHelper.hpp"
#include <algorithm>
#include <stdexcept>

HttpRequest::HttpRequest()
	: header(), data(), parsed_byte_count(0), state(), index(0)
{
}

const HttpRequestHeader &HttpRequest::get_header() const
{
	return this->header;
}

HttpRequestHeader &HttpRequest::get_header()
{
	return this->header;
}

const std::vector<char> &HttpRequest::get_data() const
{
	return this->data;
}

std::vector<char> &HttpRequest::get_data()
{
	return this->data;
}

std::size_t HttpRequest::size() const
{
	return this->parsed_byte_count;
}

const HttpRequestParsingState &HttpRequest::get_state() const
{
	return this->state;
}

void HttpRequest::parse(const std::vector<char> &buffer, std::size_t request_body_size_limit)
{
HEAD:
	if (this->is_parse_success() || this->state.is_error())
		return;
	std::vector<char>::const_iterator iterator = buffer.begin(), end = buffer.end();
	iterator += this->parsed_byte_count;
	switch (this->state.state)
	{
	case HttpRequestParsingState::During_Head:
	{
		std::vector<char>::const_iterator found = StringHelper::find_first_match(iterator, end, "\r\n\r\n", "\r\n\r\n" + 4);
		if (found == end)
		{
			found = StringHelper::find_first_match(iterator, end, "\r\n", "\r\n" + 2);
			if (found - iterator >= 4096)
			{
				this->state.status_code = 414;
				this->state.state = HttpRequestParsingState::Critical_Error;
			}
			else if (!StringHelper::all_valid_ascii_char_in_http_request_header_line(iterator, end))
			{
				this->state.status_code = 400;
				this->state.state = HttpRequestParsingState::Critical_Error;
			}
			return;
		}
		this->parsed_byte_count = (found - buffer.begin()) + 4;
		this->state.status_code = this->header.parse(buffer, this->parsed_byte_count, request_body_size_limit);
		if (this->state.is_error())
		{
			if (this->header.is_critical_error())
				this->state.state = HttpRequestParsingState::Critical_Error;
			return;
		}
		const Nullable<std::size_t> &length = this->header.get_content_length();
		if (length.has_value)
		{
			this->state.current_length = length.value;
			this->state.current_position = 0;
			this->state.state = HttpRequestParsingState::During_Normal_Content;
		}
		else if (this->header.is_chunked())
			this->state.state = HttpRequestParsingState::During_Chunked_Content_Before_Length;
		else
			this->state.state = HttpRequestParsingState::Done;
	}
		goto HEAD;
	case HttpRequestParsingState::During_Normal_Content:
	{
		std::size_t length = end - iterator;
		std::size_t required_length = this->state.current_required_read_length();
		if (length >= required_length)
		{
			this->data.insert(this->data.end(), iterator, iterator + required_length);
			this->state.state = HttpRequestParsingState::Done;
			this->parsed_byte_count += required_length;
		}
		else
		{
			this->data.insert(this->data.end(), iterator, iterator + length);
			this->parsed_byte_count += length;
			this->state.current_position += length;
		}
	}
		goto HEAD;
	case HttpRequestParsingState::During_Chunked_Content_Before_Length:
	{
		std::vector<char>::const_iterator found = StringHelper::find_first_match(iterator, end, "\r\n", "\r\n" + 2);
		if (found == end)
			return;
		if (found == iterator)
		{
			this->state.status_code = 400;
			return;
		}
		char *endptr;
		this->state.current_position = 0;
		this->state.current_length = static_cast<std::size_t>(std::strtoul(iterator.base(), &endptr, 16));
		if (this->state.current_length > request_body_size_limit || this->data.size() > request_body_size_limit - this->state.current_length)
		{
			this->state.status_code = 413;
			this->state.state = HttpRequestParsingState::Critical_Error;
			return;
		}
		if (endptr != found.base())
		{
			this->state.status_code = 400;
			return;
		}
		this->parsed_byte_count += found - end + 2;
		this->state.state = HttpRequestParsingState::During_Chunked_Content_During_Content;
		goto HEAD;
	}
	case HttpRequestParsingState::During_Chunked_Content_During_Content:
	{
		std::size_t length = end - iterator;
		std::size_t required_length = this->state.current_required_read_length();
		if (length >= required_length)
		{
			this->data.insert(this->data.end(), iterator, iterator + required_length);
			this->state.state = HttpRequestParsingState::During_Chunked_Content_After_Content;
			this->parsed_byte_count += required_length;
			goto HEAD;
		}
		this->data.insert(this->data.end(), iterator, iterator + length);
		this->parsed_byte_count += length;
		this->state.current_position += length;
		return;
	}
	case HttpRequestParsingState::During_Chunked_Content_After_Content:
	{
		std::size_t length = end - iterator;
		if (length < 2)
			return;
		if (!StringHelper::equals(iterator, iterator + 2, "\r\n"))
		{
			this->state.status_code = 400;
			this->state.state = HttpRequestParsingState::Critical_Error;
			return;
		}
		this->parsed_byte_count += 2;
		this->state.current_position += 2;
		if (this->state.current_length)
			this->state.state = HttpRequestParsingState::During_Chunked_Content_Before_Length;
		else
			this->state.state = HttpRequestParsingState::Done;
		goto HEAD;
	}
	default:
		throw std::logic_error("HttpRequest::try_parse invalid status.");
	}
}

bool HttpRequest::is_parse_success() const
{
	return !this->state.is_error() && this->state.state == HttpRequestParsingState::Done;
}

bool HttpRequest::is_critical_error() const
{
	return this->state.is_critical_error() || this->header.is_critical_error();
}

bool HttpRequest::is_error() const
{
	return this->state.is_error();
}

void HttpRequest::clear()
{
	*this = HttpRequest();
}

bool HttpRequest::should_connection_be_closed() const
{
	return this->state.is_critical_error() || this->header.should_connection_be_closed();
}

std::size_t HttpRequest::get_index() const
{
	return this->index;
}

void HttpRequest::set_index(std::size_t index)
{
	this->index = index;
}