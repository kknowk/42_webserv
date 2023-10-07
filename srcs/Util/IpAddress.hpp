#ifndef IPADDRESS_HPP
#define IPADDRESS_HPP

#include <vector>
#include <string>

class IpAddress
{
private:
	IpAddress();
public:
	/// @return false: FAIL
	static bool append_normalize_v6(std::vector<char> &buffer, const std::string &value);
	static bool append_normalize_v4(std::vector<char> &buffer, const std::string &value);
	static bool is_valid_v6(const std::string &value);
	static bool is_valid_v4(const std::string &value);
};

#endif