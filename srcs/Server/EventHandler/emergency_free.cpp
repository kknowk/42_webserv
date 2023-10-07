#include "Server/EventHandler.hpp"

void EventHandler::emergency_free(const std::bad_alloc &e)
{
	this->connections.clear();
	this->read_resources.clear();
	this->write_resources.clear();
	std::cerr << "Error: " << e.what() << " " << current_time << std::endl;
}
