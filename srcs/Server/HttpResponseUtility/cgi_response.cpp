#include "Server/HttpResponseUtility.hpp"
#include "Server/CgiResponse.hpp"

void HttpResponseUtility::from_cgi_response(EventHandler &handler, Connection &connection)
{
	HttpResponseUtility::ErrorReferences err_ref(handler, connection);
	HttpResponse &response = connection.get_response();
	HttpResponseStatus &status = response.get_status();
	if (status.read_resource_buffer.empty())
	{
		HttpResponseUtility::from_error_status_code(err_ref, 500);
		return;
	}
	CgiResponse cgi;
	if (!cgi.parse(status.read_resource_buffer))
	{
		HttpResponseUtility::from_error_status_code(err_ref, 500);
		return;
	}
	if (cgi.is_client_redirect())
	{
		cgi.write_client_redirect(response);
		connection.start_response_sending();
	}
	else if (cgi.is_document())
	{
		cgi.write_document(response);
		connection.start_response_sending();
	}
	else if (cgi.is_local_redirect())
	{
		cgi.write_local_redirect(connection.get_buffer(), connection.get_request());
		handler.prepare_for_next_request(connection);
	}
	else
	{
		HttpResponseUtility::from_error_status_code(err_ref, 500);
	}
}
