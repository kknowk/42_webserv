#ifndef HTTP_REQUEST_PARSINGSTATE
#define HTTP_REQUEST_PARSINGSTATE

#include <cstddef>

class HttpRequestParsingState
{
public:
	enum State
	{
		During_Head,
		During_Normal_Content,
		During_Chunked_Content_Before_Length,
		During_Chunked_Content_During_Content,
		During_Chunked_Content_After_Content,
		Done,
		Critical_Error,
	};

	HttpRequestParsingState();
	int status_code;
	bool is_error() const;
	bool is_critical_error() const;
	State state;

	std::size_t current_length;
	std::size_t current_position;
	std::size_t current_required_read_length() const;
};

#endif