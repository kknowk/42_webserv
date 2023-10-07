#ifndef CONFIG_HPP
#define CONFIG_HPP

#include <string>
#include <stdint.h>
#include <vector>
#include <map>

#include "Util/Uri.hpp"
#include "Util/HttpMethod.hpp"
#include "Util/TimeSpan.hpp"
#include "Util/HttpStatusCode.hpp"
#include "Util/Nullable.hpp"
#include "Util/EnvironmentVariableDictionary.hpp"
#include "Config/ErrorPage.hpp"
#include "Config/ServerConfig.hpp"

class ServerConfig;

class Config
{
#ifdef TOOL
public:
#else
private:
#endif
	TimeSpan wait_time;
	TimeSpan keep_alive;
	std::vector<ServerConfig> servers;
	std::size_t request_body_size_limit;

	std::map<std::string, std::vector<char> > error_pages_content;
	const EnvironmentVariableDictionary &dictionary;

	Nullable<ErrorPage> try_get_error_page(const HttpStatusCode &status_code, std::size_t server_id) const;

public:
	Config(const EnvironmentVariableDictionary &dictionary);

	const ServerConfig &get_default(const std::string &ip, const std::string &port) const;

	const TimeSpan &get_wait_time() const;

	typedef std::vector<ServerConfig>::iterator iterator;
	typedef std::vector<ServerConfig>::const_iterator const_iterator;

	Nullable<std::size_t> find_server_id(const std::string &host, const std::string &port) const;
	Nullable<std::size_t> find_server_id(const std::string &host, const std::string &port, const std::string &server_name) const;

	std::size_t get_request_body_size_limit() const;

	const TimeSpan &get_keep_alive() const;

	iterator begin();
	const_iterator begin() const;

	iterator end();
	const_iterator end() const;

	void make_servers_disposable();

	const ServerConfig &operator[](std::size_t server_id) const;
	std::size_t get_server_id(const ServerConfig &value) const;

	const EnvironmentVariableDictionary &get_dictionary() const;

	void validate() const;

	/// @return false: FAIL
	bool prepare();

	std::vector<char> get_default_error_page_content(const std::string &resource_path, const HttpStatusCode &status_code) const;
	const std::vector<char> &get_error_page_content(const HttpStatusCode &status_code, std::size_t server_id, std::string &content_type) const;

#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

#endif