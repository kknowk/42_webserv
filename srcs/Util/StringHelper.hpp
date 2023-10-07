#ifndef STRINGHELPER
#define STRINGHELPER

#include <string>
#include <vector>
#include <cstdlib>
#include <cstring>
#include <ctime>

class StringHelper
{
private:
	StringHelper();

public:
	static void append(std::vector<char> &buffer, const std::string &text);
	static void append(std::vector<char> &buffer, const char *text);
	static void append(std::vector<char> &buffer, const char *text, std::size_t size);
	static void append(std::vector<char> &buffer, int value);
	static void append(std::vector<char> &buffer, std::size_t value);
	static void append(std::vector<char> &buffer, const std::time_t &value);
	static void append_line(std::vector<char> &buffer);
	static void append_html(std::vector<char> &buffer, const std::string &text);

	template <typename TSourceIterator>
	static void append_html(std::vector<char> &buffer, TSourceIterator start, TSourceIterator end)
	{
		for (; start != end; start++)
		{
			char c = *start;
			if (std::isalnum(c) || c == '_' || c == '-' || c == '/')
			{
				buffer.push_back(c);
				continue;
			}
			if (c == ' ')
			{
				buffer.push_back('&');
				buffer.push_back('n');
				buffer.push_back('b');
				buffer.push_back('s');
				buffer.push_back('p');
				buffer.push_back(';');
				continue;
			}
			unsigned int uc = static_cast<unsigned char>(c);
			unsigned int higher = ((uc >> 4) & 0xfU);
			unsigned int lower = uc & 0xfU;
			buffer.push_back('&');
			buffer.push_back('#');
			buffer.push_back('x');
			buffer.push_back((higher >= 10 ? ('A' - 10) : '0') + higher);
			buffer.push_back((lower >= 10 ? ('A' - 10) : '0') + lower);
			buffer.push_back(';');
		}
	}

	static char *strdup(const char *value);

	template <typename TSourceIterator, typename TMatchIterator>
	static TSourceIterator find_first_match(TSourceIterator source_start, TSourceIterator source_end, TMatchIterator match_start, TMatchIterator match_end)
	{
		for (; source_start != source_end; source_start++)
		{
			TSourceIterator current_source = source_start;
			TMatchIterator current_match = match_start;
			for (; current_match != match_end && current_source != source_end; current_match++, current_source++)
			{
				if ((*current_match) != (*current_source))
					goto FAIL;
			}
			if (current_match == match_end)
				return source_start;
		FAIL:;
		}
		return source_end;
	}

	template <typename TSourceIterator, typename TMatchIterator>
	static TSourceIterator find_first_match_any(TSourceIterator source_start, TSourceIterator source_end, TMatchIterator match_start, TMatchIterator match_end)
	{
		for (; source_start != source_end; source_start++)
		{
			for (TMatchIterator match_itr = match_start; match_itr != match_end; match_itr++)
			{
				if (*source_start == *match_itr)
					return source_start;
			}
		}
		return source_end;
	}

	template <typename TSourceIterator>
	static TSourceIterator find_first_match(TSourceIterator source_start, TSourceIterator source_end, char match)
	{
		for (; source_start != source_end; source_start++)
		{
			if (*source_start == match)
				return source_start;
		}
		return source_end;
	}

	template <typename TSourceIterator>
	static TSourceIterator find_last(TSourceIterator source_start, TSourceIterator source_end, char match)
	{
		TSourceIterator answer = source_end;
		for (; source_start != source_end; source_start++)
		{
			if (*source_start == match)
				answer = source_start;
		}
		return answer;
	}

	template <typename TSourceIterator>
	static bool starts_with(TSourceIterator source_start, TSourceIterator source_end, const char *match)
	{
		if (match == NULL)
			return false;
		for (; source_start != source_end && *match != '\0'; source_start++, match++)
		{
			if (*source_start != *match)
				return false;
		}
		return *match == '\0';
	}

	template <typename TSourceIterator>
	static bool ends_with(TSourceIterator source_start, TSourceIterator source_end, const char *match)
	{
		size_t size = std::strlen(match);
		TSourceIterator sub = source_end - size;
		if (sub < source_start)
			return false;
		return StringHelper::equals(sub, source_end, match);
	}

	template <typename TSourceIterator>
	static bool equals(TSourceIterator source_start, TSourceIterator source_end, const char *match)
	{
		if (match == NULL)
			return false;
		for (; source_start != source_end && *match != '\0'; source_start++, match++)
		{
			if (*source_start != *match)
				return false;
		}
		return *match == '\0' && source_start == source_end;
	}

	template <typename TSourceIterator>
	static bool equals_ignore_case(TSourceIterator source_start, TSourceIterator source_end, const char *match)
	{
		if (match == NULL)
			return false;
		for (; source_start != source_end && *match != '\0'; source_start++, match++)
		{
			char left = *source_start, right = *match;
			if (left == right)
				continue;
			if (std::tolower(left) != std::tolower(right))
				return false;
		}
		return *match == '\0' && source_start == source_end;
	}

	template <typename TSourceIterator>
	static TSourceIterator trim_start(TSourceIterator source_start, TSourceIterator source_end, char value)
	{
		while (source_start != source_end)
		{
			if (*source_start != value)
				break;
			source_start++;
		}
		return source_start;
	}

	template <typename TSourceIterator, typename TMatchIterator>
	static TSourceIterator trim_start_any(TSourceIterator source_start, TSourceIterator source_end, TMatchIterator match_start, TMatchIterator match_end)
	{
		while (source_start != source_end)
		{
			if (StringHelper::find_first_match(match_start, match_end, *source_start) == match_end)
				break;
			source_start++;
		}
		return source_start;
	}

	template <typename TSourceIterator>
	static TSourceIterator trim_end(TSourceIterator source_start, TSourceIterator source_end, char value)
	{
		while (source_start != source_end)
		{
			--source_end;
			if (*source_end != value)
				return ++source_end;
		}
		return source_start;
	}

	template <typename TSourceIterator, typename TMatchIterator>
	static TSourceIterator trim_end_any(TSourceIterator source_start, TSourceIterator source_end, TMatchIterator match_start, TMatchIterator match_end)
	{
		while (source_start != source_end)
		{
			--source_end;
			if (StringHelper::find_first_match(match_start, match_end, *source_end) == match_end)
				return ++source_end;
		}
		return source_start;
	}

	template <typename TSourceIterator>
	static std::string to_lower_string(TSourceIterator source_start, TSourceIterator source_end)
	{
		std::string answer(source_end - source_start, ' ');
		for (std::size_t index = 0; source_start != source_end; source_start++, index++)
		{
			answer[index] = std::tolower(*source_start);
		}
		return answer;
	}

	template <typename TSourceIterator>
	static TSourceIterator find_first_newline(TSourceIterator source_start, TSourceIterator source_end, std::size_t &size)
	{
		TSourceIterator found = StringHelper::find_first_match(source_start, source_end, '\n');
		if (found == source_end)
		{
			size = 0;
			return source_end;
		}
		size = 1;
		if (found != source_start)
		{
			if ((--found).operator*() == '\r')
				size = 2;
			else
				++found;
		}
		return found;
	}

	template <typename TSourceIterator>
	static bool all_valid_ascii_char_in_http_request_header_line(TSourceIterator start, TSourceIterator end)
	{
		for (; start != end; start++)
		{
			char c = *start;
			if (c == '\t')
				continue;
			if (std::isprint(c))
				continue;
			return false;
		}
		return true;
	}
};

#endif