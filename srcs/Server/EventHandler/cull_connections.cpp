#include "Server/EventHandler.hpp"
#include "Server/HttpResponseUtility.hpp"

std::map<int, Connection>::iterator EventHandler::cull_connection(std::map<int, Connection>::iterator item)
{
	std::map<int, Connection>::iterator next = item;
	next++;
	Connection &connection = item->second;
	const TimeSpan &keep_alive = this->config.get_keep_alive();
	if (!connection.is_passed(keep_alive))
		return next;
	ConnectionStatus &status = connection.get_status();
	switch (status.state)
	{
	case ConnectionStatus::DuringResponseSending:
		break;
	case ConnectionStatus::Closed:
		this->remove_connection(connection);
		break;
	default:
		this->remove_read_resource(connection);
		this->remove_write_resource(connection);
		HttpResponseUtility::from_error_status_code(this, connection, 408);
		break;
	}
	return next;
}

void EventHandler::cull_connections()
{
	std::map<int, Connection>::iterator itr = this->connections.begin();
	while (itr != this->connections.end())
	{
		itr = this->cull_connection(itr);
	}
}
