#ifndef HTTPREQUESTHEADER_HPP
#define HTTPREQUESTHEADER_HPP

#include <string>
#include <set>
#include <map>
#include <vector>
#include <iostream>
#include "Util/Nullable.hpp"
#include "Util/HttpMethod.hpp"
#include "Util/Uri.hpp"
#include "Util/TimeSpan.hpp"

class HttpRequestHeader
{
public:
	enum ConnectionKind
	{
		ConnectionKind_KeepAlive,
		ConnectionKind_Close,
	};

	enum TransferEncodingKind
	{
		TransferEncodingKind_Chunked,
		TransferEncodingKind_Compress,
		TransferEncodingKind_Deflate,
		TransferEncodingKind_Gzip,
		TransferEncodingKind_Identity,
	};

	class HostHeader
	{
	private:
		std::string host;
		/// @brief no leading :
		Nullable<std::string> port;
		bool _is_valid;

	public:
		HostHeader();
		HostHeader(const std::string &host);
		HostHeader(const std::string &host, const std::string &port);

		void clear();
		const std::string &get_host() const;
		const Nullable<std::string> &get_port() const;
		bool is_valid() const;
		void set_valid();
	};

	HttpRequestHeader();

	void clear();
	std::size_t get_size() const;
	void set_size(std::size_t size);
	HttpMethod get_method() const;
	const Uri &get_resource_location() const;
	const std::string &get_normalized_resource_location() const;
	const Nullable<std::size_t> &get_content_length() const;
	const std::set<TransferEncodingKind> &get_transfer_encoding() const;
	const HostHeader &get_host() const;
	const std::string &get_content_type() const;

	std::map<std::string, std::vector<std::string> >::const_iterator at(const std::string &key) const;
	std::map<std::string, std::vector<std::string> >::const_iterator begin() const;
	std::map<std::string, std::vector<std::string> >::const_iterator end() const;

	bool is_chunked() const;

	int parse(const std::vector<char> &buffer, std::size_t byte_size, std::size_t content_length_limit);

	bool is_critical_error() const;
	bool should_connection_be_closed() const;

private:
	std::size_t byte_size;
	HttpMethod method;
	Uri resource_location;
	std::string normalized_resource_location;
	Nullable<std::size_t> content_length;
	ConnectionKind connection;
	std::set<TransferEncodingKind> transfer_encoding;
	HostHeader host; // required
	std::string content_type;
	bool has_critical_error;
	
	std::map<std::string, std::vector<std::string> > dictionary;

	int fill_special_settings(std::size_t content_length_limit);
	int fill_host(std::map<std::string, std::vector<std::string> >::iterator not_found);
	int fill_connection(std::map<std::string, std::vector<std::string> >::iterator not_found);
	void fill_content_type(std::map<std::string, std::vector<std::string> >::iterator not_found);
	int fill_transfer_encoding(std::map<std::string, std::vector<std::string> >::iterator transfer_encoding_found);
	int fill_content_length_or_transfer_encoding(std::size_t content_length_limit, std::map<std::string, std::vector<std::string> >::iterator not_found);
	int parse_request_line(std::vector<char>::const_iterator start, std::vector<char>::const_iterator end);
	int parse_header_line(std::vector<char>::const_iterator start, std::vector<char>::const_iterator end);
};

std::ostream &operator<<(std::ostream &stream, const HttpRequestHeader::HostHeader &value);

#endif