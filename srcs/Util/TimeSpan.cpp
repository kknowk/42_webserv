#include "Util/TimeSpan.hpp"

TimeSpan::TimeSpan(std::clock_t clock)
	: time(clock)
{
}

TimeSpan::TimeSpan()
{
}

TimeSpan::~TimeSpan()
{
}

TimeSpan TimeSpan::from_milliseconds(uint64_t value)
{
	return TimeSpan(value * CLOCKS_PER_SEC / 1000);
}

TimeSpan TimeSpan::from_seconds(uint64_t value)
{
	return TimeSpan(value * CLOCKS_PER_SEC);
}

int64_t TimeSpan::to_milliseconds() const
{
	if (this->time < 0)
		return -1;
	return this->time * 1000 / CLOCKS_PER_SEC;
}

int64_t TimeSpan::to_seconds() const
{
	if (this->time < 0)
		return -1;
	return this->time / CLOCKS_PER_SEC;
}

TimeSpan TimeSpan::infinity()
{
	return TimeSpan((std::clock_t)-1);
}

bool TimeSpan::is_infinity() const
{
	return this->time == (std::clock_t)-1;
}

bool TimeSpan::operator==(const TimeSpan &right) const
{
	return this->time == right.time;
}

bool TimeSpan::operator<(const TimeSpan &right) const
{
	if (this->is_infinity())
		return false;
	if (right.is_infinity())
		return false;
	return this->time < right.time;
}

bool TimeSpan::operator<=(const TimeSpan &right) const
{
	if (this->is_infinity())
	{
		return right.is_infinity();
	}
	if (right.is_infinity())
		return true;
	return this->time <= right.time;
}

bool TimeSpan::operator>(const TimeSpan &right) const
{
	if (this->is_infinity())
		return !right.is_infinity();
	if (right.is_infinity())
		return false;
	return this->time > right.time;
}

bool TimeSpan::operator>=(const TimeSpan &right) const
{
	if (this->is_infinity())
		return true;
	if (right.is_infinity())
		return false;
	return this->time >= right.time;
}

bool TimeSpan::operator!=(const TimeSpan &right) const
{
	return this->time != right.time;
}

std::clock_t TimeSpan::to_clock() const
{
	return this->time;
}

std::ostream &operator<<(std::ostream &stream, const TimeSpan &value)
{
	if (value.is_infinity())
	{
		stream << "(infinity)";
		return stream;
	}
	int64_t sec = 0, minute = 0, hour = 0, day = 0;
	int64_t millisec = value.to_milliseconds();
	if (millisec >= 1000)
	{
		sec = millisec / 1000;
		millisec -= sec * 1000;
	}
	if (sec >= 60)
	{
		minute = sec / 60;
		sec -= minute * 60;
	}
	if (minute >= 60)
	{
		hour = minute / 60;
		minute -= hour * 60;
	}
	if (hour >= 24)
	{
		day = hour / 24;
		hour -= day * 24;
	}
	bool should_space = false;
	if (day)
	{
		stream << day << " day";
		should_space = true;
	}
	if (hour)
	{
		if (should_space)
			stream << ' ';
		stream << hour << " hour";
		should_space = true;
	}
	if (minute)
	{
		if (should_space)
			stream << ' ';
		stream << minute << " min";
		should_space = true;
	}
	if (sec)
	{
		if (should_space)
			stream << ' ';
		stream << sec << " sec";
		should_space = true;
	}
	if (millisec)
	{
		if (should_space)
			stream << ' ';
		stream << millisec << " ms";
		should_space = true;
	}
	else if (!should_space)
		stream << "0 ms";
	return stream;
}

std::ostream &current_time(std::ostream &stream)
{
	std::time_t now;
	std::time(&now);
	char tmp[256];
	std::strftime(tmp, sizeof(tmp), "%a, %d %b %Y %H:%M:%S GMT", std::gmtime(&now));
	stream << tmp;
	return stream;
}
