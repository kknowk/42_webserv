#include "Server/EventHandler.hpp"

void EventHandler::add_listener(int socket_fd, const std::string &ip, const std::string &port)
{
	std::pair<std::map<int, ListenerSocket>::iterator, bool> pair = this->listen_sockets.insert(std::make_pair(socket_fd, ListenerSocket(socket_fd, ip, port)));
	pair.first->second.force_dispose();
}

void EventHandler::add_connection(int connection_fd, int base_socket_fd, const sockaddr_storage &address)
{
	std::pair<std::map<int, Connection>::iterator, bool> pair = this->connections.insert(std::pair<int, Connection>(connection_fd, Connection(connection_fd, base_socket_fd, address, this)));
	pair.first->second.force_dispose();
}

void EventHandler::add_write_resource_pipe(int resource_fd, Connection &connection)
{
	std::pair<int, WritableResource> pair(resource_fd, WritableResource(resource_fd, connection.get_fd(), true));
	std::map<int, WritableResource>::iterator found = this->write_resources.find(resource_fd);
	if (found == this->write_resources.end())
		this->write_resources.insert(pair).first->second.force_dispose();
	else if (found->second.get_connection_fd() == connection.get_fd())
		return;
	else
	{
		found->second = pair.second;
		found->second.force_dispose();
	}
	connection.set_write_resource_fd(resource_fd);
}

void EventHandler::add_read_resource_pipe(int resource_fd, Connection &connection)
{
	std::pair<int, ReadableResource> pair(resource_fd, ReadableResource(resource_fd, connection.get_fd(), true));
	std::map<int, ReadableResource>::iterator found = this->read_resources.find(resource_fd);
	if (found == this->read_resources.end())
		this->read_resources.insert(pair).first->second.force_dispose();
	else if (found->second.get_connection_fd() == connection.get_fd())
		return;
	else
	{
		found->second = pair.second;
		found->second.force_dispose();
	}
	connection.set_read_resource_fd(resource_fd);
}

void EventHandler::add_write_resource_file(int resource_fd, Connection &connection)
{
	if (resource_fd < 0)
		return;
	std::pair<int, WritableResource> pair(resource_fd, WritableResource(resource_fd, connection.get_fd(), false));
	std::map<int, WritableResource>::iterator found = this->write_resources.find(resource_fd);
	if (found == this->write_resources.end())
		this->write_resources.insert(pair).first->second.force_dispose();
	else if (found->second.get_connection_fd() == connection.get_fd())
		return;
	else
	{
		found->second = pair.second;
		found->second.force_dispose();
	}
	connection.set_write_resource_fd(resource_fd);
}

void EventHandler::add_read_resource_file(int resource_fd, Connection &connection)
{
	if (resource_fd < 0)
		return;
	std::pair<int, ReadableResource> pair(resource_fd, ReadableResource(resource_fd, connection.get_fd(), false));
	std::map<int, ReadableResource>::iterator found = this->read_resources.find(resource_fd);
	if (found == this->read_resources.end())
		this->read_resources.insert(pair).first->second.force_dispose();
	else if (found->second.get_connection_fd() == connection.get_fd())
		return;
	else
	{
		found->second = pair.second;
		found->second.force_dispose();
	}
	connection.set_read_resource_fd(resource_fd);
}
