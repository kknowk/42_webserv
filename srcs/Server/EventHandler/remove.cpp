#include "Server/EventHandler.hpp"

void EventHandler::remove_write_resource(Connection &connection)
{
	const Nullable<int> &resource_fd = connection.get_write_resource_fd();
	if (!resource_fd.has_value || resource_fd.value < 0)
		return;
	int fd = resource_fd.value;
	std::map<int, WritableResource>::iterator found = this->write_resources.find(fd);
	if (found == this->write_resources.end())
		return;
	this->write_resources.erase(found);
	for (std::vector<EventHandlerWaitEvent>::iterator itr = this->write_resource_events.begin(); itr != this->write_resource_events.end(); itr++)
	{
		if (itr->get_fd() == fd)
		{
			this->write_resource_events.erase(itr);
			break;
		}
	}
	connection.reset_write_resource_fd();
}

void EventHandler::remove_read_resource(Connection &connection)
{
	const Nullable<int> &resource_fd = connection.get_read_resource_fd();
	if (!resource_fd.has_value || resource_fd.value < 0)
		return;
	int fd = resource_fd.value;
	std::map<int, ReadableResource>::iterator found = this->read_resources.find(fd);
	if (found == this->read_resources.end())
		return;
	this->read_resources.erase(found);
	for (std::vector<EventHandlerWaitEvent>::iterator itr = this->read_resource_events.begin(); itr != this->read_resource_events.end(); itr++)
	{
		if (itr->get_fd() == fd)
		{
			this->read_resource_events.erase(itr);
			break;
		}
	}
	connection.reset_read_resource_fd();
}

void EventHandler::remove_connection(Connection &connection)
{
	int connection_fd = connection.get_fd();
	std::map<int, Connection>::iterator found = this->connections.find(connection_fd);
	if (found == this->connections.end())
		return;
	this->remove_write_resource(connection);
	this->remove_read_resource(connection);
	this->connections.erase(found);
	for (std::vector<EventHandlerWaitEvent>::iterator itr = this->connection_events.begin(); itr != this->connection_events.end(); itr++)
	{
		if (itr->get_fd() == connection_fd)
		{
			this->connection_events.erase(itr);
			break;
		}
	}
}
