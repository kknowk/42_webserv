#include "Server/EventHandler.hpp"
#include <iostream>

/// @return true: removed connection
bool EventHandler::process_connection_read_event(Connection &connection)
{
	if (!connection.is_readble())
		return false;
	ConnectionStatus &status = connection.get_status();
	if (status.state == ConnectionStatus::Closed)
	{
		this->remove_connection(connection);
		return true;
	}
	IoResult result = connection.read();
	if (result != IoResult_Continue)
	{
		this->remove_connection(connection);
		return true;
	}
	if (status.state == ConnectionStatus::DuringWaitForResourceCompletion)
	{
		connection.make_noop();
	}
	else if (status.state == ConnectionStatus::DuringResponseSending)
	{
		connection.make_writable();
	}
	return false;
}
