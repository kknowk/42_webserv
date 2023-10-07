#ifndef WRITABLERESOURCE_HPP
#define WRITABLERESOURCE_HPP

class WritableResource
{
private:
	int fd;
	int connection_fd;
	bool _is_pipe;
	bool should_dispose;
public:
	WritableResource(int fd, int connection_fd, int _is_pipe);
	~WritableResource();

	int get_fd() const;
	int get_connection_fd() const;
	int is_pipe() const;
	void force_dispose();

	bool operator<(const WritableResource &right) const;
};

#endif