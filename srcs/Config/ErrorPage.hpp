#ifndef ERRORPAGE_HPP
#define ERRORPAGE_HPP

#include <string>

class ErrorPage
{
public:
	ErrorPage();
	ErrorPage(const std::string &path);
	ErrorPage(const std::string &path, const std::string &media_type);
	std::string path;
	std::string media_type;
};

#endif