#ifndef HTTPSTATUSCODE_HPP
#define HTTPSTATUSCODE_HPP

#include <iterator>
#include <map>
#include <string>
#include <iostream>

class HttpStatusCode
{
#ifdef TOOL
public:
#else
private:
#endif
	int value;
public:
	HttpStatusCode();
	HttpStatusCode(int value);
	HttpStatusCode(const HttpStatusCode &right);
	~HttpStatusCode();
	HttpStatusCode &operator=(const HttpStatusCode &right);

	int get_value(void) const;
	
	bool operator==(const HttpStatusCode &right) const;
	bool operator!=(const HttpStatusCode &right) const;

	bool operator<(const HttpStatusCode &right) const;
	bool operator>(const HttpStatusCode &right) const;
	bool operator<=(const HttpStatusCode &right) const;
	bool operator>=(const HttpStatusCode &right) const;

	const std::string &get_message() const;

	static void init();

#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif

private:
	static std::map<int, std::string> message_dictionary;
};

std::ostream &operator<<(std::ostream &stream, const HttpStatusCode &value);

#endif