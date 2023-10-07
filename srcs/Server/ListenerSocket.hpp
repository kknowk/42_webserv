#ifndef LISTENERSOCKET_HPP
#define LISTENERSOCKET_HPP

#include "Server/Socket.hpp"
#include <set>

class ListenerSocket : public Socket
{
private:
	std::string ip;
	std::string port;
public:
	ListenerSocket(int socket_fd, const std::string &host, const std::string &port);

	const std::string &get_ip() const;
	const std::string &get_port() const;
};

#endif