#ifndef __simpl_value_h__
#define __simpl_value_h__

#include <sstream>
#include <string>
#include <variant>

namespace simpl
{
	struct empty_t {};
	struct identifier { std::string name;  };

	using value_t = std::variant<empty_t,int,std::string>;

	std::string to_string(const value_t &r)
	{
		switch (r.index())
		{
		default:
		case 0:
			return "null";
		case 1:
		{
			std::stringstream ss;
			ss << std::get<int>(r);
			return ss.str();
		}
		case 2:
			return std::get<std::string>(r);
		}
	}
}

#endif //__simpl_value_h__