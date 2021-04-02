#ifndef __simple_tokenizer_h__
#define __simple_tokenizer_h__

#include <stdexcept>

namespace simpl
{

	bool is_op(char c)
	{
		return c == '+' || c == '-' || c == '*' || c == '/';
	}


	class token_error : public std::exception
	{
	public:
		token_error(int line, const char *message)
			:std::exception(message), line_(line)
		{
		}
	private:
		int line_;
	};

	enum token_types
	{
		identifier_token,
		literal,
		number,
		lparen,
		rparen,
		comma,
		op,
		eof,
		eos,
		comment,
		empty_token
	};

	template<typename IteratorT>
	struct token
	{
		IteratorT begin;
		IteratorT end;
		token_types type;
		token() :type(token_types::empty_token){}
		token(token_types type, IteratorT begin, IteratorT end) : type(type), begin(begin), end(end) {}

		std::string to_string() const
		{ 
			return std::string(begin, end);
		}
	};

	template <typename IteratorT>
	class tokenizer
	{
	public:
		using token_t = token<IteratorT>;

	public:
		tokenizer(IteratorT begin, IteratorT end)
			:begin_(begin), end_(end), cur_(begin), line_(0)
		{
		}

		token_t peek()
		{
			auto start = cur_;
			for (;cur_ != end_; ++cur_)
			{
				char c = *cur_;
				if (isspace(c))
				{
					continue;
				}
				else if (isalpha(c) && scan_identifier(next_))
				{
					return next_;
				}
				else if (isdigit(c) && scan_number(next_))
				{
					return next_;
				}
				else if (is_op(c) && scan_op(next_))
				{
					return next_;
				}
				else if (c == '#' && scan_comment(next_))
				{
					return next_;
				}
				else if (c == '\"' && scan_literal(next_))
				{
					return next_;
				}
				else if (c == '\r' && scan_eol())
				{
					++line_;
					continue;
				}
				else if (c == ';')
				{
					next_ = token_t(token_types::eos, start, cur_++);
					return next_;
				}
				else if (c == '(')
				{
					next_ = token_t(token_types::lparen, start, cur_++);
					return next_;
				}
				else if (c == ')')
				{
					next_ = token_t(token_types::rparen, start, cur_++);
					return next_;
				}
				else if (c == ',')
				{
					next_ = token_t(token_types::comma, start, cur_++);
					return next_;
				}
				else
					throw token_error(line_, "invalid token");
			}
			next_= token_t(token_types::eof, cur_, end_);
			return next_;
		}

		token_t next()
		{
			if (next_.type == empty_token)
				peek();
			auto tkn = next_;
			next_.type == empty_token;
			return tkn;
		}


	private:
		bool scan_identifier(token_t &t)
		{
			auto start = cur_;
			for (;cur_ != end_; ++cur_)
			{
				char c = *cur_;
				if (!isalnum(c))
					break;
			}

			t.type = token_types::identifier_token;
			t.begin = start;
			t.end = cur_;

			return true;
		}

		bool scan_number(token_t &t)
		{
			auto start = cur_;
			for (;cur_ != end_; ++cur_)
			{
				char c = *cur_;
				if (!isdigit(c))
					break;
			}
			t.type = token_types::number;
			t.begin = start;
			t.end = cur_;

			return true;
		}

		bool scan_op(token_t &t)
		{
			t.type = token_types::op;
			t.begin = cur_;
			t.end = ++cur_;
			return true;
		}

		bool scan_comment(token_t &t)
		{
			return false;
		}

		bool scan_literal(token_t &t)
		{
			++cur_;
			auto start = cur_;
			for (;cur_ != end_; ++cur_)
			{
				char c = *cur_;
				if (c == '\"')
					break;
			}

			t.type = token_types::literal;
			t.begin = start;
			t.end = cur_++;

			return true;
		}

		bool scan_eol()
		{
			return false;
		}




	private:
		IteratorT begin_, end_, cur_;
		int line_;
		token_t next_;

	};

}


#endif // __simple_tokenizer_h__