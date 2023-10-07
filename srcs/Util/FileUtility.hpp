#ifndef FILEUTILITY_HPP
#define FILEUTILITY_HPP

#include <string>
#include <vector>
#include <utility>
#include "Util/Span.hpp"
#include "Util/IoResult.hpp"

class FileUtility
{
private:
	FileUtility();

public:
	static void path_combine(std::vector<char> &buffer, const std::string &value);
	static std::string path_combine(const std::string &left, const std::string &right);

	template <typename TIterator>
	static void path_combine(std::vector<char> &buffer, TIterator start, TIterator end)
	{
		if (buffer.empty())
		{
			buffer.insert(buffer.end(), start, end);
			return;
		}
		while (!buffer.empty() && buffer.back() == '\0')
		{
			buffer.pop_back();
		}
		if (!buffer.empty() && buffer.back() != '/')
			buffer.push_back('/');
		if (start == end)
			return;
		if (*start == '/')
		{
			buffer.insert(buffer.end(), start + 1, end);
		}
		else
		{
			buffer.insert(buffer.end(), start, end);
		}
	}

	/// @return false: FAIL
	static bool add_close_on_exec_and_nonblock_flag(int fd);

	/// @return false: FAIL
	static bool add_close_on_exec_flag(int fd);

	/// @return false: FAIL
	static bool add_nonblock_flag(int fd);

	static bool is_valid_file_descriptor(int fd);

	static IoResult read(int fd, std::vector<char> &buffer);
	static std::pair<IoResult, std::size_t> write(int fd, const void *data, std::size_t size);
	static std::pair<IoResult, std::size_t> write(int fd, const std::vector<char> &buffer);
	static std::pair<IoResult, std::size_t> write(int fd, const std::vector<char> &buffer, std::size_t offset);
};

#endif