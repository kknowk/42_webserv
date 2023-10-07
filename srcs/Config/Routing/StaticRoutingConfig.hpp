#ifndef STATICROUTINGCONFIG_HPP
#define STATICROUTINGCONFIG_HPP

#include "Config/Routing/RoutingConfig.hpp"
#include "Util/Either.hpp"
#include "Util/MediaTypeDictionary.hpp"

class StaticRoutingConfig : public RoutingConfig
{
#ifdef TOOL
public:
#else
private:
#endif
	MediaTypeDictionary dictionary;
	std::string root_directory;
	Either<std::string, bool> either_index_or_listing;
	bool _accept_file_editing;
public:
	StaticRoutingConfig();
	virtual ~StaticRoutingConfig();

	virtual bool is_redirection_router() const;
	virtual bool is_cgi_router() const;
	virtual bool is_static_file_router() const;

	virtual ConfigMatchingResult is_match(const Span &path, const HttpMethod &method) const;

	virtual void validate() const;
	virtual bool prepare(const EnvironmentVariableDictionary &dictionary);

	std::vector<char> get_routed_path(const Span &path) const;

	const MediaTypeDictionary &get_dictionary() const;
	const Either<std::string, bool> &get_either_index_or_listing() const;

	bool accept_file_editing() const;

#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

#endif