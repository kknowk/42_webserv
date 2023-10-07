#include "Server/Connection.hpp"
#include "Util/Logger.hpp"
#include <cstdio>
#include <ctime>

Connection::Connection(int socket_fd, int base_socket_fd, const sockaddr_storage &sockaddr, EventHandler *handler)
	: handler(handler), status(), socket(socket_fd), base_socket_fd(base_socket_fd), last_time(std::clock()), should_dispose(false), buffer(), counterpart_ip(), counterpart_port()
{
	char text[256];
	switch (sockaddr.ss_family)
	{
	case AF_INET:
	{
		const sockaddr_in &a = *reinterpret_cast<const sockaddr_in *>(&sockaddr);
		std::snprintf(text, sizeof(text), "%u", a.sin_port);
		this->counterpart_port = std::string(text);
		uint32_t address = ntohl(a.sin_addr.s_addr);
		std::snprintf(text, sizeof(text), "%u.%u.%u.%u", (address >> 24) & 0xff, (address >> 16) & 0xff, (address >> 8) & 0xff, address & 0xff);
		this->counterpart_ip = std::string(text);
	}
	break;
	case AF_INET6:
	{
		const sockaddr_in6 &a = *reinterpret_cast<const sockaddr_in6 *>(&sockaddr);
		std::snprintf(text, sizeof(text), "%u", a.sin6_port);
		this->counterpart_port = std::string(text);
		const uint8_t(&address)[16] = a.sin6_addr.s6_addr;
		std::snprintf(text, sizeof(text), "%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x:%.2x%.2x", address[0], address[1], address[2], address[3], address[4], address[5], address[6], address[7], address[8], address[9], address[10], address[11], address[12], address[13], address[14], address[15]);
		this->counterpart_ip = std::string(text);
	}
	break;
	default:
		break;
	}
#ifdef DEBUG
	std::cerr << "Debug: Connection(" << this->get_fd() << ") is now created. Counterpart: " << this->counterpart_ip << ":" << this->counterpart_port << " " << current_time << std::endl;
#endif
}

Connection::~Connection()
{
	if (this->should_dispose)
	{
		this->close();
#ifdef DEBUG
		std::cerr << "Debug: Connection(" << this->get_fd() << ") is now disposed. " << current_time << std::endl;
#endif
	}
}

HttpRequest &Connection::get_request()
{
	return this->request;
}

const HttpRequest &Connection::get_request() const
{
	return this->request;
}

HttpResponse &Connection::get_response()
{
	return this->response;
}

const HttpResponse &Connection::get_response() const
{
	return this->response;
}

const ConnectionStatus &Connection::get_status() const
{
	return this->status;
}

ConnectionStatus &Connection::get_status()
{
	return this->status;
}

std::vector<char> &Connection::get_buffer()
{
	return this->buffer;
}

const std::vector<char> &Connection::get_buffer() const
{
	return this->buffer;
}

const std::string &Connection::get_counterpart_ip() const
{
	return this->counterpart_ip;
}

const std::string &Connection::get_counterpart_port() const
{
	return this->counterpart_port;
}

const Socket &Connection::get_socket() const
{
	return this->socket;
}

int Connection::get_fd() const
{
	return this->socket.get_fd();
}

int Connection::get_base_socket_fd() const
{
	return this->base_socket_fd;
}

void Connection::force_dispose()
{
	this->should_dispose = true;
	this->socket.force_dispose();
}

bool Connection::operator==(const Connection &right) const
{
	return this->socket == right.socket;
}

bool Connection::operator!=(const Connection &right) const
{
	return this->socket != right.socket;
}

bool Connection::operator<(const Connection &right) const
{
	return this->socket < right.socket;
}

bool Connection::operator>(const Connection &right) const
{
	return this->socket > right.socket;
}

bool Connection::operator<=(const Connection &right) const
{
	return this->socket <= right.socket;
}

bool Connection::operator>=(const Connection &right) const
{
	return this->socket >= right.socket;
}

const std::clock_t &Connection::get_last_time() const
{
	return this->last_time;
}

const Nullable<int> &Connection::get_write_resource_fd() const
{
	return this->write_resource_fd;
}

void Connection::set_write_resource_fd(int fd)
{
	this->write_resource_fd = fd;
}

void Connection::reset_write_resource_fd()
{
	this->write_resource_fd.has_value = false;
}

bool Connection::is_readble() const
{
	return this->status.read_write == ConnectionStatus::Readable;
}

bool Connection::is_writable() const
{
	return this->status.read_write == ConnectionStatus::Writable;
}

bool Connection::is_noop() const
{
	return this->status.read_write == ConnectionStatus::Noop;
}

void Connection::make_readable()
{
	this->status.read_write = ConnectionStatus::Readable;
}

void Connection::make_writable()
{
	this->status.read_write = ConnectionStatus::Writable;
}

void Connection::make_noop()
{
	this->status.read_write = ConnectionStatus::Noop;
}

const Nullable<int> &Connection::get_read_resource_fd() const
{
	return this->read_resource_fd;
}

void Connection::set_read_resource_fd(int fd)
{
	this->read_resource_fd = fd;
}

void Connection::reset_read_resource_fd()
{
	this->read_resource_fd.has_value = false;
}

IoResult Connection::read()
{
	return this->socket.read(this->buffer);
}

bool Connection::is_passed(const TimeSpan &span) const
{
	return std::clock() - this->last_time >= span.to_clock();
}

std::pair<IoResult, std::size_t> Connection::write(const void *data, std::size_t size)
{
	return this->socket.write(data, size);
}

void Connection::start_response_sending()
{
	std::cerr << "Info: => (" << this->get_fd() << "-" << this->request.get_index() << "):";
	const Nullable<HttpStatusCode> &code = this->response.get_status_code();
	if (code.has_value)
	{
		std::cerr << " Code: " << code.value;
	}
	else
	{
		std::cerr << " CGI";
	}
	std::cerr << " " << current_time << std::endl;
	this->make_writable();
	this->get_status().state = ConnectionStatus::DuringResponseSending;
}

void Connection::wait_for_resource_completion()
{
	HttpResponseStatus &status = this->response.get_status();
	status.read_resource_buffer.clear();
	status.current_offset = 0;
	this->make_noop();
	this->get_status().state = ConnectionStatus::DuringWaitForResourceCompletion;
}

void Connection::prepare_for_next_request()
{
	std::size_t consumed = this->request.size();
	this->buffer.erase(this->buffer.begin(), this->buffer.begin() + consumed);
	std::size_t next_index = this->request.get_index() + 1;
	this->request.clear();
	this->request.set_index(next_index);
	this->response.clear();
	this->status.state = ConnectionStatus::DuringRequestReading;
	this->make_readable();
	this->last_time = std::clock();
}

void Connection::close()
{
	this->status.state = ConnectionStatus::Closed;
}

ConnectionStatus::ConnectionStatus()
	: state(ConnectionStatus::DuringRequestReading), read_write(ConnectionStatus::Readable)
{
}

bool Connection::should_connection_be_closed() const
{
	if (this->status.state == ConnectionStatus::Closed)
		return true;
	if (this->request.should_connection_be_closed())
		return true;
	if (this->response.should_connection_be_closed())
		return true;
	return false;
}
