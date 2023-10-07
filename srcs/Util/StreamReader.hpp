#ifndef STREAMREADER_HPP
#define STREAMREADER_HPP

#include "Config/Config.hpp"
#include "Config/ServerConfig.hpp"
#include "Config/Routing/CgiRoutingConfig.hpp"
#include "Config/Routing/RedirectionRoutingConfig.hpp"
#include "Config/Routing/StaticRoutingConfig.hpp"
#include "Util/Either.hpp"
#include "Util/HttpRedirection.hpp"
#include "Util/HttpStatusCode.hpp"
#include "Util/Nullable.hpp"
#include "Util/MediaTypeDictionary.hpp"
#include <iterator>
#include <string>
#include <map>
#include <set>
#include <vector>
#include <inttypes.h>

class StreamReader
{
private:
	StreamReader();

public:
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpMethod &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpStatusCode &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpRedirection &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, HttpRedirection::Variable &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, unsigned long long int &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, unsigned long int &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, unsigned int &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, uint8_t &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, bool &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, char &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::string &value);
	static void read_to_lower(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::string &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Nullable<std::string> &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, TimeSpan &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, MediaTypeDictionary &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Config &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, ServerConfig &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, ErrorPage &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Nullable<ErrorPage> &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, RoutingConfigRef&value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, CgiRoutingConfig &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, RedirectionRoutingConfig &value);
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, StaticRoutingConfig &value);

	template <typename T0, typename T1>
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, Either<T0, T1> &value)
	{
		StreamReader::read(itr, end, value.is_item0);
		if (value.is_item0)
		{
			StreamReader::read(itr, end, value.item0);
		}
		else
		{
			StreamReader::read(itr, end, value.item1);
		}
	}

	template <typename TItem>
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::set<TItem> &value)
	{
		uint32_t size;
		StreamReader::read(itr, end, size);
		value.clear();
		for (uint32_t i = 0; i < size; i++)
		{
			TItem item;
			StreamReader::read(itr, end, item);
			value.insert(item);
		}
	}

	template <typename TItem>
	static void read_to_lower(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::set<TItem> &value)
	{
		uint32_t size;
		StreamReader::read(itr, end, size);
		value.clear();
		for (uint32_t i = 0; i < size; i++)
		{
			TItem item;
			StreamReader::read_to_lower(itr, end, item);
			value.insert(item);
		}
	}

	template <typename TItem>
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::vector<TItem> &value)
	{
		uint32_t size;
		StreamReader::read(itr, end, size);
		value.clear();
		value.reserve(size);
		for (uint32_t i = 0; i < size; i++)
		{
			TItem item;
			value.push_back(item);
			StreamReader::read(itr, end, value.back());
		}
	}

	template <typename TItem>
	static void read_to_lower(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::vector<TItem> &value)
	{
		uint32_t size;
		StreamReader::read(itr, end, size);
		value.clear();
		value.reserve(size);
		for (uint32_t i = 0; i < size; i++)
		{
			TItem item;
			value.push_back(item);
			StreamReader::read_to_lower(itr, end, value.back());
		}
	}

	template <typename TKey, typename TValue>
	static void read(std::vector<char>::const_iterator &itr, const std::vector<char>::const_iterator &end, std::map<TKey, TValue> &value)
	{
		uint32_t size;
		StreamReader::read(itr, end, size);
		value.clear();
		std::pair<TKey, TValue> pair;
		for (uint32_t i = 0; i < size; i++)
		{
			StreamReader::read(itr, end, pair.first);
			StreamReader::read(itr, end, pair.second);
			value.insert(pair);
		}
	}
};

#endif