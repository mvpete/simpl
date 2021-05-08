#ifndef __simpl_value_h__
#define __simpl_value_h__

#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <variant>
#include <vector>

#include <simpl/detail/type_traits.h>
#include <simpl/object.h>

namespace simpl
{
	struct empty_t {};
	struct blob_t;
	struct array_t;
	
	using blobref_t = std::shared_ptr<blob_t>;
	blobref_t new_blob()
	{
		return std::make_shared<blob_t>();
	}

	using arrayref_t = std::shared_ptr<array_t>;
	arrayref_t new_array()
	{
		return std::make_shared<array_t>();
	}

	using value_t = std::variant<empty_t, bool, double, std::string, blobref_t, arrayref_t, objectref_t>;
	struct blob_t { std::map<std::string, value_t> values; };
	struct array_t { std::vector<value_t> values; };

namespace detail
{

    template<>
    struct is_valid_arg_type<bool> : std::true_type {};

    template<>
    struct is_valid_arg_type<double> : std::true_type {};

    template<>
    struct is_valid_arg_type<std::string> : std::true_type {};

    template<>
    struct is_valid_arg_type<arrayref_t> : std::true_type {};

    template<>
    struct is_valid_arg_type<blobref_t> : std::true_type {};

    template<>
    struct is_valid_arg_type<value_t> : std::true_type {};

    template<>
    struct is_valid_arg_type<void> : std::true_type {};


    template<>
    struct is_valid_return_type<bool> : std::true_type {};

    template<>
    struct is_valid_return_type<double> : std::true_type {};

    template<>
    struct is_valid_return_type<std::string> : std::true_type {};

    template<>
    struct is_valid_return_type<arrayref_t> : std::true_type {};

    template<>
    struct is_valid_return_type<blobref_t> : std::true_type {};

    template<>
    struct is_valid_return_type<value_t> : std::true_type {};

    template<>
    struct is_valid_return_type<void> : std::true_type {};

    template<>
    struct is_valid_return_type<objectref_t> : std::true_type {};

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
        else if (std::holds_alternative<objectref_t>(v))
        {
            return std::get<objectref_t>(v)->type();
        }

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

    template <typename T>
    struct simple_type_info;

    template<typename T>
    ref_t<T> convert_to(objectref_t &base)
    {
        if (typeid(T).name() != base->type())
            throw std::runtime_error("type mismatch");
        return std::dynamic_pointer_cast<ref<T>>(base);
    }

    template <typename T>
    typename std::enable_if<is_one_of<T, empty_t, bool, double, std::string, blobref_t, arrayref_t, objectref_t>::value, T>::type& get_value(value_t &v)
    {
        return std::get<T>(v);
    }

    template<typename T>
    typename std::enable_if<std::is_same_v<T,value_t>, value_t>::type& get_value(value_t &v)
    {
        return v;
    }

    template <typename T>
    typename std::enable_if<!is_one_of<T, empty_t, bool, double, std::string, blobref_t, arrayref_t, objectref_t, value_t>::value, T>::type& get_value(value_t &v)
    {
        if (!std::holds_alternative<objectref_t>(v))
            throw std::runtime_error("not an object");

        auto objref = std::get<objectref_t>(v);
        return convert_to<T>(objref)->value();
    }


}



}

#endif //__simpl_value_h__