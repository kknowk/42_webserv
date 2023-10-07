#include "Server/ListenerSocket.hpp"

ListenerSocket::ListenerSocket(int socket_fd, const std::string &ip, const std::string &port)
	: Socket(socket_fd), ip(ip), port(port)
{
}

const std::string &ListenerSocket::get_ip() const
{
	return this->ip;
}

const std::string &ListenerSocket::get_port() const
{
	return this->port;
}
