#ifndef __simpl_statement_h__
#define __simpl_statement_h__

#include "vm.h"
#include "value.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <stdexcept>


namespace simpl
{
	enum keywords
	{
		if_keyword,
		else_keyword,
		def_keyword,
		end_keyword,
		while_keyword,
		unknown_keyword,
	};

	class expression;
	using expression_ptr = std::unique_ptr<expression>;

	class expression
	{
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

		virtual void evaluate(vm &vm)
		{
			if (value_.index() == 0)
				throw  std::logic_error("invalid expression");
			if (value_.index() == 1)
				return vm.push_stack(std::get<value_t>(value_));
			return std::get<expression_ptr>(value_)->evaluate(vm);
		}

		virtual ~expression() {};
	private:
		std::variant<empty_t, value_t, expression_ptr> value_;
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


	struct statement
	{
		virtual ~statement() {};
		virtual void evaluate(vm &vm) = 0;
	};

	class call_statement : public statement
	{
		std::string function_;
		expression_ptr expr_;
	public:
		call_statement(const std::string &function, expression_ptr expr)
			:function_(function), expr_(std::move(expr))
		{
		}

		virtual void evaluate(vm &vm) override
		{
			expr_->evaluate(vm);
			vm.call(function_);
		}
	};


	using statement_ptr = std::unique_ptr<statement>;


}

#endif // __simpl_statement_h__
