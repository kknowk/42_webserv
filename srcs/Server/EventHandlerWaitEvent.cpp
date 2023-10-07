#include "Server/EventHandlerWaitEvent.hpp"

EventHandlerWaitEvent::EventHandlerWaitEvent()
{
}

EventHandlerWaitEvent::~EventHandlerWaitEvent()
{
}

int EventHandlerWaitEvent::get_fd() const
{
	return this->value.fd;
}

bool EventHandlerWaitEvent::is_readable_event() const
{
	return (this->value.revents & POLLIN) != 0;
}

bool EventHandlerWaitEvent::is_writable_event() const
{
	return (this->value.revents & POLLOUT) != 0;
}

bool EventHandlerWaitEvent::is_hangup_event() const
{
	return (this->value.revents & POLLHUP) != 0;
}

bool EventHandlerWaitEvent::is_error_event() const
{
	return (this->value.revents & POLLERR) != 0;
}

bool EventHandlerWaitEvent::is_invalid_event() const
{
	return (this->value.revents & POLLNVAL) != 0;
}

bool EventHandlerWaitEvent::is_empty_event() const
{
	return this->value.revents == 0;
}

bool EventHandlerWaitEvent::operator<(const EventHandlerWaitEvent &right) const
{
	return this->get_fd() < right.get_fd();
}
