#ifndef __simpl_expression_h__
#define __simpl_expression_h__

#include "value.h"
#include "vm.h"

#include <memory>
#include <variant>

namespace simpl
{
	class expression;
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

		virtual void evaluate(vm &vm)
		{
			if (value_.index() == 0)
				throw  std::logic_error("invalid expression");
			if (value_.index() == 1)
			{
				vm.push_stack(std::get<value_t>(value_));				
			}
			else if (value_.index() == 2)
			{
				std::get<expression_ptr>(value_)->evaluate(vm);
			}
			else if (value_.index() == 3)
			{
				auto id = std::get<identifier>(value_);
				vm.load_var(id.name);
			}
		}

		virtual ~expression() {};
	private:
		value_type value_;
	};

	class nary_expression : public expression
	{
		op_type op_;
		std::vector<expression_ptr> expressions_;
	public:
		nary_expression(op_type op)
			:op_(op)
		{
		}
		void add(expression_ptr exp)
		{
			expressions_.emplace_back(std::move(exp));
		}
		void add(std::vector<expression_ptr> &&expressions)
		{
			expressions_ = std::move(expressions);
		}
		virtual void evaluate(vm &vm) override
		{
			switch (op_)
			{
			case op_type::add:
			{
				do_add(vm);
				break;
			}
			case op_type::sub:
			{
				do_sub(vm);
				break;
			}
			case op_type::div:
			{
				do_div(vm);
				break;
			}
			case op_type::mult:
			{
				do_mult(vm);
				break;
			}
			}
		}

	private:
		void do_add(vm &vm)
		{
			if (expressions_.size() != 2)
				throw std::logic_error("invalid add");
			auto &left = expressions_[1];
			auto &right = expressions_[0];

			left->evaluate(vm);
			right->evaluate(vm);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) + std::get<int>(rvalue));
		}
		void do_sub(vm &vm)
		{
			if (expressions_.size() != 2)
				throw std::logic_error("invalid add");
			auto &left = expressions_[1];
			auto &right = expressions_[0];

			left->evaluate(vm);
			right->evaluate(vm);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) - std::get<int>(rvalue));
		}
		void do_div(vm &vm)
		{
			if (expressions_.size() != 2)
				throw std::logic_error("invalid add");
			auto &left = expressions_[1];
			auto &right = expressions_[0];

			left->evaluate(vm);
			right->evaluate(vm);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) / std::get<int>(rvalue));
		}
		value_t do_mult(vm &vm)
		{
			if (expressions_.size() != 2)
				throw std::logic_error("invalid add");
			auto &left = expressions_[1];
			auto &right = expressions_[0];

			left->evaluate(vm);
			right->evaluate(vm);

			auto rvalue = vm.pop_stack();
			auto lvalue = vm.pop_stack();

			vm.push_stack(std::get<int>(lvalue) * std::get<int>(rvalue));
		}

	};
}

#endif // __simpl_expression_h__
