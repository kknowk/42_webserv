#include "Util/Span.hpp"
#include "Util/StringHelper.hpp"
#include <new>
#include <stdexcept>
#include <algorithm>
#include <cstring>

const std::string Span::empty("");

Span::Span()
	: value(Span::empty), offset(), length()
{
}

Span::Span(const std::string &value)
	: value(value), offset(), length(value.size())
{
}

Span::Span(const std::string &value, std::string::size_type offset)
	: value(value), offset(offset), length(value.size())
{
	if (this->offset > this->length)
	{
		this->length = 0;
		return;
	}
	this->length -= this->offset;
}

Span::Span(const std::string &value, std::string::size_type offset, std::string::size_type length)
	: value(value), offset(offset), length()
{
	if (length == 0)
		return;
	this->length = value.size();
	if (this->offset > this->length)
	{
		this->length = 0;
		return;
	}
	this->length -= this->offset;
	if (this->length > length)
		this->length = length;
}

Span::Span(const Span &right)
	: value(right.value), offset(right.offset), length(right.length)
{
}

Span::~Span()
{
}

Span::Span(const Span &right, std::string::size_type offset)
	: value(right.value), offset(right.offset + offset), length(right.length)
{
	if (offset > this->length)
		this->length = 0;
	else
		this->length -= offset;
}

Span::Span(const Span &right, std::string::size_type offset, std::string::size_type length)
	: value(right.value)
{
	if (offset > right.length)
		offset = right.length;
	this->offset = right.offset + offset;
	this->length = right.length - offset;
	if (length < this->length)
		this->length = length;
}

Span &Span::operator=(const Span &right)
{
	if (this != &right)
	{
		new (this) Span(right);
	}
	return *this;
}

char Span::operator[](std::string::size_type index) const
{
	if (index >= this->length)
		throw std::out_of_range("index is out of range.");
	return this->value[index + this->offset];
}

Span::iterator Span::begin() const
{
	return this->value.begin() + this->offset;
}

Span::iterator Span::end() const
{
	return this->begin() + this->length;
}

std::string::size_type Span::size() const
{
	return this->length;
}

Span Span::rebase(const std::string &value) const
{
	return Span(value, this->offset, this->length);
}

bool Span::operator==(const std::string &right) const
{
	if (this->length != right.size())
		return false;
	if (&this->value == &right && this->offset == 0)
		return true;
	Span::iterator itr = this->begin();
	Span::iterator end = this->end();
	std::string::const_iterator ritr = right.begin();
	while (itr != end)
	{
		if (*itr != *ritr)
			return false;
		itr++;
		ritr++;
	};
	return true;
}

bool Span::operator!=(const std::string &right) const
{
	if (this->length != right.size())
		return true;
	if (&this->value == &right && this->offset == 0)
		return false;
	Span::iterator itr = this->begin();
	Span::iterator end = this->end();
	std::string::const_iterator ritr = right.begin();
	while (itr != end)
	{
		if (*itr != *ritr)
			return true;
		itr++;
		ritr++;
	};
	return false;
}

bool Span::contains(char c) const
{
	if (this->length == 0)
		return false;
	for (Span::iterator itr = this->begin(); itr != this->end(); itr++)
	{
		if (*itr == c)
			return true;
	}
	return false;
}

std::string::size_type Span::find(char c) const
{
	Span::iterator begin = this->begin();
	Span::iterator end = this->end();
	Span::iterator found = std::find(begin, end, c);
	if (found == end)
		return std::string::npos;
	return found - begin;
}

std::string::size_type Span::rfind(char c) const
{
	for (std::string::size_type i = this->length; i-- > 0;)
	{
		if ((*this)[i] == c)
			return i;
	}
	return std::string::npos;
}

bool Span::starts_with(const char *str) const
{
	return StringHelper::starts_with(this->begin(), this->end(), str);
}

bool Span::ends_with(const char *str) const
{
	return StringHelper::ends_with(this->begin(), this->end(), str);
}

Span Span::slice(std::string::size_type offset) const
{
	return Span(*this, offset);
}

Span Span::slice(std::string::size_type offset, std::string::size_type length) const
{
	return Span(*this, offset, length);
}

bool Span::operator==(const Span &right) const
{
	if (this->length != right.length)
		return false;
	if (&this->value == &right.value && this->offset == 0)
		return true;
	Span::iterator itr = this->begin();
	Span::iterator end = this->end();
	Span::iterator ritr = right.begin();
	while (itr != end)
	{
		if (*itr != *ritr)
			return false;
		itr++;
		ritr++;
	};
	return true;
}

bool Span::operator!=(const Span &right) const
{
	if (this->length != right.length)
		return true;
	if (&this->value == &right.value && this->offset == 0)
		return false;
	Span::iterator itr = this->begin();
	Span::iterator end = this->end();
	Span::iterator ritr = right.begin();
	while (itr != end)
	{
		if (*itr != *ritr)
			return true;
		itr++;
		ritr++;
	};
	return false;
}

std::string Span::to_string() const
{
	return this->length != 0 ? this->value.substr(this->offset, this->length) : std::string();
}
