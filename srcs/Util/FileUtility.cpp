#include "Util/FileUtility.hpp"
#include "Util/Logger.hpp"
#include <fcntl.h>
#include <unistd.h>

FileUtility::FileUtility()
{
}

void FileUtility::path_combine(std::vector<char> &buffer, const std::string &value)
{
	if (value.empty())
		return;
	if (buffer.empty())
	{
		buffer.insert(buffer.end(), value.begin(), value.end());
		return;
	}
	while (!buffer.empty() && buffer.back() == '\0')
	{
		buffer.pop_back();
	}
	if (value[0] == '/')
	{
		buffer.insert(buffer.end(), value.begin(), value.end());
	}
	else if (!buffer.empty() && buffer.back() != '/')
	{
		buffer.push_back('/');
		buffer.insert(buffer.end(), value.begin(), value.end());
	}
	else
	{
		buffer.insert(buffer.end(), value.begin(), value.end());
	}
}

std::string FileUtility::path_combine(const std::string &left, const std::string &right)
{
	if (left.empty())
	{
		return right;
	}
	if (left.at(left.size() - 1) == '/')
	{
		return left + right;
	}
	return left + "/" + right;
}

/// @return false: FAIL
bool FileUtility::add_close_on_exec_flag(int fd)
{
	if (fd < 0)
		return false;
	int flags = ::fcntl(fd, F_GETFD);
	if (flags < 0)
	{
		Logger::log_error("add_close_on_exec_flag getfd.", errno);
		return false;
	}
	if ((flags & FD_CLOEXEC) != 0)
		return true;
	if (::fcntl(fd, F_SETFD, flags | FD_CLOEXEC) == -1)
	{
		Logger::log_error("add_close_on_exec_flag setfd.", errno);
		return false;
	}
	return true;
}

/// @return false: FAIL
bool FileUtility::add_nonblock_flag(int fd)
{
	if (fd < 0)
		return false;
	int flags = ::fcntl(fd, F_GETFL);
	if (flags < 0)
	{
		Logger::log_error("add_nonblock_flag getfd.", errno);
		return false;
	}
	if ((flags & O_NONBLOCK) != 0)
		return true;
	if (::fcntl(fd, F_SETFL, flags | O_NONBLOCK) == -1)
	{
		Logger::log_error("add_nonblock_flag setfd.", errno);
		return false;
	}
	return true;
}

bool FileUtility::add_close_on_exec_and_nonblock_flag(int fd)
{
	return add_nonblock_flag(fd) && add_close_on_exec_flag(fd);
}

bool FileUtility::is_valid_file_descriptor(int fd)
{
	return ::fcntl(fd, F_GETFD) >= 0;
}

IoResult FileUtility::read(int fd, std::vector<char> &buffer)
{
	buffer.resize(buffer.size() + 4096);
	char *destination = buffer.data() + buffer.size() - 4096;
	ssize_t byte_count = ::read(fd, destination, 4096);
	if (byte_count == 0)
	{
		buffer.resize(buffer.size() - 4096);
		return IoResult_Done;
	}
	if (byte_count > 0)
	{
		buffer.resize(buffer.size() - 4096 + static_cast<std::size_t>(byte_count));
		return IoResult_Continue;
	}
	buffer.resize(buffer.size() - 4096);
	// Because of the 42's requirements the errno check after ::read is strictly forbidden.
	/*
	int error_code = errno;
	if (error_code == EAGAIN || error_code == EWOULDBLOCK)
		return IoResult_Continue;
	*/
	return IoResult_Error;
}

std::pair<IoResult, std::size_t> FileUtility::write(int fd, const std::vector<char> &buffer, std::size_t offset)
{
	return FileUtility::write(fd, buffer.data() + offset, buffer.size() >= offset ? buffer.size() - offset : 0);
}

std::pair<IoResult, std::size_t> FileUtility::write(int fd, const std::vector<char> &buffer)
{
	return FileUtility::write(fd, buffer.data(), buffer.size());
}

std::pair<IoResult, std::size_t> FileUtility::write(int fd, const void *data, std::size_t size)
{
	if (fd < 0)
		return std::pair<IoResult, std::size_t>(IoResult_Error, 0);
	if (size == 0)
		return std::make_pair(IoResult_Done, size);
	if (data == NULL)
		return std::pair<IoResult, std::size_t>(IoResult_Error, 0);
	ssize_t byte_count = ::write(fd, data, size);
	if (byte_count >= 0)
	{
		std::size_t unsigned_byte_count = static_cast<std::size_t>(byte_count);
		return std::pair<IoResult, std::size_t>(unsigned_byte_count == size ? IoResult_Done : IoResult_Continue, unsigned_byte_count);
	}
	// Because of the 42's requirements the errno check after ::read is strictly forbidden.
	/*
	int error_code = errno;
	if (error_code == EAGAIN || error_code == EWOULDBLOCK)
		return std::pair<IoResult, std::size_t>(IoResult_Continue, 0);
	*/
	return std::pair<IoResult, std::size_t>(IoResult_Error, 0);
}
