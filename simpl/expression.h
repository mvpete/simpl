#ifndef __simpl_expression_h__
#define __simpl_expression_h__

#include "value.h"
#include "op.h"

#include <memory>
#include <variant>
#include <vector>

namespace simpl
{
	class expression;
	class nary_expression;

	class expression_visitor
	{
	public:
		virtual ~expression_visitor() = default;
		virtual void visit(expression &cs) = 0;
		virtual void visit(nary_expression &cs) = 0;
	};


	using expression_ptr = std::unique_ptr<expression>;

	// a simpl expression tree, either a value, or an expression.
	class expression
	{
		using value_type = std::variant<empty_t, value_t, expression_ptr, identifier>;

	public:
		expression()
			:value_(empty_t{})
		{
		}
		expression(const value_t &v)
			:value_(v)
		{
		}
		expression(expression_ptr exp)
			:value_(std::move(exp))
		{
		}
		expression(const identifier &i)
			:value_(i)
		{

		}

		virtual void evaluate(expression_visitor &v)
		{
			v.visit(*this);
		}

		const value_type& value() const
		{
			return value_;
		}

		virtual ~expression() =default;
	private:
		value_type value_;
	};

	class nary_expression : public expression
	{
		op_type op_;
		const std::string fn_name_;
		std::vector<expression_ptr> expressions_;
	public:
		nary_expression(const std::string &name, std::vector<expression_ptr> expr)
			:op_(op_type::func), fn_name_(name), expressions_(std::move(expr))
		{
		}
		nary_expression(op_type op)
			:op_(op)
		{
		}
		const std::string &function() const
		{
			return fn_name_;
		}
		void add(expression_ptr exp)
		{
			expressions_.emplace_back(std::move(exp));
		}
		void add(std::vector<expression_ptr> &&expressions)
		{
			expressions_ = std::move(expressions);
		}

		const op_type &op() const 
		{
			return op_;
		}
		const std::vector<expression_ptr> &expressions() const
		{
			return expressions_;
		}

		virtual void evaluate(expression_visitor &v) override
		{
			 v.visit(*this);
		}

	private:
		

	};

}

#endif // __simpl_expression_h__
