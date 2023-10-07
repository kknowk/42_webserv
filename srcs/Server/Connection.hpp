#ifndef CONNECTION_HPP
#define CONNECTION_HPP

#include <string>
#include <vector>
#include <ctime>
#include <set>
#include "Util/Nullable.hpp"
#include "Util/TimeSpan.hpp"
#include "Server/Socket.hpp"
#include "Server/HttpResponse.hpp"
#include "Server/HttpRequest.hpp"
#include "Server/EventHandler.hpp"

struct ConnectionStatus
{
public:
	enum StateKind
	{
		DuringRequestReading,
		DuringWaitForResourceCompletion,
		DuringResponseSending,
		Closed,
	};

	enum ReadWriteKind
	{
		Noop,
		Readable,
		Writable,
	};

	StateKind state;
	ReadWriteKind read_write;

	ConnectionStatus();
};

class EventHandler;

class Connection
{
public:
	Connection(int socket_fd, int base_socket_fd, const sockaddr_storage &sockaddr, EventHandler *handler);
	~Connection();

	HttpRequest &get_request();
	const HttpRequest &get_request() const;

	HttpResponse &get_response();
	const HttpResponse &get_response() const;

	const ConnectionStatus &get_status() const;
	ConnectionStatus &get_status();

	bool is_readble() const;
	bool is_writable() const;
	bool is_noop() const;

	bool should_connection_be_closed() const;

	void make_readable();
	void make_writable();
	void make_noop();
	
	const Socket &get_socket() const;
	int get_fd() const;

	const std::clock_t &get_last_time() const;

	int get_base_socket_fd() const;

	std::vector<char> &get_buffer();
	const std::vector<char> &get_buffer() const;

	const std::string &get_counterpart_ip() const;
	const std::string &get_counterpart_port() const;

	const Nullable<int> &get_write_resource_fd() const;

	const Nullable<int> &get_read_resource_fd() const;

	void force_dispose();

	IoResult read();

	bool is_passed(const TimeSpan &span) const;

	void start_response_sending();
	void wait_for_resource_completion();
	void prepare_for_next_request();

	std::pair<IoResult, std::size_t> write(const void *data, std::size_t size);

	bool operator==(const Connection &right) const;
	bool operator!=(const Connection &right) const;
	bool operator<(const Connection &right) const;
	bool operator>(const Connection &right) const;
	bool operator<=(const Connection &right) const;
	bool operator>=(const Connection &right) const;

private:
	EventHandler *handler;
	ConnectionStatus status;
	HttpRequest request;
	HttpResponse response;
	Socket socket;
	int base_socket_fd;
	Nullable<int> write_resource_fd;
	Nullable<int> read_resource_fd;
	std::clock_t last_time;
	bool should_dispose;
	std::vector<char> buffer;
	std::string counterpart_ip;
	std::string counterpart_port;

	void set_write_resource_fd(int fd);
	void set_read_resource_fd(int fd);
	void reset_write_resource_fd();
	void reset_read_resource_fd();

	void close();
	friend class EventHandler;
};

#endif