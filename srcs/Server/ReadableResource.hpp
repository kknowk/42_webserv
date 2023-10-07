#ifndef READABLERESOURCE_HPP
#define READABLERESOURCE_HPP

class ReadableResource
{
private:
	int fd;
	int connection_fd;
	bool _is_pipe;
	bool should_dispose;
public:
	ReadableResource(int fd, int connection_fd, bool is_pipe);
	~ReadableResource();

	int get_fd() const;
	int get_connection_fd() const;
	bool is_pipe() const;
	void force_dispose();

	bool operator<(const ReadableResource &right) const;
};

#endif