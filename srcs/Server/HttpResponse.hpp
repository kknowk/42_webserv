#ifndef HTTPRESPONSE_HPP
#define HTTPRESPONSE_HPP

#include "Util/HttpStatusCode.hpp"
#include "Util/Nullable.hpp"
#include "Util/Uri.hpp"
#include <map>
#include <string>
#include <vector>

class Connection;

struct HttpResponseStatus
{
public:
	enum Kind
	{
		DuringConstruction,
		Header,
		SingleContent,
		FirstChunk,
		MiddleChunk,
		LastChunk,
	};

	HttpResponseStatus();

	Kind sending_target;
	char chunk_length_buffer[64];
	std::vector<char> read_resource_buffer;

	std::size_t current_length;
	std::size_t current_offset;

	bool is_construction_completed() const;
	bool is_reading_done;
	std::size_t current_rest_length() const;

	void prepare_for_single_content_sending(const Nullable<std::vector<char> > &buffer);
	void prepare_for_first_or_last_chunk(bool is_read_fd_alive);
	void prepare_for_middle_or_last_chunk(bool is_read_fd_alive);

	void clear();
};

class HttpResponse
{
private:
	HttpResponseStatus status;
	Nullable<HttpStatusCode> status_code;
	std::map<std::string, std::string> header;
	Nullable<std::vector<char> > content;

	bool _is_chunked_response;

	Nullable<Uri> local_redirection;

	void encode_response_header();

public:
	HttpResponse();

	const Nullable<HttpStatusCode> &get_status_code() const;
	void set_status_code(const HttpStatusCode &value);
	void set_status_code(int value);

	const HttpResponseStatus &get_status() const;
	HttpResponseStatus &get_status();

	const std::map<std::string, std::string> &get_header() const;
	std::map<std::string, std::string> &get_header();

	void register_header(const std::string &name, const std::string &value);

	bool is_construction_completed() const;
	void construction_complete();

	bool is_single_content_response() const;

	bool is_chunked_response() const;
	void make_chunked();

	template<typename TIterator>
	void add_content(TIterator iterator, TIterator end)
	{
		if (!content.has_value)
		{
			content = std::vector<char>();
		}
		content.value.insert(content.value.end(), iterator, end);
	}

	const Nullable<std::vector<char> > &get_content() const;
	Nullable<std::vector<char> > &get_content();

	const Nullable<Uri> &get_local_redirection() const;
	Nullable<Uri> &get_local_redirection();

	void clear();

	bool should_connection_be_closed() const;
};

#ifndef SERVER_NAME
#define SERVER_NAME "webserv"
#endif

#endif