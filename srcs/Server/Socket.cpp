#include "Server/Socket.hpp"
#include "Util/Logger.hpp"
#include "Util/FileUtility.hpp"
#include <cstring>
#include <unistd.h>
#include <cerrno>
#include <netinet/in.h>
#include <arpa/inet.h>

Socket::Socket(int fd)
	: should_close(false), fd(fd)
{
}

Socket::Socket()
	: should_close(false), fd(-1)
{
}

Socket::~Socket()
{
	if (should_close)
		this->close();
}

void Socket::close()
{
	if (fd < 0)
		return;
	::close(this->fd);
	this->fd = -1;
}

int Socket::get_fd() const
{
	return this->fd;
}

int Socket::create_listener(const std::string &ip, const std::string &port, bool is_ipv4)
{
	int socket_fd = -1;
	// https://linuxjm.osdn.jp/html/LDP_man-pages/man3/getaddrinfo.3.html
	struct addrinfo hints;
	std::memset(&hints, 0, sizeof(struct addrinfo));
	hints.ai_family = is_ipv4 ? AF_INET : AF_INET6;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;

	struct addrinfo *results;
	std::cerr << "Info: Create Listener. Host: " << ip << " Port: " << port << " " << current_time << std::endl;
	if (::getaddrinfo(ip.c_str(), port.c_str(), &hints, &results))
	{
		Logger::log_error(std::string("getaddrinfo Host: ") + ip + " Port: " + port, errno);
		return -1;
	}
	struct addrinfo *result_iterator = results;
	for (; result_iterator != NULL; result_iterator = result_iterator->ai_next)
	{
		// https://linuxjm.osdn.jp/html/LDP_man-pages/man2/socket.2.html
#if defined(SOCK_CLOEXEC) && defined(SOCK_NONBLOCK)
		socket_fd = ::socket(result_iterator->ai_family, result_iterator->ai_socktype | SOCK_CLOEXEC | SOCK_NONBLOCK, result_iterator->ai_protocol);
		if (socket_fd < 0)
			continue;
#else
		socket_fd = ::socket(result_iterator->ai_family, result_iterator->ai_socktype, result_iterator->ai_protocol);
		if (socket_fd < 0)
			continue;
		if (!FileUtility::add_close_on_exec_and_nonblock_flag(socket_fd))
		{
			::close(socket_fd);
			socket_fd = -1;
			continue;
		}
#endif
		// https://qiita.com/on-keyday/items/5b7415f0f887578ce0a9#setsockopt
		u_long yes = 1;
		if (::setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)))
		{
			Logger::log_error("setsockopt SO_REUSEADDR", errno);
			::close(socket_fd);
			socket_fd = -1;
			continue;
		}
		if (!is_ipv4)
		{
			if (::setsockopt(socket_fd, IPPROTO_IPV6, IPV6_V6ONLY, &yes, sizeof(yes)))
			{
				Logger::log_error("setsockopt IPV6_V6ONLY", errno);
				::close(socket_fd);
				socket_fd = -1;
				continue;
			}
		}
		// https://linuxjm.osdn.jp/html/LDP_man-pages/man2/bind.2.html
		if (::bind(socket_fd, result_iterator->ai_addr, result_iterator->ai_addrlen))
		{
			std::cerr << "Error: bind to " << ip << ":" << port << " failed. Reason: " << strerror(errno) << " " << current_time << std::endl;
			::close(socket_fd);
			socket_fd = -1;
			continue;
		}
		break;
	}
	if (result_iterator == NULL)
	{
		Logger::log_error("all sockets are invalid.", errno);
		::close(socket_fd);
		return -1;
	}
	::freeaddrinfo(results);
	// https://linuxjm.osdn.jp/html/LDP_man-pages/man2/listen.2.html
	if (listen(socket_fd, SOMAXCONN))
	{
		Logger::log_error("listen", errno);
		::close(socket_fd);
		socket_fd = -1;
	}
	return socket_fd;
}

bool Socket::operator==(const Socket &right) const
{
	return this->fd == right.fd;
}

bool Socket::operator!=(const Socket &right) const
{
	return this->fd != right.fd;
}

bool Socket::operator<(const Socket &right) const
{
	return this->fd < right.fd;
}

bool Socket::operator>(const Socket &right) const
{
	return this->fd > right.fd;
}

bool Socket::operator<=(const Socket &right) const
{
	return this->fd <= right.fd;
}

bool Socket::operator>=(const Socket &right) const
{
	return this->fd >= right.fd;
}

bool Socket::operator==(const int &right) const
{
	return this->fd == right;
}
bool Socket::operator!=(const int &right) const
{
	return this->fd != right;
}
bool Socket::operator<(const int &right) const
{
	return this->fd < right;
}
bool Socket::operator>(const int &right) const
{
	return this->fd > right;
}
bool Socket::operator<=(const int &right) const
{
	return this->fd <= right;
}
bool Socket::operator>=(const int &right) const
{
	return this->fd >= right;
}

IoResult Socket::read(std::vector<char> &buffer) const
{
	return FileUtility::read(this->fd, buffer);
}

std::pair<IoResult, std::size_t> Socket::write(const void *data, std::size_t size) const
{
	return FileUtility::write(this->fd, data, size);
}

void Socket::force_dispose()
{
	this->should_close = true;
}
