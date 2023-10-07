#include "Server/HttpResponseUtility.hpp"

static void append_redirection(std::vector<char> &buffer, const HttpRequest &request, const ServerConfig &server_config, const std::string &location, const Either<std::string, HttpRedirection::Variable> &either)
{
	if (either.is_item0)
	{
		buffer.insert(buffer.end(), either.item0.begin(), either.item0.end());
		return;
	}
	switch (either.item1)
	{
	case HttpRedirection::HttpRedirection_Variable_Ip:
	{
		const std::string &value = server_config.get_ip();
		buffer.insert(buffer.end(), value.begin(), value.end());
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_Host:
	{
		const std::string &value = request.get_header().get_host().get_host();
		buffer.insert(buffer.end(), value.begin(), value.end());
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_PortWithoutColon:
	{
		const std::string &value = server_config.get_port();
		std::string::const_iterator start = value.begin();
		if (!value.empty() && *start == ':')
			++start;
		buffer.insert(buffer.end(), start, value.end());
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_PortWithColon:
	{
		const std::string &value = server_config.get_port();
		std::string::const_iterator start = value.begin();
		if (!value.empty() && *start != ':')
			buffer.push_back(':');
		buffer.insert(buffer.end(), start, value.end());
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_PathWithoutLocation:
	{
		Span value(request.get_header().get_normalized_resource_location());
		value = value.slice(location.size());
		buffer.insert(buffer.end(), value.begin(), value.end());
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_HasParameter:
	{
		const Span &value = request.get_header().get_resource_location().get_parameter();
		if (value.size() != 0)
			buffer.push_back('?');
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_Parameter:
	{
		const Span &value = request.get_header().get_resource_location().get_parameter();
		if (value.size() != 0)
			buffer.insert(buffer.end(), value.begin() + 1, value.end());
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_HasFragment:
	{
		const Span &value = request.get_header().get_resource_location().get_anchor();
		if (value.size() != 0)
			buffer.push_back('#');
	}
	break;
	case HttpRedirection::HttpRedirection_Variable_Fragment:
	{
		const Span &value = request.get_header().get_resource_location().get_anchor();
		if (value.size() != 0)
			buffer.insert(buffer.end(), value.begin() + 1, value.end());
	}
	break;
	}
}

void HttpResponseUtility::from_redirection_config(const RedirectionRoutingConfig &routing, const HttpResponseUtility::References &references)
{
	std::vector<char> redirection_location;
	const std::vector<Either<std::string, HttpRedirection::Variable> > &converter = routing.get_redirection().get_converter();
	for (std::vector<Either<std::string, HttpRedirection::Variable> >::const_iterator itr = converter.begin(); itr != converter.end(); itr++)
	{
		append_redirection(redirection_location, references.request, references.server, routing.get_location(), *itr);
	}
	HttpResponse &response = references.response;
	response.set_status_code(routing.get_redirection().get_code());
	response.register_header("location", std::string(redirection_location.begin(), redirection_location.end()));
	response.get_content() = std::vector<char>();
	response.construction_complete();
	references.connection.start_response_sending();
}
