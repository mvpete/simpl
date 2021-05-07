#ifndef __simpl_detail_signature_h__
#define __simpl_detail_signature_h__

#include <simpl/value.h>
#include <simpl/detail/format.h>

namespace simpl
{      
    namespace detail
    {
        template<typename T>
        struct signature : signature<decltype(&T::operator())> {};

        template <typename R, typename ...Args>
        struct signature_impl
        {
            static_assert(are_valid_arg_types<Args...>::value, "cannot register function - invalid argument type");
            static_assert(are_valid_return_types<R>::value, "cannot register function - invalid return type");

            static constexpr size_t arity = sizeof...(Args);
            using result_type = R;

            template <size_t i>
            struct arg
            {
                typedef typename std::tuple_element<i, std::tuple<Args...>>::type type;
            };

            using types = typename decay_tuple<Args...>::type;

            std::string arguments_string() const
            {
                return to_string<Args...>::value();
            }

            std::vector<std::string> arguments()
            {
                return to_vector<Args...>::value();
            }

        };

        // Free functions
        template<typename R, typename ...Args>
        struct signature<R(*)(Args...)> 
            : signature_impl<R,Args...>{};

        // Pointer to member
        template <typename R, typename C, typename... Args>
        struct signature<R(C:: *)(Args...)> 
            : signature_impl<R, Args...> {};

        // Pointer to const member
        template <typename R, typename C, typename... Args>
        struct signature<R(C:: *)(Args...) const> 
            : signature_impl<R, Args...> {};

        template <typename Fn>
        constexpr auto get_signature()->signature<Fn>
        {
            return signature<Fn>{};
        }


    }
}

#endif // __simpl_detail_signature_h__