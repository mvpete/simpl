#ifndef __simpl_type_traits_h__
#define __simpl_type_traits_h__

#include <optional>
#

namespace simpl
{
namespace detail
{
    template <typename ...>
    struct to_string
    {
        static std::string value()
        {
            return "";
        }
    };

    template <typename T, typename ...Ts>
    struct to_string<T, Ts...>
    {
        static std::string value()
        {
            return std::string(typeid(T).name()) + "," + to_string<Ts...>::value();
        }
    };

    template <typename T>
    struct to_string<T>
    {
        static std::string value()
        {
            return std::string(typeid(T).name());
        }
    };

    template <typename ...>
    struct to_vector
    {
        static std::vector<std::string> &value()
        {
            static std::vector<std::string> values;
            return values;

        }
    };

    template <typename T, typename ...Ts>
    struct to_vector<T, Ts...>
    {
        static std::string value()
        {
            return to_vector<Ts...>::value().push_back(std::string(typeid(T).name()));
        }
    };
           

    template <typename...>
    struct is_one_of : std::false_type {};

    template <typename F, typename S, typename... T>
    struct is_one_of<F, S, T...> : std::conditional_t<std::is_same_v<F, S>, std::true_type, is_one_of<F, T...>> {};


    template<typename T>
    struct is_valid_arg_type : std::false_type {};


    template<typename T>
    struct is_valid_return_type : std::false_type {};

    template <typename ...>
    struct are_valid_arg_types : std::true_type {};

    template<typename T, typename ...Ts>
    struct are_valid_arg_types<T, Ts...>
        : std::conditional_t<is_valid_arg_type<typename std::decay<T>::type>::value, are_valid_arg_types<Ts...>, std::false_type> {};


    template <typename ...>
    struct are_valid_return_types : std::true_type {};

    template<typename T, typename ...Ts>
    struct are_valid_return_types<T, Ts...>
        : std::conditional_t<is_valid_return_type<typename std::decay<T>::type>::value, are_valid_return_types<Ts...>, std::false_type> {};

    template<typename ...>
    struct decay_tuple
    {
        using type = std::tuple<>;
    };

    template <typename T, typename ...Args>
    struct decay_tuple<T, Args...>
    {
        using type = std::tuple<typename std::decay_t<T>, typename std::decay_t<Args>...>;
    };

}
}

#endif // __simple_type_traits_h__