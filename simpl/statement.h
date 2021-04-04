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
	class def_statement;
	class while_statement;
	class assignment_statement;

	class statement_visitor
	{
	public:
		virtual ~statement_visitor() = default;
		virtual void visit(call_statement &cs) = 0;
		virtual void visit(let_statement &cs) = 0;
		virtual void visit(if_statement &is) = 0;
		virtual void visit(def_statement &ds) = 0;
		virtual void visit(while_statement &ws) = 0;
		virtual void visit(assignment_statement &ws) = 0;
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
		std::vector<expression_ptr> expr_;
	public:
		call_statement(const std::string &function, std::vector<expression_ptr> &&expr)
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
		const std::vector<expression_ptr> &expr() const
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

	class def_statement : public statement
	{

	public:
		def_statement(const std::string &name, std::vector<identifier> &&id_list, statement_ptr statement)
			:name_(name), identifiers_(std::move(id_list)), statement_(std::move(statement))
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

		const std::vector<identifier> &identifiers()
		{
			return identifiers_;
		}

		statement_ptr release_statement()
		{
			return std::move(statement_);
		}

	private:
		std::string name_;
		std::vector<identifier> identifiers_;
		statement_ptr statement_;
	};

	class while_statement : public statement
	{
	public:
		while_statement(expression_ptr cond, statement_ptr block)
			:cond_(std::move(cond)), block_(std::move(block))
		{
		}
		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}
		const expression_ptr &cond() const
		{
			return cond_;
		}
		const statement_ptr &block() const
		{
			return block_;
		}

	private:
		expression_ptr cond_;
		statement_ptr block_;
	};

	class assignment_statement : public statement
	{
	public:
		assignment_statement(const std::string &id, expression_ptr expr)
			:identifier_(id), expression_(std::move(expr))
		{
		}
		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}

		const std::string &identifier() const
		{
			return identifier_;
		}
		const expression_ptr &expr() const
		{
			return expression_;
		}

	private:
		std::string identifier_;
		expression_ptr expression_;
	};
}

#endif // __simpl_statement_h__
