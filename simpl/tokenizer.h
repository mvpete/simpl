#ifndef __simple_tokenizer_h__
#define __simple_tokenizer_h__

#include "op.h"

#include <stdexcept>
#include <sstream>

namespace simpl
{


	bool is_eol(char c)
	{
		return c == '\r' || c == '\n';
	}


	class token_error : public std::exception
	{
	public:
		token_error(int line, const char *message)
			:std::exception(message), line_(line)
		{
		}

		int line() const
		{
			return line_;
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
		lbrack,
		rbrack,
		comma,
		op,
		eof,
		eos,
		comment,
		empty_token
	};

	template<typename CharT>
	struct token
	{
		const CharT *begin;
		const CharT *end;
		token_types type;
		token() :type(token_types::empty_token),begin(nullptr),end(nullptr){}
		token(token_types type, const CharT *begin, const CharT *end) : type(type), begin(begin), end(end) {}

		std::string to_string() const
		{ 
			return std::string(begin, end);
		}
	};

	template <typename CharT>
	class tokenizer
	{
	public:
		using token_t = token<CharT>;

	public:
		template<typename IteratorT>
		tokenizer(IteratorT begin,  IteratorT end)
			:begin_(begin), end_(end), cur_(begin), line_(0)
		{
		}

		token_t peek()
		{
			if (next_.type != empty_token)
				return next_;

			auto start = cur_;
			for (; cur_ != end_; ++cur_)
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
					continue;
				}
				else if (c == '\"' && scan_literal(next_))
				{
					return next_;
				}
				else if (is_eol(c) && scan_eol())
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
				else if (c == '{')
				{
					next_ = token_t(token_types::lbrack, start, cur_++);
					return next_;
				}
				else if (c == '}')
				{
					next_ = token_t(token_types::rbrack, start, cur_++);
					return next_;
				}
				else if (c == ',')
				{
					next_ = token_t(token_types::comma, start, cur_++);
					return next_;
				}
				else
				{
					std::stringstream ss;
					ss << "err: invalid token '" << c << "'";

					throw token_error(line_, ss.str().c_str());
				}
			}
			next_= token_t(token_types::eof, cur_, end_);
			return next_;
		}

		token_t next()
		{
			if (next_.type == empty_token)
				peek();
			auto tkn = next_;
			next_.type = empty_token;
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
			auto start = cur_;
			for (; cur_ != end_; ++cur_)
			{
				char c = *cur_;
				if (!is_op(c))
					break;
			}

			t.type = token_types::op;
			t.begin = start;
			t.end = cur_;
			return true;
		}

		bool scan_comment(token_t &t)
		{
			while (!is_eol(*cur_) && cur_ != end_) ++cur_;
			return true;
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

			if (cur_ == end_ || *cur_ != '\"')
				throw token_error(line_,"missing closing quote");

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
		const CharT *begin_, *end_, *cur_;
		int line_;
		token_t next_;

	};

}


#endif // __simple_tokenizer_h__