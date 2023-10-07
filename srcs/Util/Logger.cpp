#include "Util/Logger.hpp"
#include "Util/TimeSpan.hpp"
#include <string.h>

void Logger::log_error(const std::exception &exception)
{
	std::cerr << "Error: " << exception.what() << " " << current_time << std::endl;
}

void Logger::log_error(const char *text, int error_code)
{
	std::cerr << "Error: " << text << " " << strerror(error_code) << " " << current_time << std::endl;
}

void Logger::log_error(const std::string &text, int error_code)
{
	std::cerr << "Error: " << text << " " << strerror(error_code) << " " << current_time << std::endl;
}

void Logger::log_debug(const std::string &text, const char *file, size_t line)
{
#ifdef DEBUG
	std::cerr << "Debug: " << text << " File: " << file << " Line: " << line << " " << current_time << std::endl;
#else
	(void)text;
	(void)file;
	(void)line;
#endif
}

void Logger::log_info(const std::string &text)
{
	std::cerr << "Info: " << text << " " << current_time << std::endl;
}
