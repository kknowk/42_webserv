#ifndef CGIROUTINGCONFIG_HPP
#define CGIROUTINGCONFIG_HPP

#include "Config/Routing/RoutingConfig.hpp"
#include "Util/Either.hpp"
#include "Util/Span.hpp"
#include "Util/EnvironmentVariableDictionary.hpp"

class CgiRoutingConfig : public RoutingConfig
{
#ifdef TOOL
public:
#else
private:
#endif
	std::string root_directory;
	Either<std::string, bool> either_index_or_listing;
	Nullable<std::string> location_suffix;
	Nullable<std::string> executor;

	std::string actual_executor;

public:
	CgiRoutingConfig();
	virtual ~CgiRoutingConfig();

	virtual bool is_cgi_router() const;
	virtual bool is_static_file_router() const;
	virtual bool is_redirection_router() const;
	virtual ConfigMatchingResult is_match(const Span &path, const HttpMethod &method) const;

	virtual void validate() const;

	const Nullable<std::string> &get_location_suffix() const;
	const std::string &get_executor() const;
	const Either<std::string, bool> &get_either_index_or_listing() const;
	const std::string &get_root_directory() const;
	bool handle_single_executable() const;

	virtual bool prepare(const EnvironmentVariableDictionary &dictionary);

	std::vector<char> get_routed_path(const Span &path) const;
	Span get_file_name(const Span &path) const;
	Span get_path_info(const Span &path) const;
	Span get_script_name(const Span &path) const;

#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

#endif