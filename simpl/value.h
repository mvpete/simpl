#ifndef __simpl_value_h__
#define __simpl_value_h__

#include <sstream>
#include <string>
#include <variant>

namespace simpl
{
	struct empty_t {};
	struct identifier { std::string name;  };
	enum class op_type { add, sub, mult, div, mod, eq, neq, exp, none};

	int get_precendence(op_type op)
	{
		switch (op)
		{
		default:
			return 1;
		case op_type::add:
		case op_type::sub:
			return 2;
		case op_type::mult:
		case op_type::div:
		case op_type::mod:
			return 3;
		case op_type::exp:
			return 4;
		}
	}

	size_t get_cardinality(op_type op)
	{
		switch (op)
		{
		default:
			return 1;
		case op_type::add:
		case op_type::sub:
		case op_type::mult:
		case op_type::div:
		case op_type::mod:
		case op_type::exp:
			return 2;
		}
	}

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