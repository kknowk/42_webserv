#ifndef LOGGER_HPP
#define LOGGER_HPP

#include <stdexcept>
#include <iostream>
#include <ios>
#include <cerrno>

class Logger
{
private:
	Logger();
public:
	static void log_error(const std::exception &exception);
	static void log_error(const char *text, int error_code);
	static void log_error(const std::string &text, int error_code);
	static void log_debug(const std::string &text, const char *file, size_t line);

	static void log_info(const std::string &text);
};

#endif