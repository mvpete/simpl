#ifndef __simpl_detail_signature_h__
#define __simpl_detail_signature_h__

#include <simpl/value.h>
#include <simpl/detail/format.h>
#include <simpl/detail/type_traits.h>

namespace simpl
{
    namespace detail
    {
        template <typename...>
        struct is_one_of : std::false_type {};

        template <typename F, typename S, typename... T>
        struct is_one_of<F, S, T...> : std::conditional_t<std::is_same_v<F, S>, std::true_type, is_one_of<F, T...>> {};
        
        template <typename ...>
        struct are_valid_types : std::true_type {};

        template<typename T, typename ...Ts>
        struct are_valid_types<T,Ts...>
            : std::conditional_t<is_one_of<typename std::decay<T>::type, bool, double, std::string, blob_t, array_t, value_t, void>::value, are_valid_types<Ts...>, std::false_type> {};

        template<typename ...>
        struct decay_tuple 
        {
            using type = std::tuple<>;
        };

        template <typename T, typename ...Args>
        struct decay_tuple<T, Args...> 
        {
            using type = std::tuple<typename std::decay_t<T>, Args...>;
        };

        template<typename T>
        struct signature : signature<decltype(&T::operator())> {};

        template <typename R, typename ...Args>
        struct signature_impl
        {
            static_assert(are_valid_types<Args...>::value, "callable has an invalid argument");
            static_assert(are_valid_types<R>::value, "invalid return type");

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