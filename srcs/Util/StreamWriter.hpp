#ifndef STREAMWRITER_HPP
#define STREAMWRITER_HPP

#include <stdint.h>
#include <fstream>
#include <string>
#include <map>
#include <set>
#include <vector>
#include "Util/HttpStatusCode.hpp"
#include "Util/HttpRedirection.hpp"
#include "Util/Nullable.hpp"
#include "Util/MediaTypeDictionary.hpp"
#include "Config/Config.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include "Config/Routing/StaticRoutingConfig.hpp"

class StreamWriter
{
private:
	StreamWriter();

public:
	static void write(std::ostream &stream, const unsigned long long int &value);
	static void write(std::ostream &stream, const unsigned long int &value);
	static void write(std::ostream &stream, const unsigned int &value);
	static void write(std::ostream &stream, const uint8_t &value);
	static void write(std::ostream &stream, const bool &value);
	static void write(std::ostream &stream, const char &value);
	static void write(std::ostream &stream, const std::string &value);
	static void write(std::ostream &stream, const char *value);
	static void write_to_lower(std::ostream &stream, const std::string &value);
	static void write(std::ostream &stream, const Nullable<std::string> &value);
	static void write(std::ostream &stream, const HttpStatusCode &value);
	static void write(std::ostream &stream, const HttpRedirection &value);
	static void write(std::ostream &stream, const HttpRedirection::Variable &value);
	static void write(std::ostream &stream, const HttpMethod &value);
	static void write(std::ostream &stream, const TimeSpan &value);
	static void write(std::ostream &stream, const MediaTypeDictionary &value);
	static void write(std::ostream &stream, const Config &value);
	static void write(std::ostream &stream, const ServerConfig &value);
	static void write(std::ostream &stream, const ErrorPage &value);
	static void write(std::ostream &stream, const Nullable<ErrorPage> &value);
	static void write(std::ostream &stream, RoutingConfig const *const &value);
	static void write(std::ostream &stream, const CgiRoutingConfig &value);
	static void write(std::ostream &stream, const RedirectionRoutingConfig &value);
	static void write(std::ostream &stream, const StaticRoutingConfig &value);

	template <typename T0, typename T1>
	static void write(std::ostream &stream, const Either<T0, T1> &value)
	{
		StreamWriter::write(stream, value.is_item0);
		if (value.is_item0)
		{
			StreamWriter::write(stream, value.item0);
		}
		else
		{
			StreamWriter::write(stream, value.item1);
		}
	}

	template <typename T>
	static void write(std::ostream &stream, const std::vector<T> &value)
	{
		if (value.size() > UINT32_MAX)
			throw std::out_of_range("value.size if out of range uint32_t max value.");
		StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
		for (typename std::vector<T>::const_iterator itr = value.begin(); itr != value.end(); itr++)
		{
			StreamWriter::write(stream, *itr);
		}
	}

	template <typename T>
	static void write_to_lower(std::ostream &stream, const std::vector<T> &value)
	{
		if (value.size() > UINT32_MAX)
			throw std::out_of_range("value.size if out of range uint32_t max value.");
		StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
		for (typename std::vector<T>::const_iterator itr = value.begin(); itr != value.end(); itr++)
		{
			StreamWriter::write_to_lower(stream, *itr);
		}
	}

	template <typename T>
	static void write(std::ostream &stream, const std::set<T> &value)
	{
		if (value.size() > UINT32_MAX)
			throw std::out_of_range("value.size if out of range uint32_t max value.");
		StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
		for (typename std::set<T>::const_iterator itr = value.begin(); itr != value.end(); itr++)
		{
			StreamWriter::write(stream, *itr);
		}
	}

	template <typename T>
	static void write_to_lower(std::ostream &stream, const std::set<T> &value)
	{
		if (value.size() > UINT32_MAX)
			throw std::out_of_range("value.size if out of range uint32_t max value.");
		StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
		for (typename std::set<T>::const_iterator itr = value.begin(); itr != value.end(); itr++)
		{
			StreamWriter::write_to_lower(stream, *itr);
		}
	}

	template <typename TKey, typename TValue>
	static void write(std::ostream &stream, const std::map<TKey, TValue> &value)
	{
		if (value.size() > UINT32_MAX)
			throw std::out_of_range("value.size if out of range uint32_t max value.");
		StreamWriter::write(stream, static_cast<uint32_t>(value.size()));
		for (typename std::map<TKey, TValue>::const_iterator itr = value.begin(); itr != value.end(); itr++)
		{
			StreamWriter::write(stream, itr->first);
			StreamWriter::write(stream, itr->second);
		}
	}
};

#endif