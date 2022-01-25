#ifndef __simpl_expression_h__
#define __simpl_expression_h__

#include <simpl/value.h>
#include <simpl/op.h>
#include <simpl/detail/type_traits.h>

#include <memory>
#include <variant>
#include <vector>

namespace simpl
{
	class expression;
	class nary_expression;
	class new_blob_expression;
	class new_array_expression;
	class new_object_expression;

	using indexor = std::variant<std::string, size_t>;
	struct identifier 
	{ 
		std::string name; 
		std::vector<indexor> path; 

		identifier() = default;
		identifier(const std::string &name)
			:name(name)
		{
		}

		template <typename T>
		void push_path(T &&val)
		{
			path.emplace_back(std::forward<T>(val));
		}
	};

	struct argument
	{
		argument(const std::string &name)
			:name(name)
		{
		}

		argument(const std::string &name, const std::string &type_name)
			:name(name), type(type_name)
		{
		}

		std::string name;
		std::optional<std::string> type;
	};

	using expression_ptr = std::unique_ptr<expression>;

	struct object_definition
	{
		struct member
		{
			member(const std::string &name)
				:name(name)
			{
			}
			member(const std::string &name, expression_ptr init)
				:name(name), initializer(std::move(init))
			{
			}


			std::string name;
			expression_ptr initializer;
		};

		std::string name;
		std::optional<std::string> is_a;
	};

	class expression_visitor
	{
	public:
		virtual ~expression_visitor() = default;
		virtual void visit(expression &cs) = 0; // this needs a rework
		virtual void visit(nary_expression &cs) = 0;
		virtual void visit(new_blob_expression &ns) = 0;
		virtual void visit(new_array_expression &nas) = 0;
		virtual void visit(new_object_expression &nos) = 0;
	};


	struct initializer 
	{ 
		initializer(std::string &&id, expression_ptr expr)
			:identifier(std::move(id)), expr(std::move(expr))
		{
		}

		std::string identifier; 
		expression_ptr expr; 
	};
	using initializer_list_t = std::vector<initializer>;
	using expression_list_t = std::vector<expression_ptr>;

	// a simpl expression tree, either a value, or an expression.
	class expression
	{
	public:
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
		const identifier identifier_;
		std::vector<expression_ptr> expressions_;
	public:
		nary_expression(const identifier &id, std::vector<expression_ptr> expr)
			:op_(op_type::func), identifier_(id), expressions_(std::move(expr))
		{
		}
		nary_expression(op_type op)
			:op_(op)
		{
		}
		const identifier &identifier() const
		{
			return identifier_;
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

	class new_blob_expression : public expression
	{
	public:
		new_blob_expression(initializer_list_t &&init)
			:initializers_(std::move(init))
		{
		}

		const initializer_list_t& initializers()
		{
			return initializers_;
		}

		virtual void evaluate(expression_visitor &v) override
		{
			v.visit(*this);
		}

	private:
		initializer_list_t initializers_;
	};

	class new_object_expression : public expression
	{
	public:
		new_object_expression(const std::string &type, initializer_list_t &&init)
			:type_(type), initializers_(std::move(init))
		{
		}

		const std::string &type() const
		{
			return type_;
		}

		const initializer_list_t &initializers()
		{
			return initializers_;
		}

		virtual void evaluate(expression_visitor &v) override
		{
			v.visit(*this);
		}

	private:
		std::string type_;
		initializer_list_t initializers_;

	};

	class new_array_expression : public expression
	{
	public:
		new_array_expression(expression_list_t &&init)
			:expressions_(std::move(init))
		{
		}

		const expression_list_t &expressions()
		{
			return expressions_;
		}

		virtual void evaluate(expression_visitor &v) override
		{
			v.visit(*this);
		}

	private:
		expression_list_t expressions_;
	};

}

#endif // __simpl_expression_h__
