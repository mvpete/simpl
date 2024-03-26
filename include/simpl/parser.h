#ifndef __simpl_parser_h__
#define __simpl_parser_h__

#include <simpl/detail/format.h>
#include <simpl/tokenizer.h>
#include <simpl/statement.h>

#include <memory>
#include <stack>
#include <string>
#include <vector>
#include <variant>

/*
*/
namespace simpl
{
	enum class keywords
	{
		if_keyword,
		keyword_is,
		else_keyword,
		let_keyword,
		def_keyword,
		for_keyword,
		end_keyword,
		new_keyword,
		while_keyword,
		return_keyword,
		object_keyword,
		unknown_keyword,
	};

	class parse_error : public std::exception
	{
	public:
		parse_error(const position &p, const char *msg)
			:std::exception(detail::format("syntax error {0}. ({1})", msg, p).c_str()), pos_(p)
		{
		}

		parse_error(const position &p, const std::string &msg)
			:std::exception(detail::format("syntax error {0}. ({1})", msg, p).c_str()), pos_(p)
		{
		}

	public:
		const position& pos() const
		{
			return pos_;
		}

	private:
		const position pos_;
	};

	struct address_identifier_t { std::string name;  };

	using parse_val = std::variant<empty_t, op_type, identifier, keywords, value_t, address_identifier_t>;

	class parser
	{
	public:
		using tokenizer_t = basic_tokenizer<char>;
		using token_t = typename tokenizer_t::token_t;
		using statement_t = statement;
		
	private:
		enum class scopes { main, function, while_, for_, object_ };
		void scope(scopes scope)
		{
			scope_ = scope;
		}

		scopes scope()
		{
			return scope_;
		}

		class change_scope
		{
			parser &parser_;
			scopes prev_;
		public:
			change_scope(parser &p, scopes next)
				:parser_(p), prev_(p.scope())
			{
				parser_.scope(next);
			}
			~change_scope()
			{
				parser_.scope(prev_);
			}
		};

	public:

		parser(const std::string &text)
			:tokenizer_(text.c_str(), text.c_str() + text.length()), scope_(scopes::main)
		{
		}

		parser(const char *begin, const char *end)
			:tokenizer_(begin, end), scope_(scopes::main)
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
			case keywords::def_keyword:
				return parse_def_statement(t);
			case keywords::while_keyword:
				return parse_while_statement(t);
			case keywords::for_keyword:
				return parse_for_statement(t);
			case keywords::return_keyword:
				return parse_return_statement(t);
			case keywords::object_keyword:
				return parse_object_statement(t);
			default:
				break;
			}

			if (t.type == token_types::directive)
			{
				return parse_directive_statement(t);
			}
			// back up one because it's an expression.
			tokenizer_.reverse(t);
			auto expr = parse_expression();
			if (expr == nullptr)
			{
				throw parse_error(tokenizer_.pos(), "expected an expression");
			}
			close_statement();

			return std::make_unique<expr_statement>(std::move(expr));
			
		}

		parse_val next_val()
		{
			auto tkn = tokenizer_.peek();
			if (tkn.type == token_types::literal)
			{
				tokenizer_.next();
				return value_t{ tkn.to_string() };
			}
			if (tkn.type == token_types::identifier_token)
			{
				tokenizer_.next();
				keywords kw;
				if (is_keyword(tkn, kw))
					return kw;

				return parse_identifier(tkn);
				
			}
			if (tkn.type == token_types::number)
			{
				tokenizer_.next();
				return value_t{ std::stod(tkn.to_string()) };
			}
			if (tkn.type == token_types::op && builtins::compare(tkn.begin, tkn.end, "&"))
			{
				tokenizer_.next();

				return address_identifier_t{ tokenizer_.next().to_string() };
			}
			if (tkn.type == token_types::op)
			{
				tokenizer_.next();
				return to_op(tkn);
			}
			return parse_val{};
		}

		identifier parse_identifier(const token_t &tkn)
		{
			identifier id{ tkn.to_string() };
			while (1)
			{
				auto n = tokenizer_.peek();
				auto nt = n.type;
				if (nt == token_types::op && builtins::compare(n.begin, n.end, "."))
				{
					tokenizer_.next();
					if (tokenizer_.peek().type != token_types::identifier_token)
						throw parse_error(tokenizer_.pos(), "expected an identifier");
					auto acc = tokenizer_.next();
					id.push_path(acc.to_string());
				}
				// variable[index]
				else if (nt == token_types::sqlbrack)
				{
					tokenizer_.next();
					auto tkn_type = tokenizer_.peek().type;
					if (tkn_type != token_types::identifier_token && tkn_type != token_types::number)
						throw parse_error(tokenizer_.pos(), "expected an identifier or number");

					auto acc = tokenizer_.next();
					if (tokenizer_.peek().type != token_types::sqrbrack)
						throw parse_error(tokenizer_.pos(), "expected a closing ']'");

					tokenizer_.next();
					indexor val;
					if (tkn_type == token_types::number)
						val = to<size_t>(acc.to_string());
					else
						val = acc.to_string();
					id.push_path(val);
				}
				else
					break;
			}
			return id;
		}

		expression_ptr parse_expression()
		{
			std::stack<expression_ptr> ostack;
			std::stack<parse_val> opstack;

			auto val = next_val();

			if (std::holds_alternative<empty_t>(val))
				return nullptr;

			if (val.index() < 1)
				throw parse_error(tokenizer_.pos(), "expected an value or identifier");
			bool expect_op = false;
			while (!std::holds_alternative<empty_t>(val)) // while
			{
				if (val.index() > 1)
				{
					if (expect_op)
						break;
					if (std::holds_alternative<value_t>(val))
					{
						ostack.push(std::make_unique<expression>(std::get<value_t>(val)));
					}
					else if (std::holds_alternative<identifier>(val))
					{
						auto id = std::get<identifier>(val);
						if (tokenizer_.peek().type == token_types::lparen)
						{
							tokenizer_.next(); // swallow (
							auto expr_list = parse_expression_list(token_types::rparen);
							// this is a function call. We want to then parse the expression list
							// and create an expression
							auto close = tokenizer_.next();
							if (close.type != token_types::rparen)
								throw parse_error(tokenizer_.pos(), "expected a ')'");
							ostack.push(std::make_unique<nary_expression>(id, std::move(expr_list)));
						}
						else
							ostack.push(std::make_unique<expression>(std::get<identifier>(val)));
					}
					else if (std::holds_alternative<keywords>(val))
					{
						auto keyword = std::get<keywords>(val);
						ostack.push(parse_keyword_expression(keyword));
					}
					else if (std::holds_alternative<address_identifier_t>(val))
					{
						auto address = std::get<address_identifier_t>(val);
						ostack.push(std::make_unique<function_address_expression>(address.name));
					}
					else
						throw parse_error(tokenizer_.pos(), "undefined parse_val");
					expect_op = true;
				}
				else if (std::holds_alternative<op_type>(val))
				{
					auto op = std::get<op_type>(val);
					if (op == op_type::increment || op == op_type::decrement) 
					{
						ostack.push(std::make_unique<expression>(empty_t{}));
					}
					auto pr = get_precendence(op);
					if (!opstack.empty() && pr <= get_precendence(std::get<op_type>(opstack.top())))
					{
						make_op_expression(ostack, opstack);
					}
					opstack.push(val);
					expect_op = false;
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
			size_t arity = get_arity(std::get<op_type>(op));
			if (arity > ostack.size())
				throw parse_error(tokenizer_.pos(), "not enough arguments");
			std::vector<expression_ptr> exp;
			while (arity)
			{
				exp.push_back(std::move(ostack.top()));
				ostack.pop();
				--arity;
			}
			auto nexp = std::make_unique<nary_expression>(std::get<op_type>(op));
			nexp->add(std::move(exp));
			ostack.push(std::move(nexp));
		}

		statement_ptr parse_function_call_statement(token_t &t)
		{
			auto exprs = parse_expression_list(token_types::rparen);
			auto close = tokenizer_.next();
			if (close.type != token_types::rparen)
				throw parse_error(tokenizer_.pos(), "expected a ')'");
			close_statement();
			return std::make_unique<expr_statement>(std::make_unique<nary_expression>(t.to_string(), std::move(exprs)));
		}

		// let {identifier} = {expression};
		statement_ptr parse_let_statement(token_t &)
		{
			auto identifier = tokenizer_.next();
			if (identifier.type != token_types::identifier_token)
				throw parse_error(tokenizer_.pos(), "expected an identifier");
			auto eq = tokenizer_.next();
			expression_ptr expr;
			if (eq.type != token_types::empty_token && eq.type != token_types::eos && eq.type != token_types::eof)
			{
				if (eq.type != token_types::op || !builtins::compare(eq.begin, eq.end, "="))
					throw parse_error(tokenizer_.pos(), "expected an '='");
				expr = parse_expression();
			}
			close_statement();
			return std::make_unique<let_statement>(identifier.to_string(), std::move(expr));
		}

		statement_ptr parse_block_statement()
		{
			if (maybe(token_types::lbrack))
			{
				auto blk = std::make_unique<block_statement>();
				while (tokenizer_.peek().type != token_types::rbrack)
				{
					if (tokenizer_.peek().type == token_types::eof)
						return nullptr;

					auto tkn = tokenizer_.next();
					auto stmt = parse_statement(tkn);
					if (stmt)
						blk->add(std::move(stmt));
				}
				tokenizer_.next(); // consume }
				return blk;
			}
			else
			{
				auto tkn = tokenizer_.next();
				return parse_statement(tkn);
			}
		}

		// if(cond) { // list of statements }
		statement_ptr parse_if_statement(token_t &t)
		{
			expect(token_types::lparen);
			auto cond = parse_expression();
			if (cond == nullptr)
				throw parse_error(tokenizer_.pos(), "expected an expression");
			expect(token_types::rparen);
			
			auto statement = parse_block_statement();

			if (statement == nullptr)
				throw parse_error(tokenizer_.pos(), "expected a statement");

			auto if_stmt = std::make_unique<if_statement>(std::move(cond), std::move(statement));
			auto else_if = if_stmt.get();
			while (1)
			{
				auto nxt = tokenizer_.peek();
				keywords kw;
				if (nxt.type != token_types::identifier_token || !is_keyword(nxt, kw) || kw != keywords::else_keyword)
					break;
				// it's at least else, consume
				tokenizer_.next();
				nxt = tokenizer_.peek();
				if (nxt.type != token_types::identifier_token || !is_keyword(nxt, kw) || kw != keywords::if_keyword)
				{
					auto else_block = parse_block_statement();
					// add it
					else_if->else_statement(std::move(else_block));
					break;
				}
				
				// else we parse a full if, consume if
				tokenizer_.next();

				expect(token_types::lparen);
				cond = parse_expression();
				if (cond == nullptr)
					throw parse_error(tokenizer_.pos(), "expected an expression");
				expect(token_types::rparen);
				statement = parse_block_statement();

				if (statement == nullptr)
					throw parse_error(tokenizer_.pos(), "expected a statement");

				auto next_if = std::make_unique<if_statement>(std::move(cond), std::move(statement));
				else_if->next(std::move(next_if));
				else_if = else_if->next().get();
			}
			return if_stmt;

		}

		statement_ptr parse_while_statement(token_t &t)
		{
			expect(token_types::lparen);
			auto cond = parse_expression();
			if (cond == nullptr)
				throw parse_error(tokenizer_.pos(), "expected an expression");
			expect(token_types::rparen);
			auto statement = parse_block_statement();

			return std::make_unique<while_statement>(std::move(cond), std::move(statement));
		}

		// for(expr;expr;expr)
		statement_ptr parse_for_statement(token_t &t)
		{
			expect(token_types::lparen);
			auto lt = tokenizer_.next();
			auto init = parse_let_statement(lt);
			auto cond = parse_expression();
			close_statement();
			auto incr = parse_expression();
			expect(token_types::rparen);
			change_scope sp(*this, scopes::for_);
			auto block = parse_block_statement();

			return std::make_unique<for_statement>(std::move(init), std::move(cond), std::move(incr), std::move(block));
		}

		statement_ptr parse_return_statement(token_t &t)
		{
			auto expr = parse_expression();
			close_statement();
			return std::make_unique<return_statement>(std::move(expr));
		}

		statement_ptr parse_directive_statement(token_t& t)
		{
			auto directive = tokenizer_.next();
			if (directive.type != token_types::identifier_token)
				throw parse_error(tokenizer_.pos(), "expected a directive.");

			if (builtins::compare(directive.begin, directive.end, "import"))
			{
				auto libtkn = tokenizer_.next();
				if (libtkn.type != token_types::identifier_token)
					throw parse_error(tokenizer_.pos(), detail::format("import expected an identifier: {0}", libtkn.to_string()));
				
				return std::make_unique<import_statement>(libtkn.to_string());
			}
			else if (builtins::compare(directive.begin, directive.end, "loadlib"))
			{
				auto pathtkn = tokenizer_.next();
				if (pathtkn.type != token_types::literal)
					throw parse_error(tokenizer_.pos(), detail::format("expected a path, got: {0}", pathtkn.to_string()));

				return std::make_unique<load_library_statement>(pathtkn.to_string());

			}
			else
				throw parse_error(tokenizer_.pos(), detail::format("invalid directive: {0}", directive.to_string()));


		}

		/*
		* object <object_name> <lbrack> <members ...> <rbrack>
		* object is a collection of members + initializers.
		*/
		statement_ptr parse_object_statement(const token_t &t)
		{
			auto name = expect(token_types::identifier_token);

			std::optional<std::string> inherits;
			auto nxt = tokenizer_.peek();
			if (nxt.type == token_types::identifier_token && nxt.to_string() == "inherits")
			{
				tokenizer_.next(); // consume is-a
				nxt = expect(token_types::identifier_token);
				inherits = nxt.to_string();
			}

			expect(token_types::lbrack);
			change_scope sp(*this, scopes::object_);
			auto members = parse_object_members();
			expect(token_types::rbrack);

			return std::make_unique<object_definition_statement>(name.to_string(),inherits,std::move(members));
		}

		std::vector<object_definition::member> parse_object_members()
		{
			// member variable or initializer statement.
			std::vector<object_definition::member> members;
			while (tokenizer_.peek().type != token_types::rbrack)
			{
				if (tokenizer_.peek().type == token_types::eof)
					return std::vector<object_definition::member>{};

				auto tkn = tokenizer_.next();
				if (tkn.type != token_types::identifier_token)
					throw parse_error(tokenizer_.pos(), "expected an identifier");

				auto nxt = tokenizer_.peek();
				if (nxt.type == token_types::op && to_op_type(nxt.begin, nxt.end) == op_type::eq)
				{
					tokenizer_.next();
					auto init = parse_expression();
					members.emplace_back(tkn.to_string(),std::move(init));
				}
				else
				{
					// no initializer.
					members.emplace_back(tkn.to_string());
				}
				close_statement();
			}
			return members;
		}

		statement_ptr parse_def_statement(const token_t &t)
		{
			if (scope_ != scopes::main)
				throw parse_error(tokenizer_.pos(), "cannot define a function here");

			auto identifier = tokenizer_.next();
			if (identifier.type != token_types::identifier_token)
			{
				throw parse_error(tokenizer_.pos(), "expected an identifier");
			}
			expect(token_types::lparen);
			auto id_list = parse_argument_list();
			expect(token_types::rparen);
			change_scope sp(*this, scopes::function);
			auto block = parse_block_statement();

			if (block == nullptr) return nullptr;

			return std::make_unique<def_statement>(identifier.to_string(), std::move(id_list), std::move(block));
		}

		std::vector<argument> parse_argument_list()
		{
			std::vector<argument> list;
			while (tokenizer_.peek().type != token_types::rparen)
			{
				if (tokenizer_.peek().type == token_types::eof)
					return std::vector<argument>();

				auto tkn = tokenizer_.next();
				if (tkn.type != token_types::identifier_token)
					throw parse_error(tokenizer_.pos(), "expected an identifier");

				auto pk = tokenizer_.peek();

				keywords kw{ keywords::unknown_keyword };
				if (pk.type == token_types::identifier_token && is_keyword(pk, kw) && kw == keywords::keyword_is)
				{
					tokenizer_.next();
					auto type = tokenizer_.next();
					if (type.type != token_types::identifier_token)
						throw parse_error(tokenizer_.pos(), "expected a type");

					list.emplace_back(tkn.to_string(), type.to_string());
					pk = tokenizer_.peek();
				}
				else
				{
					list.emplace_back(tkn.to_string());
				}


				if (pk.type == token_types::rparen)
					break;

				if (pk.type != token_types::comma)
				{
					throw parse_error(tokenizer_.pos(), detail::format("argument list, expected a comma. ({0})", pk.to_string()));
				}
				tokenizer_.next();
			}

			return list;
		}

		std::vector<identifier> parse_identifier_list()
		{			
			std::vector<identifier> list;
			while (tokenizer_.peek().type != token_types::rparen)
			{
				if (tokenizer_.peek().type == token_types::eof)
					return std::vector<identifier>();

				auto tkn = tokenizer_.next();
				if (tkn.type != token_types::identifier_token)
					throw parse_error(tokenizer_.pos(), "expected an identifier");
				list.emplace_back(identifier{ tkn.to_string() });

				auto pk = tokenizer_.peek();
				if (pk.type == token_types::rparen)
					break;

				if (pk.type != token_types::comma)
				{
					throw parse_error(tokenizer_.pos(), detail::format("identifier list, expected a comma. ({0})", pk.to_string()));
				}
				tokenizer_.next();
			}

			return list;
		}

		std::vector<expression_ptr> parse_expression_list(token_types right)
		{
			std::vector<expression_ptr> list;
			while (tokenizer_.peek().type != right)
			{
				if (tokenizer_.peek().type == token_types::eof)
					return list;

				auto exp = parse_expression();
				if(exp)
					list.emplace_back(std::move(exp));

				auto pk = tokenizer_.peek();
				if (pk.type == right)
					break;

				if (pk.type != token_types::comma)
				{
					throw parse_error(tokenizer_.pos(), detail::format("expression list, expected a comma. ({0})", pk.to_string()));
				}
				tokenizer_.next();
			}
			return list;
		}

		expression_ptr parse_keyword_expression(keywords kw)
		{
			switch (kw)
			{
			case keywords::new_keyword:
			{
				if (tokenizer_.peek().type == token_types::sqlbrack)
					return parse_new_array_expression();
				else if (tokenizer_.peek().type == token_types::identifier_token)
					return parse_new_object_expression();
				else
					return parse_new_blob_expression();
			}
			default:
				throw parse_error(tokenizer_.pos(), "you can't do that here.");
			}
		}

		expression_ptr parse_new_blob_expression()
		{			
			auto initializers = parse_initializer_list();			
			return std::make_unique<new_blob_expression>(std::move(initializers));
		}

		expression_ptr parse_new_array_expression()
		{
			expect(token_types::sqlbrack);
			auto expr = parse_expression_list(token_types::sqrbrack);
			expect(token_types::sqrbrack);
			return std::make_unique<new_array_expression>(std::move(expr));
		}

		expression_ptr parse_new_object_expression()
		{
			auto id = tokenizer_.next();
			auto initializers = parse_initializer_list();
			return std::make_unique<new_object_expression>(id.to_string(), std::move(initializers));
		}

		initializer_list_t parse_initializer_list()
		{
			initializer_list_t list;
			expect(token_types::lbrack);
			while (tokenizer_.peek().type != token_types::rbrack)
			{
				if (tokenizer_.peek().type == token_types::eof)
					return list;

				auto id = tokenizer_.next();
				if (id.type != token_types::identifier_token)
					throw parse_error(tokenizer_.pos(), "expected an identifier");

				auto eq = tokenizer_.next();
				if (eq.type != token_types::op || !builtins::compare(eq.begin, eq.end, "="))
					throw parse_error(tokenizer_.pos(), "expected '='");

				auto expr = parse_expression();
				if (expr == nullptr)
					throw parse_error(tokenizer_.pos(), "expected an expression");

				list.emplace_back(id.to_string(), std::move(expr));

				auto pk = tokenizer_.peek();
				if (pk.type == token_types::rbrack)
					break;
				if (pk.type != token_types::comma)
				{
					throw parse_error(tokenizer_.pos(), "expected a comma");
				}
				tokenizer_.next();
			}
			expect(token_types::rbrack);
			return list;
		}

		void close_statement()
		{
			if (tokenizer_.next().type != token_types::eos)
				throw parse_error(tokenizer_.pos(), "expected a ';'");
		}

		template <typename IteratorT>
		keywords to_keyword(IteratorT begin, IteratorT end)
		{
			if (builtins::compare(begin, end, "if"))
				return keywords::if_keyword;
			else if (builtins::compare(begin, end, "else"))
				return keywords::else_keyword;
			else if (builtins::compare(begin, end, "is"))
				return keywords::keyword_is;
			else if (builtins::compare(begin, end, "def"))
				return keywords::def_keyword;
			else if (builtins::compare(begin, end, "new"))
				return keywords::new_keyword;
			else if (builtins::compare(begin, end, "let"))
				return keywords::let_keyword;
			else if (builtins::compare(begin, end, "while"))
				return keywords::while_keyword;
			else if (builtins::compare(begin, end, "for"))
				return keywords::for_keyword;
			else if (builtins::compare(begin, end, "return"))
				return keywords::return_keyword;
			else if (builtins::compare(begin, end, "object"))
				return keywords::object_keyword;
			return keywords::unknown_keyword;
		}

		keywords to_keyword(token_t &t)
		{
			return to_keyword(t.begin, t.end);
		}

		bool is_keyword(token_t &tkn, keywords &kw_out)
		{
			kw_out = to_keyword(tkn);
			return kw_out != keywords::unknown_keyword;
		}

		op_type to_op(token_t &t)
		{
			return to_op_type(t.begin, t.end);
		}
		

	private:
		token_t expect(token_types ttype)
		{
			auto tkn = tokenizer_.next();
			if (tkn.type != ttype)
				throw parse_error(tokenizer_.pos(), "expected a token");
			return tkn;
		}

		bool maybe(token_types ttype, token_t &tkn)
		{
			auto peek = tokenizer_.peek();
			if (peek.type == ttype)
			{
				tkn = tokenizer_.next();
				return true;
			}
			return false;
		}

		bool maybe(token_types ttype)
		{
			token_t tkn;
			return maybe(ttype, tkn);
		}

	private:
		tokenizer_t tokenizer_;
		scopes scope_;
	};

	inline syntax_tree parse(const std::string &s)
	{
		parser p(s);
		syntax_tree ast;
		while (1)
		{
			auto stmt = p.next();
			if (!stmt) break;
			ast.push_back(std::move(stmt));
		}
		return ast;
	}
	
}

#endif // __simpl_parser_h__