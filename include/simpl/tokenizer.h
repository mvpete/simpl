#ifndef __simple_tokenizer_h__
#define __simple_tokenizer_h__

#include <simpl/op.h>
#include <simpl/detail/format.h>

#include <stdexcept>
#include <sstream>

namespace simpl
{
	inline bool is_eol(char c)
	{
		return c == '\r' || c == '\n';
	}

	struct position
	{
		size_t line;
		size_t col;

		position(size_t line, size_t col)
			:line(line), col(col)
		{
		}
	};

	inline std::ostream &operator<<(std::ostream &os, const position &p)
	{
		os << p.line << ":" << p.col;
		return os;
	}

	class token_error : public std::exception
	{
	public:
		token_error(const position &pos, const char *message)
			:std::exception(detail::format("token error: {0} ({1})", message, pos).c_str()), pos_(pos)
		{
		}

		position pos() const
		{
			return pos_;
		}
	private:
		position pos_;
	};

	enum class token_types
	{
		identifier_token,
		literal,
		number,
		lparen,
		rparen,
		sqlbrack,
		sqrbrack,
		lbrack,
		rbrack,
		comma,
		op,
		eof,
		eos,
		comment,
		empty_token,
		directive
	};

	template<typename CharT>
	struct token
	{
		token_types type;
		const CharT *begin;
		const CharT *end;
		position pos;

		token() :type(token_types::empty_token),begin(nullptr),end(nullptr), pos(0,0) {}
		token(token_types type, const CharT *begin, const CharT *end, const position &p) 
			: type(type), begin(begin), end(end), pos(p) {}

		std::string to_string() const
		{ 
			return std::string(begin, end);
		}
	};

	template <typename CharT>
	class basic_tokenizer
	{
	public:
		using token_t = token<CharT>;

	public:

		basic_tokenizer(const std::basic_string<CharT> &text)
			:begin_(&text[0]), end_(&text[text.length()]), cur_(begin_), position_(1, 1)
		{
		}

		template<typename IteratorT>
		basic_tokenizer(IteratorT begin,  IteratorT end)
			:begin_(begin), end_(end), cur_(begin),position_(1,1)
		{
		}

		token_t peek()
		{
			if (next_.type != token_types::empty_token)
				return next_;

			auto start = cur_;
			for (; cur_ < end_; ++cur_, ++position_.col)
			{
				char c = *cur_;
				if (isspace(c))
				{
					if (is_eol(c) && scan_eol())
					{
						++position_.line;
						position_.col = 1;
					}
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
				else if (c == ';')
				{
					next_ = token_t(token_types::eos, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == '(')
				{
					next_ = token_t(token_types::lparen, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == ')')
				{
					next_ = token_t(token_types::rparen, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == '{')
				{
					next_ = token_t(token_types::lbrack, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == '}')
				{
					next_ = token_t(token_types::rbrack, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == '[')
				{
					next_ = token_t(token_types::sqlbrack, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == ']')
				{
					next_ = token_t(token_types::sqrbrack, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == ',')
				{
					next_ = token_t(token_types::comma, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else if (c == '@')
				{
					next_ = token_t(token_types::directive, start, cur_++, position_);
					++position_.col;
					return next_;
				}
				else
				{
					throw token_error(position_, detail::format("invalid token '{0}'", c).c_str());
				}
			}
			next_= token_t(token_types::eof, cur_, end_, position_);
			return next_;
		}

		token_t next()
		{
			if (next_.type == token_types::empty_token)
				peek();
			auto tkn = next_;
			next_.type = token_types::empty_token;
			return tkn;
		}

		void reverse(token_t &t)
		{
			cur_ = t.begin;
			next_.type = token_types::empty_token;
		}

		const position &pos() const
		{
			return position_;
		}

	private:
		bool scan_identifier(token_t &t)
		{
			auto start = cur_;
			for (;cur_ != end_; ++cur_)
			{
				char c = *cur_;
				if (!isalnum(c) && c != '_')
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
			for (++cur_; cur_ != end_; ++cur_)
			{
				if (is_op_maybe(start, cur_))
					continue;
				// If it's not an op, we went one too far ahead so we need to backtrack.
				if (!is_op(start, cur_))
					--cur_;
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
				throw token_error(position_,"missing closing quote");

			t.type = token_types::literal;
			t.begin = start;
			t.end = cur_++;

			return true;
		}

		bool scan_eol()
		{
			if (*cur_ == '\r' && cur_ + 1 != end_ && *(cur_ + 1) == '\n')
			{
				cur_++;
				return true;
			}
			else if(*cur_ == '\r' || *cur_ == '\n')
			{
				return true;
			}
			return false;
		}

	private:
		const CharT *begin_, *end_, *cur_;
		position position_;
		token_t next_;

	};


	using tokenizer = basic_tokenizer<char>;
	using wtokenizer = basic_tokenizer<wchar_t>;
}


#endif // __simple_tokenizer_h__