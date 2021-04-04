#ifndef __simpl_op_h__
#define __simpl_op_h__

namespace simpl
{
	namespace builtins
	{
		const char *print_fn = "print";

		template <typename IteratorT, typename Iterator2T>
		bool compare(IteratorT begin, IteratorT end, Iterator2T val)
		{
			auto len = end - begin;
			if (strlen(val) != len)
				return false;

			while (begin != end)
			{
				if (*begin != *val)
					return false;
				++val;
				++begin;
			}
			return true;
		}

	}

	enum class op_type { add, sub, mult, div, mod, eq, eqeq, neq, exp, gt, lt, gteq, lteq, none };

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
		case op_type::gt:
		case op_type::lt:
		case op_type::gteq:
		case op_type::lteq:
		case op_type::eqeq:
		case op_type::neq:
			return 7;
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
		case op_type::gt:
		case op_type::lt:
		case op_type::gteq:
		case op_type::lteq:
		case op_type::eqeq:
		case op_type::neq:
			return 2;
		}
	}

	bool is_op(char c)
	{
		return c == '+' || c == '-' || c == '*' || c == '/' || c == '=' || c == '!' || c == '<' || c == '>';
	}

	template <typename IteratorT>
	op_type to_op_type(IteratorT begin, IteratorT end)
	{
		if (builtins::compare(begin, end, "+"))
			return op_type::add;
		else if (builtins::compare(begin, end, "-"))
			return op_type::sub;
		else if (builtins::compare(begin, end, "/"))
			return op_type::div;
		else if (builtins::compare(begin, end, "*"))
			return op_type::mult;
		else if (builtins::compare(begin, end, "=="))
			return op_type::eqeq;
		else if (builtins::compare(begin, end, "!="))
			return op_type::neq;
		else if (builtins::compare(begin, end, "<"))
			return op_type::lt;
		else if (builtins::compare(begin, end, "<="))
			return op_type::lteq;
		else if (builtins::compare(begin, end, ">"))
			return op_type::gt;
		else if (builtins::compare(begin, end, ">="))
			return op_type::gteq;
		return op_type::none;
	}


}

#endif // __simpl_op_h__