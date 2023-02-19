#ifndef __simpl_statement_h__
#define __simpl_statement_h__

#include <simpl/expression.h>
#include <simpl/value.h>

#include <memory>
#include <string>
#include <variant>
#include <vector>

#include <stdexcept>


namespace simpl
{

	class expr_statement;
	class let_statement;
	class if_statement;
	class def_statement;
	class return_statement;
	class while_statement;
	class for_statement;
	class block_statement;
	class assignment_statement;
	class object_definition_statement;
	class import_statement;
	class load_library_statement;

	class statement_visitor
	{
	public:
		virtual ~statement_visitor() = default;
		virtual void visit(expr_statement &cs) = 0;
		virtual void visit(let_statement &cs) = 0;
		virtual void visit(if_statement &is) = 0;
		virtual void visit(def_statement &ds) = 0;
		virtual void visit(return_statement &rs) = 0;
		virtual void visit(while_statement &ws) = 0;
		virtual void visit(for_statement &fs) = 0;
		virtual void visit(block_statement &bs) = 0;
		virtual void visit(assignment_statement &ws) = 0;
		virtual void visit(object_definition_statement &os) = 0;
		virtual void visit(import_statement& is) = 0;
		virtual void visit(load_library_statement& lls) = 0;
	};

	struct statement
	{
		virtual ~statement() {};
		virtual void evaluate(statement_visitor &v) = 0;
	};

	using statement_ptr = std::unique_ptr<statement>;
	using syntax_tree = std::vector<statement_ptr>;

	class expr_statement : public statement
	{
		expression_ptr expr_;
	public:
		expr_statement(expression_ptr expr)
			:expr_(std::move(expr))
		{
		}

		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
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
		std::unique_ptr<if_statement> next_;
		statement_ptr else_;
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

		const std::unique_ptr<if_statement> &next() const
		{
			return next_;
		}

		void next(std::unique_ptr<if_statement> next)
		{
			next_ = std::move(next);
		}

		const statement_ptr &else_statement() const
		{
			return else_;
		}

		void else_statement(statement_ptr else_st)
		{
			else_ = std::move(else_st);
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
			v.visit(*this);
		}
		const std::vector<statement_ptr> &statements() const
		{
			return statements_;
		}

	};

	class def_statement : public statement
	{

	public:
		def_statement(const std::string &name, std::vector<argument> &&id_list, statement_ptr statement)
			:name_(name), arguments_(std::move(id_list)), statement_(std::move(statement))
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

		const std::vector<argument> &arguments()
		{
			return arguments_;
		}

		statement_ptr release_statement()
		{
			return std::move(statement_);
		}

	private:
		std::string name_;
		std::vector<argument> arguments_;
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

	class return_statement : public statement
	{
	public:
		return_statement(expression_ptr expr)
			:expr_(std::move(expr))
		{
		}
		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}
		const expression_ptr &expr() const
		{
			return expr_;
		}
	private:
		expression_ptr expr_;

	};

	class for_statement : public statement
	{
	public:
		for_statement(statement_ptr init, expression_ptr cond, statement_ptr incr, statement_ptr block)
			:init_(std::move(init)), cond_(std::move(cond)), incr_(std::move(incr)), block_(std::move(block))
		{
		}
		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}
		const statement_ptr &init() const
		{
			return init_;
		}
		
		const expression_ptr &cond() const
		{
			return cond_;
		}

		const statement_ptr &incr() const
		{
			return incr_;
		}

		const statement_ptr &block() const
		{
			return block_;
		}

	private:
		statement_ptr init_;
		expression_ptr cond_;
		statement_ptr incr_;
		statement_ptr block_;
	};

	class object_definition_statement : public statement
	{
	public:

		object_definition_statement(const std::string &type, std::vector<object_definition::member> members)
			:type_(type), members_(std::move(members))
		{
		}

		object_definition_statement(const std::string &type, const std::optional<std::string> &inherits, std::vector<object_definition::member> members)
			:type_(type), inherits_(inherits), members_(std::move(members))
		{
		}

		virtual void evaluate(statement_visitor &v) override
		{
			v.visit(*this);
		}

		const std::string &type_name() const
		{
			return type_;
		}

		const std::optional<std::string> &inherits() const
		{
			return inherits_;
		}

		const std::vector<object_definition::member>& members() const
		{
			return members_;
		}

		std::vector<object_definition::member> move_members()
		{
			return std::move(members_);
		}

	private:
		std::string type_;
		std::optional<std::string> inherits_;

		std::vector<object_definition::member> members_;
	};

	class import_statement : public statement
	{
	public:

		import_statement(const std::string& libname)
			:libname_(libname)
		{
		}

		virtual void evaluate(statement_visitor& v) override
		{
			v.visit(*this);
		}

		const std::string& libname() const
		{
			return libname_;
		}

	private:
		std::string libname_;

	};

	class load_library_statement : public statement
	{
	public:
		load_library_statement(const std::string& path)
			:path_(path)
		{
		}

		virtual void evaluate(statement_visitor& v) override
		{
			v.visit(*this);
		}

		const std::string& path() const
		{
			return path_;
		}

	private:
		std::string path_;
	};
}

#endif // __simpl_statement_h__
