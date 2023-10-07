#ifndef TIMESPAN_HPP
#define TIMESPAN_HPP

#include <ctime>
#include <inttypes.h>
#include <iostream>

class TimeSpan
{
private:
	std::clock_t time;

public:
	TimeSpan(std::clock_t clock);
	TimeSpan();
	~TimeSpan();

	static TimeSpan from_milliseconds(uint64_t value);
	static TimeSpan from_seconds(uint64_t value);

	static TimeSpan infinity();

	int64_t to_milliseconds() const;
	int64_t to_seconds() const;

	std::clock_t to_clock() const;

	bool is_infinity() const;

	bool operator==(const TimeSpan &right) const;
	bool operator!=(const TimeSpan &right) const;
	bool operator<(const TimeSpan &right) const;
	bool operator<=(const TimeSpan &right) const;
	bool operator>(const TimeSpan &right) const;
	bool operator>=(const TimeSpan &right) const;
};

std::ostream &operator<<(std::ostream &stream, const TimeSpan &value);

std::ostream &current_time(std::ostream &stream);

#endif