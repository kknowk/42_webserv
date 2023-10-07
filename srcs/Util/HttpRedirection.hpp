#ifndef HTTPREDIRECTION_HPP
#define HTTPREDIRECTION_HPP

#include <string>
#include <vector>
#include "Util/HttpStatusCode.hpp"
#include "Util/Either.hpp"

class HttpRedirection
{
public:
	enum Variable
	{
		HttpRedirection_Variable_Ip,
		HttpRedirection_Variable_Host,
		HttpRedirection_Variable_PortWithoutColon,
		HttpRedirection_Variable_PortWithColon,
		HttpRedirection_Variable_PathWithoutLocation,
		HttpRedirection_Variable_HasParameter,
		HttpRedirection_Variable_Parameter,
		HttpRedirection_Variable_HasFragment,
		HttpRedirection_Variable_Fragment,
	};
#ifdef TOOL
public:
#else
private:
#endif
	std::vector<Either<std::string, HttpRedirection::Variable> > converter;
	class HttpStatusCode code;

public:
	HttpRedirection();
	HttpRedirection(const std::vector<Either<std::string, HttpRedirection::Variable> >& converter, const class HttpStatusCode &code);
	HttpRedirection(const HttpRedirection &right);
	~HttpRedirection();
	HttpRedirection &operator=(const HttpRedirection &right);

	const std::vector<Either<std::string, HttpRedirection::Variable> > &get_converter() const;
	const class HttpStatusCode &get_code() const;

	void converter_add(const std::string &value);
	void converter_add(Variable value);

#ifndef TOOL
	friend class StreamReader;
	friend class StreamWriter;
#endif
};

#endif