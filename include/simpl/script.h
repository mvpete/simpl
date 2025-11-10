#ifndef __simpl_script_h__
#define __simpl_script_h__

#include <simpl/evaluate.h>
#include <simpl/tokenizer.h>
#include <simpl/parser.h>
#include <simpl/engine.h>
#include <simpl/value.h>

namespace simpl
{
namespace detail
{
	template <typename EngineT>
	class basic_script
	{
		struct tag {};
	public:

		void evaluate(const std::string& str, tag = {})
		{
			auto ast = simpl::parse(str);
			simpl::evaluate(ast, e_);
		}

		template <typename CallableT>
		void register_function(const std::string& name, CallableT&& c)
		{
			e_.machine().reg_fn(name, std::forward<CallableT>(c));
		}

		template<typename T>
		void register_type()
		{
			const auto simpl_name = simple_type_info<T>::name();
			std::stringstream ss;
			ss << "make_" << simpl_name;
			e_.machine().register_type<T>();
			e_.machine().reg_fn(ss.str(), []()
			{
				return simpl::make_ref<T>();
			});
		}

	private:
		EngineT e_;

	};
}

using script = detail::basic_script<simpl::engine>;



#define SIMPL_TYPE_DEF(type, type_name)\
template<>\
struct simpl::detail::is_valid_arg_type<type> : std::true_type {};\
\
\
\
template<>\
struct simpl::detail::is_valid_return_type<type> : std::true_type {};\
\
template<>\
struct simpl::detail::simple_type_info<type>\
{\
	static const char* name() noexcept\
	{\
		return type_name;\
	};\
\
	static bool is_convertible(const std::string& t)\
	{\
		return false;\
	}\
\
	static const char* inherits() noexcept\
	{\
		return nullptr;\
	}\
}

}

#endif // __simpl_script_h__