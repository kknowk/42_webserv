#include "Server/EventHandler.hpp"
#include "Util/Logger.hpp"
#include <poll.h>

EventHandler::EventHandler(const Config &config)
	: fd(-1), listen_sockets(), connections(), read_resources(), write_resources(), listen_socket_events(), connection_events(), read_resource_events(), write_resource_events(), config(config)
{
}

EventHandler::~EventHandler()
{
}

static void prepare_pollfds(std::vector<EventHandlerWaitEvent> &events, const std::map<int, ListenerSocket> &listen_sockets)
{
	EventHandlerWaitEvent event;
	event.value.events = POLLIN;
	for (std::map<int, ListenerSocket>::const_iterator start = listen_sockets.begin(), end = listen_sockets.end(); start != end; start++)
	{
		event.value.fd = start->first;
		events.push_back(event);
	}
}

static void prepare_pollfds(std::vector<EventHandlerWaitEvent> &events, const std::map<int, Connection> &connections)
{
	EventHandlerWaitEvent event;
	for (std::map<int, Connection>::const_iterator start = connections.begin(), end = connections.end(); start != end; start++)
	{
		event.value.fd = start->first;
		switch (start->second.get_status().read_write)
		{
		case ConnectionStatus::Noop:
			continue;
		case ConnectionStatus::Readable:
		case ConnectionStatus::Writable:
			event.value.events = POLLIN | POLLOUT;
			break;
		default:
			throw std::out_of_range("Connection Status ReadWriteKind is out of range.");
		}
		events.push_back(event);
	}
}

static void prepare_pollfds(std::vector<EventHandlerWaitEvent> &events, const std::map<int, ReadableResource> &resources)
{
	EventHandlerWaitEvent event;
	event.value.events = POLLIN;
	for (std::map<int, ReadableResource>::const_iterator start = resources.begin(), end = resources.end(); start != end; start++)
	{
		event.value.fd = start->first;
		events.push_back(event);
	}
}

static void prepare_pollfds(std::vector<EventHandlerWaitEvent> &events, const std::map<int, WritableResource> &resources)
{
	EventHandlerWaitEvent event;
	event.value.events = POLLOUT;
	for (std::map<int, WritableResource>::const_iterator start = resources.begin(), end = resources.end(); start != end; start++)
	{
		event.value.fd = start->first;
		events.push_back(event);
	}
}

void EventHandler::wait(const TimeSpan &timeout)
{
	std::vector<EventHandlerWaitEvent> events;
	prepare_pollfds(events, this->listen_sockets);
	prepare_pollfds(events, this->connections);
	prepare_pollfds(events, this->read_resources);
	prepare_pollfds(events, this->write_resources);
	if (!events.empty())
	{
		int timeout_int = timeout.is_infinity() ? -1 : static_cast<int>(timeout.to_milliseconds());
		int event_count = ::poll(reinterpret_cast<struct pollfd *>(events.data()), events.size(), timeout_int);
		if (event_count < 0)
		{
			std::cerr << "Error: EventHandler::wait. Wait for " << events.size() << ", but no event occurs. " << current_time << std::endl;
			this->clear_events();
			return;
		}
		if (event_count == 0)
			events.clear();
	}
	this->classify_events(events);
}
