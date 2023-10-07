#include "Server/HttpRequestParsingState.hpp"

bool HttpRequestParsingState::is_error() const
{
	return this->status_code != 0 || this->state == HttpRequestParsingState::Critical_Error;
}

std::size_t HttpRequestParsingState::current_required_read_length() const
{
	return this->current_length - this->current_position;
}

bool HttpRequestParsingState::is_critical_error() const
{
	return this->state == HttpRequestParsingState::Critical_Error;
}

HttpRequestParsingState::HttpRequestParsingState()
	: status_code(0), state(HttpRequestParsingState::During_Head), current_length(0), current_position(0)
{
}