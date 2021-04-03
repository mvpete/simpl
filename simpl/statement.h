#ifndef __simpl_statement_h__
#define __simpl_statement_h__

#include "expression.h"
#include "value.h"

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <stdexcept>


namespace simpl
{
	
	class call_statement;
	class let_statement;
	class if_statement;

	class statement_visitor
	{
	public:
		virtual ~statement_visitor() = default;
		virtual void visit(call_statement &cs) = 0;
		virtual void visit(let_statement &cs) = 0;
		virtual void visit(if_statement &is) = 0;
	};

	struct statement
	{
		virtual ~statement() {};
		virtual void evaluate(statement_visitor &v) = 0;
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

		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}

		const std::string &function() const
		{
			return function_;
		}
		const expression_ptr &expr() const
		{
			return expr_;
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

		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}
		const std::string &name() const
		{
			return name_;
		}
		const expression_ptr &expr() const
		{
			return expr_;
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

		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}

		const statement_ptr &statement() const
		{
			return doif_;
		}
		const expression_ptr &cond() const
		{
			return cond_;
		}

	private:
		
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
		virtual void evaluate(statement_visitor &v) override
		{
			for (const auto &stmt : statements_)
				stmt->evaluate(v);
		}

	};



}

#endif // __simpl_statement_h__
