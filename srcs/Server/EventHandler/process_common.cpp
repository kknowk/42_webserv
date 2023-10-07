#include "Server/EventHandler.hpp"
#include "Util/FileUtility.hpp"
#include <algorithm>
#include <iostream>
#include <cerrno>

void EventHandler::process_listen_sockets()
{
	for (std::vector<EventHandlerWaitEvent>::iterator itr = this->listen_socket_events.begin(); itr != this->listen_socket_events.end(); itr++)
	{
		int fd = itr->get_fd();
		sockaddr_storage address;
		socklen_t address_length = sizeof(address);
		int connection_fd = ::accept(fd, reinterpret_cast<sockaddr *>(&address), &address_length);
		if (connection_fd < 0)
		{
			const char *error_text = ::strerror(errno);
			std::cerr << "Error: EventHandler::accept. Listen Socket: " << fd << " has failed to accept new connection. Reason: " << error_text << " " << current_time << std::endl;
			continue;
		}
		if (!FileUtility::add_close_on_exec_and_nonblock_flag(connection_fd))
		{
			::close(connection_fd);
			continue;
		}
		this->add_connection(connection_fd, fd, address);
	}
}

void EventHandler::process_connections()
{
	for (std::size_t index = 0; index < this->connection_events.size();)
	{
		EventHandlerWaitEvent &event = this->connection_events[index];
		int fd = event.get_fd();
		Connection &connection = this->get_connection_by_connection_fd(fd);
		if (event.is_invalid_event() || event.is_hangup_event() || event.is_error_event())
		{
			this->remove_connection(connection);
			continue;
		}
		if (event.is_readable_event())
		{
			if (this->process_connection_read_event(connection))
				continue;
		}
		if (event.is_writable_event())
		{
			if (this->process_connection_write_event(connection))
				continue;
		}
		index++;
	}
}

void EventHandler::process_read_resources()
{
	for (std::size_t index = 0; index < this->read_resource_events.size();)
	{
		EventHandlerWaitEvent &event = this->read_resource_events[index];
		int fd = event.get_fd();
		std::map<int, ReadableResource>::iterator found = this->read_resources.find(fd);
		if (found == this->read_resources.end())
			continue;
		Connection &connection = this->get_connection_by_connection_fd(found->second.get_connection_fd());
		if (event.is_invalid_event())
		{
			this->remove_connection(connection);
			continue;
		}
		else if (event.is_readable_event() || event.is_hangup_event() || event.is_error_event())
		{
			if (this->process_read_resource(connection, found->second))
				continue;
		}
		index++;
	}
}

void EventHandler::process_write_resources()
{
	for (std::size_t index = 0; index < this->write_resource_events.size();)
	{
		EventHandlerWaitEvent &event = this->write_resource_events[index];
		int fd = event.get_fd();
		std::map<int, WritableResource>::iterator found = this->write_resources.find(fd);
		if (found == this->write_resources.end())
			continue;
		Connection &connection = this->get_connection_by_connection_fd(found->second.get_connection_fd());
		if (event.is_invalid_event())
		{
			this->remove_connection(connection);
			continue;
		}
		else if (event.is_error_event())
		{
			this->remove_write_resource(connection);
			continue;
		}
		else if (event.is_writable_event())
		{
			if (this->process_write_resource(connection, found->second))
				continue;
		}
		index++;
	}
}
