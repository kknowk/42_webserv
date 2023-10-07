#include "Server/EventHandler.hpp"

void EventHandler::prepare_for_next_request(Connection &connection)
{
	this->remove_read_resource(connection);
	this->remove_write_resource(connection);
	connection.prepare_for_next_request();
}
