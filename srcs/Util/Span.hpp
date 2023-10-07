#ifndef SPAN_HPP
#define SPAN_HPP

#include <string>

class Span
{
private:
	const std::string &value;
	std::string::size_type offset;
	std::string::size_type length;

	static const std::string empty;

public:
	Span();
	Span(const std::string &value);
	Span(const std::string &value, std::string::size_type offset);
	Span(const std::string &value, std::string::size_type offset, std::string::size_type length);
	Span(const Span &right);
	Span(const Span &right, std::string::size_type offset);
	Span(const Span &right, std::string::size_type offset, std::string::size_type length);
	~Span();
	Span &operator=(const Span &right);

	char operator[](std::string::size_type index) const;
	
	typedef std::string::const_iterator iterator;

	iterator begin() const;
	iterator end() const;
	std::string::size_type size() const;
	
	Span rebase(const std::string& value) const;
	
	bool operator==(const std::string &right) const;
	bool operator!=(const std::string &right) const;
	bool operator==(const Span &right) const;
	bool operator!=(const Span &right) const;

	std::string to_string() const;
	bool contains(char c) const;
	std::string::size_type	find(char c) const;
	std::string::size_type	rfind(char c) const;
	bool starts_with(const char *str) const;
	bool ends_with(const char *str) const;
	
	Span slice(std::string::size_type offset) const;
	Span slice(std::string::size_type offset, std::string::size_type length) const;
};

#endif