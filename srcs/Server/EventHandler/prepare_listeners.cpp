#include "Server/EventHandler.hpp"

/// @return false: FAIL
bool EventHandler::prepare_listeners()
{
	for (Config::const_iterator itr = config.begin(); itr != config.end(); itr++)
	{
		const std::string &ip = itr->get_ip(), &port = itr->get_port();
		bool is_ipv4 = itr->is_ipv4();
		std::map<int, ListenerSocket>::const_iterator found = this->get_corresponding_listener(ip, port);
		if (found != this->listen_sockets.end())
			continue;
		int socket_fd = Socket::create_listener(ip, port, is_ipv4);
		if (socket_fd < 0)
			return false;
		this->add_listener(socket_fd, ip, port);
	}
	return true;
}
