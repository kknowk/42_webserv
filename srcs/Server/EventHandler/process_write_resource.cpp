#include "Server/EventHandler.hpp"
#include "Server/HttpResponseUtility.hpp"
#include "Util/FileUtility.hpp"

bool EventHandler::process_write_resource(Connection &connection, const WritableResource &resource)
{
	HttpResponse &response = connection.get_response();
	HttpResponseStatus &status = response.get_status();
	const HttpRequest &request = connection.get_request();
	const std::vector<char> &buffer = request.get_data();
	if (resource.is_pipe())
	{
		std::pair<IoResult, std::size_t> pair = FileUtility::write(resource.get_fd(), buffer.data() + status.current_offset, buffer.size() - status.current_offset);
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			this->remove_write_resource(connection);
			return true;
		case IoResult_Error:
			this->remove_connection(connection);
			return true;
		}
	}
	else
	{
		std::pair<IoResult, std::size_t> pair = FileUtility::write(resource.get_fd(), buffer.data() + status.current_offset, buffer.size() - status.current_offset);
		switch (pair.first)
		{
		case IoResult_Continue:
			status.current_offset += pair.second;
			break;
		case IoResult_Done:
			this->remove_write_resource(connection);
			response.construction_complete();
			connection.start_response_sending();
			return true;
		case IoResult_Error:
			this->remove_write_resource(connection);
			HttpResponseUtility::from_error_status_code(this, connection, 400);
			break;
		}
	}
	return false;
}
