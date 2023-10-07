#ifndef CGIRESPONSE_HPP
#define CGIRESPONSE_HPP

#include <map>
#include <string>
#include <vector>
#include "Server/HttpResponse.hpp"
#include "Server/HttpRequest.hpp"

class CgiResponse
{
public:
	std::map<std::string, std::string> header;
	std::size_t header_byte_count;
	std::size_t document_byte_count;
	
	CgiResponse();

	bool parse(const std::vector<char> &buffer);

	bool is_document() const;
	bool is_client_redirect() const;
	bool is_local_redirect() const;

	void write_document(HttpResponse &response) const;
	void write_client_redirect(HttpResponse &response) const;
	void write_local_redirect(std::vector<char> &buffer, const HttpRequest &request) const;
};

#endif