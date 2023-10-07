#include "Server/EventHandler.hpp"
#include "Server/HttpResponseUtility.hpp"
#include "Util/FileUtility.hpp"

bool EventHandler::process_read_resource(Connection &connection, const ReadableResource &resource)
{
	HttpResponse &response = connection.get_response();
	HttpResponseStatus &status = response.get_status();
	switch (FileUtility::read(resource.get_fd(), status.read_resource_buffer))
	{
	case IoResult_Continue:
		return false;
	case IoResult_Done:
		this->remove_read_resource(connection);
		if (resource.is_pipe())
		{
			this->remove_write_resource(connection);
			HttpResponseUtility::from_cgi_response(*this, connection);
		}
		return true;
	default:
		this->remove_connection(connection);
		return true;
	}
}