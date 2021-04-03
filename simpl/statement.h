#ifndef __simpl_statement_h__
#define __simpl_statement_h__

#include "expression.h"
#include "vm.h"
#include "value.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <stdexcept>


namespace simpl
{
	

	struct statement
	{
		virtual ~statement() {};
		virtual void evaluate(vm &vm) = 0;
	};
	using statement_ptr = std::unique_ptr<statement>;

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
			if (expr_)
				expr_->evaluate(vm);
			vm.call(function_);
		}
	};
	/// <summary>
	/// let {identifier} = {expression}[;]
	/// </summary>
	class let_statement : public statement
	{
		std::string name_;
		expression_ptr expr_;
	public:
		let_statement(const std::string &name, expression_ptr expr)
			:name_(name), expr_(std::move(expr))
		{
		}

		virtual void evaluate(vm &vm) override
		{
			value_t v;
			if (expr_)
			{
				expr_->evaluate(vm);
				v = vm.pop_stack();
			}

			vm.create_var(name_, v);
		}
	};

	class if_statement : public statement
	{
		expression_ptr cond_;
		statement_ptr doif_;
	public:
		if_statement(expression_ptr cond, statement_ptr doif)
			:cond_(std::move(cond)), doif_(std::move(doif))
		{
		}

		virtual void evaluate(vm &vm) override
		{
			if (cond_)
				cond_->evaluate(vm);
			
			const auto val = vm.pop_stack();
			if (is_true(val))
			{
				doif_->evaluate(vm);
			}
		}

	private:
		bool is_true(const value_t &v)
		{
			if (v.index() == 0)
				return false;
			if (v.index() == 1)
			{
				return std::get<int>(v) > 0;
			}
			else if (v.index() == 2)
			{
				const auto &strval = std::get<std::string>(v);
				return strval == "true" || strval == "1" || strval == "t" || strval == "TRUE" || strval == "T";
			}
		}
	};

	class block_statement : public statement
	{
		std::vector<statement_ptr> statements_;
	public:
		block_statement() {};
		void add(statement_ptr stmt)
		{
			statements_.emplace_back(std::move(stmt));
		}
	};


	class statement_visitor
	{
	public:
		virtual ~statement_visitor() {};
		virtual void visit(call_statement &cs) = 0;
		virtual void visit(let_statement &cs) = 0;
		virtual void visit(if_statement &is) = 0;
		virtual void visit(block_statement &bs) = 0;
	};
}

#endif // __simpl_statement_h__
