#ifndef __simpl_type_traits_h__
#define __simpl_type_traits_h__

#include <optional>

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


    // TODO: Use variadics for this, and just used the value_t
    std::string get_type_string(const value_t &v)
    {
        if (std::holds_alternative<empty_t>(v))
            return typeid(empty_t{}).name();
        else if (std::holds_alternative<bool>(v))
            return typeid(bool{}).name();
        else if (std::holds_alternative<double>(v))
            return typeid(double{}).name();
        else if (std::holds_alternative<std::string>(v))
            return typeid(std::string{}).name();
        else if (std::holds_alternative<blobref_t>(v))
            return typeid(blob_t{}).name();
        else if (std::holds_alternative<arrayref_t>(v))
            return typeid(array_t{}).name();
        throw std::runtime_error("unknown type");
    }

    std::optional<std::string> to_type_string(const std::string &simpl_type)
    {
        if (simpl_type == "string")
        {
            return typeid(std::string).name();
        }
        else if (simpl_type == "bool")
        {
            return typeid(bool).name();
        }
        else if (simpl_type == "number")
        {
            return typeid(double).name();
        }
        else if (simpl_type == "blob")
        {
            return typeid(blob_t).name();
        }
        else if (simpl_type == "array")
        {
            return typeid(array_t).name();
        }
        return std::optional<std::string>{};        
    }
}
}

#endif // __simple_type_traits_h__