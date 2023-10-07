#include "Server/EventHandler.hpp"

bool EventHandler::process_connection_write_event(Connection &connection)
{
	if (!connection.is_writable())
		return false;
	HttpResponse &response = connection.get_response();
	HttpResponseStatus &status = response.get_status();
	std::pair<IoResult, std::size_t> pair;
	switch (status.sending_target)
	{
	case HttpResponseStatus::Header:
		if (status.current_length == 0)
			break;
		pair = connection.write(status.read_resource_buffer.data() + status.current_offset, status.current_rest_length());
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			if (response.is_chunked_response())
				status.prepare_for_first_or_last_chunk(connection.get_read_resource_fd().has_value);
			else if (response.is_single_content_response())
				status.prepare_for_single_content_sending(response.get_content());
			else
			{
				if (connection.should_connection_be_closed())
				{
					this->remove_connection(connection);
					return true;
				}
				this->prepare_for_next_request(connection);
			}
			break;
		default:
			this->remove_connection(connection);
			return true;
		}
		break;
	case HttpResponseStatus::FirstChunk:
		if (status.current_length == 0)
		{
			status.prepare_for_first_or_last_chunk(connection.get_read_resource_fd().has_value);
			break;
		}
		pair = connection.write(status.read_resource_buffer.data() + status.current_offset, status.current_rest_length());
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			status.prepare_for_middle_or_last_chunk(connection.get_read_resource_fd().has_value);
			break;
		default:
			this->remove_connection(connection);
			return true;
		}
		break;
	case HttpResponseStatus::MiddleChunk:
		if (status.current_length == 0)
		{
			status.prepare_for_middle_or_last_chunk(connection.get_read_resource_fd().has_value);
			break;
		}
		pair = connection.write(status.read_resource_buffer.data() + status.current_offset, status.current_rest_length());
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			status.prepare_for_middle_or_last_chunk(connection.get_read_resource_fd().has_value);
			break;
		default:
			this->remove_connection(connection);
			return true;
		}
		break;
	case HttpResponseStatus::LastChunk:
		pair = connection.write(status.read_resource_buffer.data() + status.current_offset, status.current_rest_length());
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			if (connection.should_connection_be_closed())
			{
				this->remove_connection(connection);
				return true;
			}
			this->prepare_for_next_request(connection);
			break;
		default:
			this->remove_connection(connection);
			return true;
		}
		break;
	case HttpResponseStatus::SingleContent:
		pair = connection.write(response.get_content().value.data() + status.current_offset, status.current_rest_length());
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			if (connection.should_connection_be_closed())
			{
				this->remove_connection(connection);
				return true;
			}
			this->prepare_for_next_request(connection);
			break;
		default:
			this->remove_connection(connection);
			return true;
		}
		break;
	default:
		std::cerr << "Error: EventHandler::process_connection_write_event: Connection Id: " << connection.get_fd() << ", HttpResponseStatus: " << status.sending_target << " " << current_time << std::endl;
		break;
	}
	return false;
}
