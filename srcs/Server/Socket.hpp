#ifndef SOCKET_HPP
#define SOCKET_HPP

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <unistd.h>
#include <vector>
#include <set>
#include "Util/Span.hpp"
#include "Util/IoResult.hpp"
#include "Util/TimeSpan.hpp"

class Socket
{
private:
	bool should_close;
	int fd;

	void close();

public:
	Socket();
	Socket(int fd);
	~Socket();
	int get_fd() const;

	IoResult read(std::vector<char> &buffer) const;
	std::pair<IoResult, std::size_t> write(const void *data, std::size_t size) const;

	static int create_listener(const std::string &host, const std::string &port, bool is_ipv4);

	void force_dispose();

	bool operator==(const Socket &right) const;
	bool operator!=(const Socket &right) const;
	bool operator<(const Socket &right) const;
	bool operator>(const Socket &right) const;
	bool operator<=(const Socket &right) const;
	bool operator>=(const Socket &right) const;

	bool operator==(const int &right) const;
	bool operator!=(const int &right) const;
	bool operator<(const int &right) const;
	bool operator>(const int &right) const;
	bool operator<=(const int &right) const;
	bool operator>=(const int &right) const;
};

#endif