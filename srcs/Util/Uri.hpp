#ifndef URI_HPP
#define URI_HPP

#include "Util/Span.hpp"
#include <vector>
#include <deque>

class Uri
{
private:
	std::string value;
	Span domain;
	Span userinfo;
	Span port;
	Span path;
	Span parameter;
	Span fragment;

	bool is_valid_port();
	bool is_valid_domain();
	bool is_valid_path();
	bool is_valid_parameter();
	bool is_valid_fragment();
	bool try_parse_domain();

	bool try_parse_parameter_and_fragment();

public:
	Uri();
	Uri(const Uri &right);
	~Uri();
	Uri &operator=(const Uri &right);

	static bool try_parse(const std::string &input, Uri &output);
	static bool try_parse_start_from_path(const std::string &input, Uri &output);

	const std::string &get_value() const;
	const Span &get_domain() const;
	const Span &get_port() const;
	const Span &get_path() const;
	const Span &get_parameter() const;
	const Span &get_anchor() const;

	/// @return false: Invalid Path
	bool append_normalize_path(std::vector<char> &buffer) const;
	static void decode_path(std::vector<char> &buffer);

private:
	static int calculate_hex_digit(char c0);
	static int calculate_hex_digits(char c0, char c1);
	static bool is_uri_reserved_char(int c);
	static bool is_not_appropriate_percent_encoding(int c);
	static void push_char(std::vector<char> &buffer, int c);

	template <typename TIterator>
	static bool normalize_url_encode(TIterator itr, TIterator end, std::vector<char> &buffer)
	{
		for (; itr != end; itr++)
		{
			char c = *itr;
			if (c == '%')
			{
				if (++itr == end)
					return false;
				char c0 = *itr;
				if (!std::isxdigit(c0))
					return false;
				if (++itr == end)
					return false;
				char c1 = *itr;
				if (!std::isxdigit(c1))
					return false;
				int value = calculate_hex_digits(c0, c1);
				push_char(buffer, value);
			}
			else
			{
				if (is_uri_reserved_char(c))
					buffer.push_back(c);
				else
					push_char(buffer, c);
			}
		}
		return true;
	}

	template <typename TIterator>
	static void divide_path_segment(TIterator itr, TIterator end, std::deque<std::vector<char> > &buffer)
	{
		for (; itr != end; itr++)
		{
			char c = *itr;
			if (c == '/')
				buffer.push_back(std::vector<char>());
			std::vector<char> &last = buffer.back();
			last.push_back(c);
		}
	}

	template <typename TIterator>
	static void combine_path_segment(TIterator itr, TIterator end, std::vector<char> &buffer)
	{
		if (itr == end)
		{
			buffer.push_back('/');
			return;
		}
		for (; itr != end; itr++)
		{
			buffer.insert(buffer.end(), itr->begin(), itr->end());
		}
	}

	static bool is_single_dot_path_segment(const std::vector<char> &buffer);
	static bool is_double_dot_path_segment(const std::vector<char> &buffer);
	static void normalize_path_segment(std::deque<std::vector<char> > &buffer);
};

bool is_c0_control(int c);
bool is_c0_control_or_space(int c);
bool is_control(int c);
bool is_forbidden_host_codepoint(int c);
bool is_forbidden_domain_codepoint(int c);

#endif