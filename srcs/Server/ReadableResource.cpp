#include "Server/ReadableResource.hpp"
#include <unistd.h>

ReadableResource::ReadableResource(int fd, int connection_fd, bool is_pipe)
	: fd(fd), connection_fd(connection_fd), _is_pipe(is_pipe), should_dispose(false)
{
}

ReadableResource::~ReadableResource()
{
	if (this->should_dispose)
	{
		::close(this->fd);
		this->fd = -1;
	}
	this->should_dispose = false;
}

int ReadableResource::get_fd() const
{
	return this->fd;
}

int ReadableResource::get_connection_fd() const
{
	return this->connection_fd;
}

bool ReadableResource::is_pipe() const
{
	return this->_is_pipe;
}

void ReadableResource::force_dispose()
{
	this->should_dispose = true;
}

bool ReadableResource::operator<(const ReadableResource &right) const
{
	return this->fd < right.fd;
}
