#include "Util/HttpRedirection.hpp"

HttpRedirection::HttpRedirection()
{
}

HttpRedirection::HttpRedirection(const std::vector<Either<std::string, HttpRedirection::Variable> >& converter, const HttpStatusCode &code)
	: converter(converter), code(code)
{
}

HttpRedirection::HttpRedirection(const HttpRedirection &right)
	: converter(right.converter), code(right.code)
{
}

HttpRedirection::~HttpRedirection()
{
}

HttpRedirection &HttpRedirection::operator=(const HttpRedirection &right)
{
	if (this != &right)
	{
		this->converter = right.converter;
		this->code = right.code;
	}
	return *this;
}

const std::vector<Either<std::string, HttpRedirection::Variable> > &HttpRedirection::get_converter() const
{
	return this->converter;
}

const HttpStatusCode &HttpRedirection::get_code() const
{
	return this->code;
}

void HttpRedirection::converter_add(const std::string &value)
{
	this->converter.push_back(Either<std::string, HttpRedirection::Variable>(value));
}

void HttpRedirection::converter_add(Variable value)
{
	this->converter.push_back(Either<std::string, HttpRedirection::Variable>(value));
}
