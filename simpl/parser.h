#ifndef __simpl_parser_h__
#define __simpl_parser_h__

#include "tokenizer.h"
#include "statement.h"

#include <memory>
#include <stack>
#include <vector>
#include <variant>



/*
	IF { expr } THEN { statement }
	PRINT { expr }

	statement := statement | expression
*/
namespace simpl
{
	enum keywords
	{
		if_keyword,
		else_keyword,
		let_keyword,
		def_keyword,
		end_keyword,
		while_keyword,
		unknown_keyword,
	};


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

		template <typename IteratorT, typename Iterator2T>
		bool compare(token<IteratorT> &tkn, IteratorT val)
		{
			return compare(tkn.begin, tkn.end, val);
		}
	}

	class parse_error : public std::exception
	{
	public:
		parse_error(const char *msg)
			:std::exception(msg)
		{
		}
	};

	using parse_val = std::variant<empty_t, op_type, identifier, value_t>;

	class parser
	{
	public:
		using tokenizer_t = tokenizer<char>;
		using token_t = typename tokenizer_t::token_t;
		using statement_t = statement;

	public:
		parser(const char *begin, const char *end)
			:tokenizer_(begin, end)
		{
		}

		statement_ptr next()
		{
			auto t = tokenizer_.next();
			return parse_statement(t);
		}

	private:
		statement_ptr parse_statement(token_t &t)
		{
			if (t.type == token_types::eof)
				return nullptr;

			// the first identifier, has to bea keyword
			auto kw = to_keyword(t);
			switch (kw)
			{
			case keywords::let_keyword:
				return parse_let_statement(t);
			case keywords::if_keyword:
				return parse_if_statement(t);
			}
			// peek the next token
			auto nxt = tokenizer_.peek();
			if (nxt.type == token_types::lparen)
			{
				tokenizer_.next();
				if (kw != unknown_keyword)
				{
					throw parse_error("not implemented");
				}
				return parse_function_call_statement(t);
			}
			std::stringstream ss;
			ss << "unexpected identifier - " << t.to_string() << std::endl;
			throw parse_error(ss.str().c_str());
		}

		parse_val next_val()
		{
			auto tkn = tokenizer_.peek();
			if (tkn.type == token_types::literal)
			{
				tokenizer_.next();
				return std::string(tkn.begin, tkn.end);
			}
			if (tkn.type == token_types::identifier_token)
			{
				return identifier{ std::string(tkn.begin, tkn.end) };
			}
			if (tkn.type == token_types::number)
			{
				tokenizer_.next();
				return std::stoi(std::string(tkn.begin, tkn.end));
			}
			if (tkn.type == token_types::op)
			{
				tokenizer_.next();
				return to_op(tkn);
			}
			return parse_val{};
		}

		expression_ptr parse_expression()
		{
			std::stack<expression_ptr> ostack;
			std::stack<parse_val> opstack;

			auto val = next_val();

			if (val.index() == 0)
				return nullptr;

			if (val.index() < 1)
				throw parse_error("expected an value or identifier");

			while (val.index() != 0)
			{
				if (val.index() > 1)
				{
					if (val.index() == 3)
					{
						ostack.push(std::make_unique<expression>(std::get<value_t>(val)));
					}
					else
					{
						ostack.push(std::make_unique<expression>(std::get<identifier>(val)));
					}
				}
				else if (val.index() == 1)
				{
					auto pr = get_precendence(std::get<op_type>(val));
					if (!opstack.empty() && pr <= get_precendence(std::get<op_type>(opstack.top())))
					{
						make_op_expression(ostack, opstack);
					}
					opstack.push(val);
				}
				val = next_val();
			}
			while (!opstack.empty())
			{
				make_op_expression(ostack, opstack);
			}
			return std::move(ostack.top());
		}

		void make_op_expression(std::stack<expression_ptr> &ostack, std::stack<parse_val> &opstack)
		{
			const auto op = opstack.top();
			opstack.pop();
			size_t card = get_cardinality(std::get<op_type>(op));
			if (card > ostack.size())
				throw parse_error("not enough arguments");
			std::vector<expression_ptr> exp;
			while (card)
			{
				exp.push_back(std::move(ostack.top()));
				ostack.pop();
				--card;
			}
			auto nexp = std::make_unique<nary_expression>(std::get<op_type>(op));
			nexp->add(std::move(exp));
			ostack.push(std::move(nexp));
		}

		statement_ptr parse_function_call_statement(token_t &t)
		{
			auto expr = parse_expression();
			auto close = tokenizer_.next();
			if (close.type != token_types::rparen)
				throw parse_error("expected a ')'");
			close_statement();
			return std::make_unique<call_statement>(t.to_string(), std::move(expr));
		}

		// let {identifier} = {expression};
		statement_ptr parse_let_statement(token_t &t)
		{
			auto identifier = tokenizer_.next();
			if (identifier.type != token_types::identifier_token)
				throw parse_error("expected an identifier");
			auto eq = tokenizer_.next();
			expression_ptr expr;
			if (eq.type != token_types::empty_token && eq.type != token_types::eos && eq.type != token_types::eof)
			{
				if (eq.type != token_types::op || !builtins::compare(eq.begin, eq.end, "="))
					throw parse_error("expected an '='");
				expr = parse_expression();
			}
			close_statement();
			return std::make_unique<let_statement>(identifier.to_string(), std::move(expr));
		}

		statement_ptr parse_block_statement()
		{
			auto next = tokenizer_.next();
			if (next.type != token_types::lbrack)
				return parse_statement(next);
			auto blk = std::make_unique<block_statement>();
			while (tokenizer_.peek().type != token_types::rbrack)
			{
				auto tkn = tokenizer_.next();
				auto stmt = parse_statement(tkn);
				if (stmt)
					blk->add(std::move(stmt));
			}
			tokenizer_.next(); // consume {
			return blk;
		}

		// if(cond) { // list of statements } else
		statement_ptr parse_if_statement(token_t &t)
		{
			expect(token_types::lparen);
			auto cond = parse_expression();
			if (cond == nullptr)
				throw parse_error("expected an expression");
			expect(token_types::rparen);
			auto statement = parse_block_statement();

			return std::make_unique<if_statement>(std::move(cond), std::move(statement));

		}

		void close_statement()
		{
			if (tokenizer_.next().type != token_types::eos)
				throw parse_error("expected a ';'");
		}

		keywords to_keyword(token_t &t)
		{
			if (builtins::compare(t.begin, t.end, "if"))
				return keywords::if_keyword;
			else if (builtins::compare(t.begin, t.end, "def"))
				return keywords::def_keyword;
			else if (builtins::compare(t.begin, t.end, "let"))
				return keywords::let_keyword;
			return keywords::unknown_keyword;
		}

		op_type to_op(token_t &t)
		{
			if (builtins::compare(t.begin, t.end, "+"))
				return op_type::add;
			else if (builtins::compare(t.begin, t.end, "-"))
				return op_type::sub;
			else if (builtins::compare(t.begin, t.end, "/"))
				return op_type::div;
			else if (builtins::compare(t.begin, t.end, "*"))
				return op_type::mult;
			else if (builtins::compare(t.begin, t.end, "=="))
				return op_type::eqeq;
			return op_type::none;
		}

		
	private:
		token_t expect(token_types ttype)
		{
			auto tkn = tokenizer_.next();
			if (tkn.type != ttype)
				throw parse_error("expected a token");
			return tkn;
		}


	private:
		tokenizer_t tokenizer_;

	};


	
}

#endif // __simpl_parser_h__