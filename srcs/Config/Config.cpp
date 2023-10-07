#include "Config/Config.hpp"
#include <stdexcept>
#include "Util/FileUtility.hpp"
#include "Util/StringHelper.hpp"
#include <iostream>
#include <fstream>
#include <ios>
#include <set>

Config::Config(const EnvironmentVariableDictionary &dictionary)
	: dictionary(dictionary)
{
}

const ServerConfig &Config::get_default(const std::string &ip, const std::string &port) const
{
	for (std::vector<ServerConfig>::const_iterator itr = this->servers.begin(); itr != this->servers.end(); itr++)
	{
		if (itr->get_ip() == ip && itr->get_port() == port)
			return *itr;
	}
	throw std::out_of_range("ip and port pair is not found.");
}

void Config::make_servers_disposable()
{
	for (std::vector<ServerConfig>::iterator itr = this->servers.begin(); itr != this->servers.end(); itr++)
	{
		itr->force_dispose();
	}
}

Nullable<std::size_t> Config::find_server_id(const std::string &host, const std::string &port) const
{
	std::size_t index = 0;
	if (port.empty())
	{
		for (std::vector<ServerConfig>::const_iterator itr = this->servers.begin(); itr != this->servers.end(); itr++, index++)
		{
			if (StringHelper::equals_ignore_case(host.begin(), host.end(), itr->get_ip().c_str()) && itr->get_port() == "80")
				return Nullable<std::size_t>(index);
		}
	}
	else
	{
		for (std::vector<ServerConfig>::const_iterator itr = this->servers.begin(); itr != this->servers.end(); itr++, index++)
		{
			if (StringHelper::equals_ignore_case(host.begin(), host.end(), itr->get_ip().c_str()) && itr->get_port() == port)
				return Nullable<std::size_t>(index);
		}
	}
	return Nullable<std::size_t>();
}

Nullable<std::size_t> Config::find_server_id(const std::string &ip, const std::string &port, const std::string &server_name) const
{
	std::size_t index = 0;
	if (port.empty())
	{
		for (std::vector<ServerConfig>::const_iterator itr = this->servers.begin(); itr != this->servers.end(); itr++, index++)
		{
			if (StringHelper::equals_ignore_case(ip.begin(), ip.end(), itr->get_ip().c_str()) && itr->get_port() == "80")
			{
				const std::set<std::string> &server_names = itr->get_server_names();
				if (server_names.find(server_name) != server_names.end())
					return Nullable<std::size_t>(index);
			}
		}
	}
	else
	{
		for (std::vector<ServerConfig>::const_iterator itr = this->servers.begin(); itr != this->servers.end(); itr++, index++)
		{
			if (StringHelper::equals_ignore_case(ip.begin(), ip.end(), itr->get_ip().c_str()) && itr->get_port() == port)
			{
				const std::set<std::string> &server_names = itr->get_server_names();
				if (server_names.find(server_name) != server_names.end())
					return Nullable<std::size_t>(index);
			}
		}
	}
	return Nullable<std::size_t>();
}

const TimeSpan &Config::get_keep_alive() const
{
	return this->keep_alive;
}

Config::iterator Config::begin()
{
	return this->servers.begin();
}

Config::const_iterator Config::begin() const
{
	return this->servers.begin();
}

Config::iterator Config::end()
{
	return this->servers.end();
}

Config::const_iterator Config::end() const
{
	return this->servers.end();
}

const ServerConfig &Config::operator[](std::size_t server_id) const
{
	return this->servers.at(server_id);
}

std::size_t Config::get_server_id(const ServerConfig &value) const
{
	std::size_t server_id = 0;
	for (Config::const_iterator itr = this->begin(); itr != this->end(); itr++, server_id++)
	{
		if (&(*itr) == &value)
			return server_id;
	}
	throw std::out_of_range("server is not found.");
}

static bool has_duplicate_server_name(const std::set<std::string> &left, const std::set<std::string> &right)
{
	if (left.empty() || right.empty())
		return false;
	for (std::set<std::string>::const_iterator itr = left.begin(), rend = right.end(); itr != left.end(); itr++)
	{
		if (right.find(*itr) != rend)
			return true;
	}
	return false;
}

void Config::validate() const
{
	if (this->servers.empty())
		throw std::runtime_error("servers is empty.");
	if (this->wait_time > this->keep_alive)
		throw std::runtime_error("wait time must be less than or equal to keep alive.");
	for (std::size_t index0 = 0; index0 < this->servers.size(); index0++)
	{
		const ServerConfig &serverConfig0(this->servers.at(index0));
		serverConfig0.validate();
		for (std::size_t index1 = index0; index1-- > 0;)
		{
			const ServerConfig &serverConfig1(this->servers.at(index1));
			if (serverConfig0.get_ip() == serverConfig1.get_ip() && serverConfig0.get_port() == serverConfig1.get_port() && has_duplicate_server_name(serverConfig0.get_server_names(), serverConfig1.get_server_names()))
			{
				throw std::invalid_argument("ServerConfig's ip and port and names are same.");
			}
		}
	}
}

const TimeSpan &Config::get_wait_time() const
{
	return this->wait_time;
}

/// @return false: FAIL
static bool read_file(std::map<std::string, std::vector<char> > &map, const std::string &file_path)
{
	std::pair<std::string, std::vector<char> > pair;
	pair.first = file_path;
	std::map<std::string, std::vector<char> >::iterator found = map.find(pair.first);
	if (found != map.end())
		return true;
	std::ifstream input(pair.first.c_str(), std::ios::in);
	if (input.fail())
		return false;
	pair.second = std::vector<char>((std::istreambuf_iterator<char>(input)), std::istreambuf_iterator<char>());
	map.insert(pair);
	return true;
}

bool Config::prepare()
{
	for (std::vector<ServerConfig>::iterator server_iterator = this->servers.begin(),
											 server_end = this->servers.end();
		 server_iterator != server_end;
		 server_iterator++)
	{
		if (!server_iterator->prepare(this->dictionary))
			return false;
		for (std::map<HttpStatusCode, ErrorPage>::const_iterator iterator = server_iterator->get_error_pages().begin(),
																 end = server_iterator->get_error_pages().end();
			 iterator != end;
			 iterator++)
		{
			if (!read_file(this->error_pages_content, iterator->second.path))
				return false;
		}
	}
	return true;
}

Nullable<ErrorPage> Config::try_get_error_page(const HttpStatusCode &status_code, std::size_t server_id) const
{
	std::map<HttpStatusCode, ErrorPage>::const_iterator found;
	try
	{
		const ServerConfig &server = this->servers.at(server_id);
		found = server.get_error_pages().find(status_code);
		if (found != server.get_error_pages().end())
			return Nullable<ErrorPage>(found->second);
		const Nullable<ErrorPage> &server_default_error = server.get_default_error_page();
		if (server_default_error.has_value)
			return server_default_error;
		const ServerConfig &default_server = this->get_default(server.get_ip(), server.get_port());
		if (&default_server == &server)
			return Nullable<ErrorPage>();
		found = default_server.get_error_pages().find(status_code);
		if (found != default_server.get_error_pages().end())
			return Nullable<ErrorPage>(found->second);
		const Nullable<ErrorPage> &default_server_default_error = default_server.get_default_error_page();
		if (default_server_default_error.has_value)
			return default_server_default_error;
	}
	catch (const std::exception &e)
	{
	}
	return Nullable<ErrorPage>();
}

const std::vector<char> &Config::get_error_page_content(const HttpStatusCode &status_code, std::size_t server_id, std::string &content_type) const
{
	Nullable<ErrorPage> page = this->try_get_error_page(status_code, server_id);
	if (page.has_value)
	{
		std::map<std::string, std::vector<char> >::const_iterator found = this->error_pages_content.find(page.value.path);
		if (found != this->error_pages_content.end())
		{
			content_type = page.value.media_type;
			return found->second;
		}
	}
	throw std::runtime_error("invalid");
}

std::size_t Config::get_request_body_size_limit() const
{
	return this->request_body_size_limit;
}

std::vector<char> Config::get_default_error_page_content(const std::string &resource_path, const HttpStatusCode &status_code) const
{
	std::vector<char> buffer;
	StringHelper::append(buffer, "<!DOCTYPE html>\n<html>\n<head>\n<title>Error ");
	StringHelper::append(buffer, status_code.get_value());
	buffer.push_back(' ');
	StringHelper::append(buffer, status_code.get_message());
	StringHelper::append(buffer, "</title>\n<meta charset=\"utf-8\"/>\n</head>\n<body>\n<h1>Error!</h1>\n<div><h2>");
	StringHelper::append_html(buffer, resource_path);
	StringHelper::append(buffer, "</h2></div>\n</body>\n</html>\n");
	return buffer;
}

const EnvironmentVariableDictionary &Config::get_dictionary() const
{
	return this->dictionary;
}