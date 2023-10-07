#ifndef NULLABLE_HPP
#define NULLABLE_HPP

template <typename T>
class Nullable
{
public:
	T value;
	bool has_value;
	Nullable(void) : has_value(false) {}
	Nullable(const T &value) : value(value), has_value(true) {}
	Nullable(const Nullable<T> &right) : value(right.value), has_value(right.has_value) {}
	~Nullable() {}

	Nullable<T> &operator=(const Nullable<T> &right)
	{
		if (this != &right)
		{
			this->value = right.value;
			this->has_value = right.has_value;
		}
		return *this;
	}

	Nullable<T> &operator=(const T &right)
	{
		this->has_value = true;
		this->value = right;
		return *this;
	}
};

#endif