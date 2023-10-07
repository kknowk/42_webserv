#ifndef HTTPREQUEST_HPP
#define HTTPREQUEST_HPP

#include "Util/IoResult.hpp"
#include "Server/HttpRequestHeader.hpp"
#include "Server/HttpRequestParsingState.hpp"
#include <vector>
#include <string>

class HttpRequest
{
private:
	HttpRequestHeader header;
	std::vector<char> data;
	std::size_t parsed_byte_count;
	HttpRequestParsingState state;
	std::size_t index;

public:
	HttpRequest();
	const HttpRequestHeader &get_header() const;
	HttpRequestHeader &get_header();
	const std::vector<char> &get_data() const;
	std::vector<char> &get_data();
	const HttpRequestParsingState &get_state() const;
	std::size_t get_index() const;
	void set_index(std::size_t index);

	void parse(const std::vector<char> &buffer, std::size_t request_body_size_limit);
	std::size_t size() const;

	bool is_parse_success() const;
	bool is_error() const;
	bool is_critical_error() const;
	void clear();

	bool should_connection_be_closed() const;
};

#endif