#include "Server/EventHandler.hpp"
#include <algorithm>

std::map<int, ListenerSocket>::const_iterator EventHandler::get_corresponding_listener(const std::string &ip, const std::string &port) const
{
	for (std::map<int, ListenerSocket>::const_iterator itr = this->listen_sockets.begin(); itr != this->listen_sockets.end(); itr++)
	{
		const ListenerSocket &socket = itr->second;
		if (socket.get_ip() == ip && socket.get_port() == port)
			return itr;
	}
	return this->listen_sockets.end();
}

const std::string &EventHandler::get_connection_ip(const Connection &connection) const
{
	const ListenerSocket &socket = this->get_listener_by_socket_fd(connection.get_base_socket_fd());
	return socket.get_ip();
}

const std::string &EventHandler::get_connection_port(const Connection &connection) const
{
	const ListenerSocket &socket = this->get_listener_by_socket_fd(connection.get_base_socket_fd());
	return socket.get_port();
}

bool EventHandler::is_listener_id(int socket_fd) const
{
	Socket socket(socket_fd);
	std::map<int, ListenerSocket>::const_iterator found = this->listen_sockets.find(socket_fd);
	return found != this->listen_sockets.end();
}

bool EventHandler::is_connection_id(int socket_fd) const
{
	for (std::map<int, Connection>::const_iterator itr = this->connections.begin(); itr != this->connections.end(); itr++)
	{
		if (itr->second.get_fd() == socket_fd)
			return true;
	}
	return false;
}

bool EventHandler::is_write_resource_id(int fd) const
{
	std::map<int, WritableResource>::const_iterator found = this->write_resources.find(fd);
	if (found == this->write_resources.end())
		return false;
	return true;
}

bool EventHandler::is_read_resource_id(int fd) const
{
	std::map<int, ReadableResource>::const_iterator found = this->read_resources.find(fd);
	if (found == this->read_resources.end())
		return false;
	return true;
}

ListenerSocket &EventHandler::get_listener_by_socket_fd(int fd)
{
	return this->listen_sockets.at(fd);
}

const ListenerSocket &EventHandler::get_listener_by_socket_fd(int fd) const
{
	return this->listen_sockets.at(fd);
}

Connection &EventHandler::get_connection_by_connection_fd(int fd)
{
	for (std::map<int, Connection>::iterator itr = this->connections.begin(); itr != this->connections.end(); itr++)
	{
		if (itr->second.get_fd() == fd)
			return itr->second;
	}
	throw std::out_of_range("connection is not found.");
}

Connection &EventHandler::get_connection_by_write_resource_fd(int fd)
{
	WritableResource &resource = this->write_resources.at(fd);
	return this->get_connection_by_connection_fd(resource.get_connection_fd());
}

Connection &EventHandler::get_connection_by_read_resource_fd(int fd)
{
	ReadableResource &resource = this->read_resources.at(fd);
	return this->get_connection_by_connection_fd(resource.get_connection_fd());
}

bool EventHandler::not_empty() const
{
	if (!this->connections.empty())
		return true;
	if (!this->listen_sockets.empty())
		return true;
	if (!this->read_resources.empty())
		return true;
	if (!this->write_resources.empty())
		return true;
	return false;
}

const Config &EventHandler::get_config() const
{
	return this->config;
}

const EnvironmentVariableDictionary &EventHandler::get_dictionary() const
{
	return this->config.get_dictionary();
}

void EventHandler::classify_events(const std::vector<EventHandlerWaitEvent> &events)
{
	this->clear_events();
	for (std::vector<EventHandlerWaitEvent>::const_iterator itr = events.begin(); itr != events.end(); itr++)
	{
		if (itr->is_empty_event())
			continue;
		int fd = itr->get_fd();
		if (this->is_listener_id(fd))
			this->listen_socket_events.push_back(*itr);
		else if (this->is_connection_id(fd))
			this->connection_events.push_back(*itr);
		else if (this->is_read_resource_id(fd))
			this->read_resource_events.push_back(*itr);
		else if (this->is_write_resource_id(fd))
			this->write_resource_events.push_back(*itr);
		else
			continue;
	}
	std::sort(this->listen_socket_events.begin(), this->listen_socket_events.end());
	std::sort(this->connection_events.begin(), this->connection_events.end());
	std::sort(this->read_resource_events.begin(), this->read_resource_events.end());
	std::sort(this->write_resource_events.begin(), this->write_resource_events.end());
}

void EventHandler::clear_events()
{
	this->listen_socket_events.clear();
	this->connection_events.clear();
	this->read_resource_events.clear();
	this->write_resource_events.clear();
}

std::vector<EventHandlerWaitEvent> &EventHandler::get_listen_socket_events()
{
	return this->listen_socket_events;
}

std::vector<EventHandlerWaitEvent> &EventHandler::get_connection_events()
{
	return this->connection_events;
}

std::vector<EventHandlerWaitEvent> &EventHandler::get_read_resource_events()
{
	return this->read_resource_events;
}

std::vector<EventHandlerWaitEvent> &EventHandler::get_write_resoucer_events()
{
	return this->write_resource_events;
}
