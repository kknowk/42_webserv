#include "Server/WritableResource.hpp"
#include <unistd.h>

WritableResource::WritableResource(int fd, int connection_fd, int is_pipe)
	: fd(fd), connection_fd(connection_fd), _is_pipe(is_pipe), should_dispose(false)
{
}

WritableResource::~WritableResource()
{
	if (should_dispose)
	{
		::close(fd);
		this->fd = -1;
	}
	this->should_dispose = false;
}

int WritableResource::get_fd() const
{
	return this->fd;
}

int WritableResource::get_connection_fd() const
{
	return this->connection_fd;
}

int WritableResource::is_pipe() const
{
	return this->_is_pipe;
}

void WritableResource::force_dispose()
{
	this->should_dispose = true;
}

bool WritableResource::operator<(const WritableResource &right) const
{
	return this->fd < right.fd;
}
