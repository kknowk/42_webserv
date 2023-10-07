#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Util/StringHelper.hpp"
#include "Util/Span.hpp"
#include "Util/FileUtility.hpp"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdexcept>
#include <fcntl.h>

CgiRoutingConfig::CgiRoutingConfig()
	: RoutingConfig()
{
}

CgiRoutingConfig::~CgiRoutingConfig()
{
}

bool CgiRoutingConfig::is_cgi_router() const
{
	return true;
}

bool CgiRoutingConfig::is_redirection_router() const
{
	return false;
}

bool CgiRoutingConfig::is_static_file_router() const
{
	return false;
}

bool CgiRoutingConfig::handle_single_executable() const
{
	return !this->executor.has_value;
}

ConfigMatchingResult CgiRoutingConfig::is_match(const Span &path, const HttpMethod &method) const
{
	if (!path.starts_with(this->location.c_str()))
		return ConfigMatchingResult_NotFound;
	if (this->location_suffix.has_value)
	{
		// location has trailing slash('/').
		Span sliced = path.slice(this->location.size());
		Span::iterator found = StringHelper::find_first_match(sliced.begin(), sliced.end(), '/');
		if (!StringHelper::ends_with(sliced.begin(), found, this->location_suffix.value.c_str()))
			return ConfigMatchingResult_NotFound;
	}
	if (this->methods.find(method) == this->methods.end())
		return ConfigMatchingResult_MethodNotMatch;
	return ConfigMatchingResult_Found;
}

const Nullable<std::string> &CgiRoutingConfig::get_location_suffix() const
{
	return this->location_suffix;
}

const Either<std::string, bool> &CgiRoutingConfig::get_either_index_or_listing() const
{
	return this->either_index_or_listing;
}

const std::string &CgiRoutingConfig::get_executor() const
{
	return this->actual_executor;
}

void CgiRoutingConfig::validate() const
{
	if (this->location.empty())
		throw std::runtime_error("CgiRoutingConfig::validate: location is empty.");
	if (this->methods.empty())
		throw std::runtime_error("CgiRoutingConfig::validate: methods is empty.");
	if (this->executor.has_value && this->executor.value.empty())
		throw std::runtime_error("CgiRoutingConfig::validate: executor is empty.");
	if (this->root_directory.empty())
		throw std::runtime_error("CgiRoutingConfig::validate: root_directory is empty.");
	DIR *dir = opendir(this->root_directory.c_str());
	if (dir == NULL)
	{
		throw std::runtime_error("CgiRoutingConfig::validate: root_directory is not found.");
	}
	else
	{
		closedir(dir);
	}
}

const std::string &CgiRoutingConfig::get_root_directory() const
{
	return this->root_directory;
}

Span CgiRoutingConfig::get_script_name(const Span &path) const
{
	Span::iterator start = path.begin() + this->location.size();
	Span::iterator found = StringHelper::find_first_match(start, path.end(), '/');
	std::size_t size = found - path.begin();
	return path.slice(0, size);
}

std::vector<char> CgiRoutingConfig::get_routed_path(const Span &path) const
{
	std::vector<char> tmp(this->root_directory.begin(), this->root_directory.end());
	Span sliced = path.slice(this->location.size());
	Span::iterator found_slash = StringHelper::find_first_match(sliced.begin(), sliced.end(), '/');
	FileUtility::path_combine(tmp, sliced.begin(), found_slash);
	return tmp;
}

Span CgiRoutingConfig::get_file_name(const Span &path) const
{
	Span sliced = path.slice(this->location.size());
	Span::iterator found_slash = StringHelper::find_first_match(sliced.begin(), sliced.end(), '/');
	return sliced.slice(0, found_slash - sliced.begin());
}

Span CgiRoutingConfig::get_path_info(const Span &path) const
{
	Span sliced = path.slice(this->location.size());
	Span::iterator found = StringHelper::find_first_match(sliced.begin(), sliced.end(), '/');
	return sliced.slice(found - sliced.begin());
}

bool CgiRoutingConfig::prepare(const EnvironmentVariableDictionary &dictionary)
{
	this->actual_executor = this->executor.has_value ? this->executor.value : "";
	if (!this->executor.has_value)
		return true;
	const std::string &executor = this->executor.value;
	if (::access(executor.c_str(), X_OK) == 0)
		return true;
	std::map<std::string, std::string>::const_iterator found = dictionary.find("PATH");
	if (found == dictionary.end())
	{
		std::cerr << "Error: CgiRoutingConfig::prepare. Environment variable 'PATH' does not exist." << std::endl;
		return false;
	}
	std::vector<char> buffer;
	for (std::string::const_iterator itr = found->second.begin(); itr != found->second.end();)
	{
		std::string::const_iterator found_colon = StringHelper::find_first_match(itr, found->second.end(), ':');
		if (found_colon == itr)
		{
			itr++;
			continue;
		}
		buffer.clear();
		buffer.insert(buffer.end(), itr, found_colon);
		FileUtility::path_combine(buffer, executor);
		buffer.push_back('\0');
		if (::access(buffer.data(), X_OK) == 0)
		{
			this->actual_executor = std::string(buffer.data());
			return true;
		}
		if (found_colon == found->second.end())
			break;
		itr = found_colon + 1;
	}
	std::cerr << "Error: CgiRoutingConfig::prepare. There is no executor '" << executor << "' in this machine." << std::endl;
	return false;
}
