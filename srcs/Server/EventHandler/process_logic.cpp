#include "Server/EventHandler.hpp"
#include "Server/HttpResponseUtility.hpp"

void EventHandler::process_requests()
{
	for (std::map<int, Connection>::iterator itr = this->connections.begin(); itr != this->connections.end();)
	{
		itr = this->process_logic(itr);
	}
}

std::map<int, Connection>::iterator EventHandler::process_logic(std::map<int, Connection>::iterator item)
{
	std::map<int, Connection>::iterator next_item = item;
	next_item++;
	Connection &connection = item->second;
	HttpRequest &request = connection.get_request();
	switch (connection.get_status().state)
	{
	case ConnectionStatus::DuringRequestReading:
		request.parse(connection.get_buffer(), this->get_config().get_request_body_size_limit());
		if (request.is_critical_error())
		{
			int status_code = request.get_state().status_code;
			if (status_code)
				HttpResponseUtility::from_error_status_code(this, connection, status_code);
			else
				this->remove_connection(connection);
		}
		else if (request.is_error())
		{
			int status_code = request.get_state().status_code;
			HttpResponseUtility::from_error_status_code(this, connection, status_code);
		}
		else if (request.is_parse_success())
		{
			const HttpRequestHeader &header = request.get_header();
			std::cerr << "Info: <= (" << connection.get_fd() << "-" << request.get_index() << "): " << header.get_method() << " " << header.get_normalized_resource_location() << " " << current_time << std::endl;
			const ListenerSocket &socket = this->get_listener_by_socket_fd(connection.get_base_socket_fd());
			HttpResponseUtility::from_config(*this, connection, socket);
			next_item = item;
		}
		break;
	case ConnectionStatus::DuringWaitForResourceCompletion:
		connection.make_noop();
		break;
	case ConnectionStatus::DuringResponseSending:
		connection.make_writable();
		break;
	case ConnectionStatus::Closed:
		this->remove_connection(connection);
		break;
	default:
		throw std::runtime_error("Error: EventHandler::process_logic. ConnectionStatus is out of range.");
	}
	return next_item;
}
