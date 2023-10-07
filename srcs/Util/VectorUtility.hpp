#ifndef VECTORUTILITY_HPP
#define VECTORUTILITY_HPP

#include <vector>

class VectorUtility
{
private:
	VectorUtility();

public:
	template <typename T>
	void remove(std::vector<T> &buffer, std::size_t index)
	{
		if (index >= buffer.size())
			return;
		if (index < buffer.size() - 1)
			buffer[index] = buffer.back();
		buffer.pop_back();
	}
};

#endif