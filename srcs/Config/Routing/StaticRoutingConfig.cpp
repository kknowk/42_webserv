#include "Config/Routing/StaticRoutingConfig.hpp"
#include "Util/FileUtility.hpp"
#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <stdexcept>

StaticRoutingConfig::StaticRoutingConfig()
	: RoutingConfig()
{
}

StaticRoutingConfig::~StaticRoutingConfig()
{
}

bool StaticRoutingConfig::is_cgi_router() const
{
	return false;
}

bool StaticRoutingConfig::is_redirection_router() const
{
	return false;
}

bool StaticRoutingConfig::is_static_file_router() const
{
	return true;
}

ConfigMatchingResult StaticRoutingConfig::is_match(const Span &path, const HttpMethod &method) const
{
	if (!path.starts_with(this->location.c_str()))
		return ConfigMatchingResult_NotFound;
	if (this->methods.find(method) == this->methods.end())
		return ConfigMatchingResult_MethodNotMatch;
	return ConfigMatchingResult_Found;
}

const Either<std::string, bool> &StaticRoutingConfig::get_either_index_or_listing() const
{
	return this->either_index_or_listing;
}

std::vector<char> StaticRoutingConfig::get_routed_path(const Span &path) const
{
	std::vector<char> tmp(this->root_directory.begin(), this->root_directory.end());
	Span sliced = path.slice(this->location.size());
	FileUtility::path_combine(tmp, sliced.begin(), sliced.end());
	if (tmp.back() == '/' && this->either_index_or_listing.is_item0)
	{
		tmp.insert(tmp.end(), this->either_index_or_listing.item0.begin(), this->either_index_or_listing.item0.end());
	}
	return tmp;
}

void StaticRoutingConfig::validate() const
{
	if (this->location.empty())
		throw std::runtime_error("StaticRoutingConfig::validate: location is empty.");
	if (this->methods.empty())
		throw std::runtime_error("StaticRoutingConfig::validate: methods is empty.");
	if (this->root_directory.empty())
		throw std::runtime_error("StaticRoutingConfig::validate: root_directory is empty.");
	DIR *dir = opendir(this->root_directory.c_str());
	if (dir == NULL)
		throw std::runtime_error("StaticRoutingConfig::validate: root_directory is not found.");
	else
		closedir(dir);
	if (this->dictionary.get_default().empty())
		throw std::runtime_error("StaticRoutingConfig::validate: default mime-type must not be empty.");
	if (access(this->root_directory.c_str(), W_OK | R_OK) != 0)
		throw std::runtime_error("StaticRoutingConfig::validate: root_directory is invalid.");
}

bool StaticRoutingConfig::accept_file_editing() const
{
	return this->_accept_file_editing;
}

const MediaTypeDictionary &StaticRoutingConfig::get_dictionary() const
{
	return this->dictionary;
}

bool StaticRoutingConfig::prepare(const EnvironmentVariableDictionary &dictionary)
{
	(void)dictionary;
	return true;
}
