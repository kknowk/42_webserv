#ifndef EVENTHANDLER_HPP
#define EVENTHANDLER_HPP

#include "Server/Socket.hpp"
#include "Server/ListenerSocket.hpp"
#include "Server/EventHandlerWaitEvent.hpp"
#include "Server/Connection.hpp"
#include "Server/ReadableResource.hpp"
#include "Server/WritableResource.hpp"
#include "Config/Config.hpp"
#include "Util/TimeSpan.hpp"
#include <vector>
#include <set>
#include <map>

class Connection;

class EventHandler
{
private:
	int fd;

	std::map<int, ListenerSocket> listen_sockets;
	std::map<int, Connection> connections;
	std::map<int, ReadableResource> read_resources;
	std::map<int, WritableResource> write_resources;

	std::vector<EventHandlerWaitEvent> listen_socket_events;
	std::vector<EventHandlerWaitEvent> connection_events;
	std::vector<EventHandlerWaitEvent> read_resource_events;
	std::vector<EventHandlerWaitEvent> write_resource_events;

	const Config &config;

	void clear_events();
	void classify_events(const std::vector<EventHandlerWaitEvent> &events);

	/// @return true: removed connection
	bool process_connection_read_event(Connection &connection);
	/// @return true: removed connection
	bool process_connection_write_event(Connection &connection);

	/// @return true: removed read resource
	bool process_read_resource(Connection &connection, const ReadableResource &resource);
	
	/// @return true: removed write resource
	bool process_write_resource(Connection &connection, const WritableResource &resource);

	std::map<int, Connection>::iterator process_logic(std::map<int, Connection>::iterator item);

	std::map<int, Connection>::iterator cull_connection(std::map<int, Connection>::iterator item);

public:
	EventHandler(const Config &config);
	~EventHandler();

	/// @return false: FAIL
	bool prepare_listeners();

	void add_listener(int socket_fd, const std::string &host, const std::string &port);
	void add_connection(int connection_fd, int base_socket_fd, const sockaddr_storage &address);

	void add_write_resource_pipe(int resource_fd, Connection &connection);
	void add_read_resource_pipe(int resource_fd, Connection &connection);
	void add_write_resource_file(int resource_fd, Connection &connection);
	void add_read_resource_file(int resource_fd, Connection &connection);

	const Config &get_config() const;
	const EnvironmentVariableDictionary &get_dictionary() const;

	void remove_write_resource(Connection &connection);
	void remove_read_resource(Connection &connection);
	void remove_connection(Connection &connection);

	std::vector<EventHandlerWaitEvent> &get_listen_socket_events();
	std::vector<EventHandlerWaitEvent> &get_connection_events();
	std::vector<EventHandlerWaitEvent> &get_read_resource_events();
	std::vector<EventHandlerWaitEvent> &get_write_resoucer_events();

	void wait(const TimeSpan &timeout);

	/// @brief accept new connection
	void process_listen_sockets();
	/// @brief read from and write to the connection
	void process_connections();
	/// @brief read from cgi-pipe or regular-local-file
	void process_read_resources();
	/// @brief write to cgi-pipe or edit regular-local-file
	void process_write_resources();

	/// @brief process accumulated requests.
	void process_requests();

	void cull_connections();

	void prepare_for_next_request(Connection &connection);

	bool is_listener_id(int fd) const;
	bool is_connection_id(int fd) const;
	bool is_write_resource_id(int fd) const;
	bool is_read_resource_id(int fd) const;

	std::map<int, ListenerSocket>::const_iterator get_corresponding_listener(const std::string &ip, const std::string &port) const;
	const std::string &get_connection_ip(const Connection &connection) const;
	const std::string &get_connection_port(const Connection &connection) const;

	const ListenerSocket &get_listener_by_socket_fd(int fd) const;
	ListenerSocket &get_listener_by_socket_fd(int fd);
	Connection &get_connection_by_connection_fd(int fd);
	Connection &get_connection_by_write_resource_fd(int fd);
	Connection &get_connection_by_read_resource_fd(int fd);

	bool not_empty() const;

	/// @brief close all connections and read/write resources when memory allocation fails. listening port socket is still available.
	void emergency_free(const std::bad_alloc &e);
};

#endif