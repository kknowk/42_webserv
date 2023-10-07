#ifndef EITHER_HPP
#define EITHER_HPP

template <typename T0, typename T1>
class Either
{
public:
	bool is_item0;
	T0 item0;
	T1 item1;

	Either()
	{
	}

	Either(const T0 &item0)
	{
		this->item0 = item0;
		this->is_item0 = true;
	}

	Either(const T1 & item1)
	{
		this->item1 = item1;
		this->is_item0 = false;
	}

	Either(const Either<T0, T1> &right)
		: is_item0(right.is_item0)
	{
		*this = right;
	}

	~Either()
	{
	}

	Either<T0, T1> &operator=(const Either<T0, T1> &right)
	{
		if (this != &right)
		{
			this->is_item0 = right.is_item0;
			this->item0 = right.item0;
			this->item1 = right.item1;
		}
		return *this;
	}
};

#endif