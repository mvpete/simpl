#ifndef __simpl_detail_signature_h__
#define __simpl_detail_signature_h__

#include <simpl/value.h>

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
            : std::conditional_t<is_one_of<typename std::decay<T>::type, bool, double, std::string, blob_t, array_t, value_t>::value, are_valid_types<Ts...>, std::false_type> {};

        template<typename T>
        struct signature : signature<decltype(&T::operator())> {};

        template <typename ...>
        struct to_string
        {
            static std::string value()
            {
                return std::string();
            }
        };

        template <typename T, typename ...Ts>
        struct to_string<T, Ts...>
        {
            static std::string value()
            {
                return std::string(typeid(T).name()) + ", " + to_string<Ts...>::value();
            }
        };


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

            std::string arguments_string() const
            {
                return to_string<Args...>::value();
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