#ifndef EventHandlerWAITEVENT_HPP
#define EventHandlerWAITEVENT_HPP

#include <poll.h>

class EventHandlerWaitEvent
{
public:
	struct pollfd value;

	EventHandlerWaitEvent();
	~EventHandlerWaitEvent();

	int get_fd() const;

	bool is_readable_event() const;
	bool is_writable_event() const;
	/// @brief Hang up. Note that when reading from a channel such as a pipe or a stream socket, this event merely indicates that the peer closed its end of the channel. Subsequent reads from the channel will return 0 (end of file) only after all outstanding data in the channel has been consumed.
	/// @return
	bool is_hangup_event() const;
	/// @brief Error condition. This bit is also set for a file descriptor referring to the write end of a pipe when the read end has been closed.
	/// @return
	bool is_error_event() const;
	bool is_invalid_event() const;

	bool is_empty_event() const;

	bool operator<(const EventHandlerWaitEvent &right) const;
};

#endif