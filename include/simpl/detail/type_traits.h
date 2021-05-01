#ifndef __simpl_type_traits_h__
#define __simpl_type_traits_h__

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
            return std::string(typeid(T).name()) + ", " + to_string<Ts...>::value();
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
}
}

#endif // __simple_type_traits_h__