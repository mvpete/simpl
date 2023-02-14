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
    using number = double;
	struct empty_t {};
	struct blob_t;
	struct array_t;

	using blobref_t = std::shared_ptr<blob_t>;
	inline blobref_t new_blob()
	{
		return std::make_shared<blob_t>();
	}

	using arrayref_t = std::shared_ptr<array_t>;

	using value_t = std::variant<empty_t, bool, double, std::string, blobref_t, arrayref_t, objectref_t>;
	struct blob_t { std::map<std::string, value_t> values; };
	struct array_t 
    {
        array_t() = default;
        array_t(std::vector<value_t> &&v)
            :values(std::move(v))
        {
        }
        std::vector<value_t> values; 
    };

    struct simpl_object_t : simpl::object
    {
        simpl_object_t(const std::string &type)
            :type_id(type)
        {
        }

        virtual std::string type() const
        {
            return type_id;
        }

        virtual bool is_convertible(const std::string &t)
        {
            return false;
        }

        void *value()
        {
            return nullptr;
        }

        const std::string type_id;
        std::map<std::string, value_t> members;
    };

    using instanceref_t = std::shared_ptr<simpl_object_t>;

	inline arrayref_t new_array()
	{
		return std::make_shared<array_t>();
	}

    inline arrayref_t make_array(std::vector<value_t> &&v)
    {
        return std::make_shared<array_t>(std::move(v));
    }

    inline instanceref_t new_simpl_object(const std::string &type)
    {
        return std::make_shared<simpl_object_t>(type);
    }

namespace detail
{

    template<>
    struct is_valid_arg_type<bool> : std::true_type {};

    template<>
    struct is_valid_arg_type<double> : std::true_type {};

    template<>
    struct is_valid_arg_type<std::string> : std::true_type {};

    template<>
    struct is_valid_arg_type<array_t> : std::true_type {};

    template<>
    struct is_valid_arg_type<blob_t> : std::true_type {};

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
    inline std::string get_type_string(const value_t &v)
    {
        if (std::holds_alternative<empty_t>(v))
            return "empty";
        else if (std::holds_alternative<bool>(v))
            return "bool";
        else if (std::holds_alternative<double>(v))
            return "number";
        else if (std::holds_alternative<std::string>(v))
            return "string";
        else if (std::holds_alternative<blobref_t>(v))
            return "blob";
        else if (std::holds_alternative<arrayref_t>(v))
            return "array";
        else if (std::holds_alternative<objectref_t>(v))
        {
            return std::get<objectref_t>(v)->type();
        }

        throw std::runtime_error("unknown type");
    }

    inline std::optional<std::string> to_builtin_type_string(const std::string &simpl_type)
    {
        return simpl_type;
    }

    template <typename T>
    typename std::enable_if<is_one_of<T, empty_t, bool, double, std::string, objectref_t>::value, T>::type& get_value(value_t &v)
    {
        return std::get<T>(v);
    }

    template<typename T>
    typename std::enable_if<std::is_same_v<T,value_t>, value_t>::type& get_value(value_t &v)
    {
        return v;
    }

    template<typename T>
    typename std::enable_if<std::is_same_v<T, blob_t>, blob_t>::type &get_value(value_t &v)
    {
        auto &blobref = std::get<blobref_t>(v);
        return *blobref;
    }

    template<typename T>
    typename std::enable_if<std::is_same_v<T, array_t>, array_t>::type &get_value(value_t &v)
    {
        auto &arrayref = std::get<arrayref_t>(v);
        return *arrayref;
    }

    template <typename T>
    typename std::enable_if<!is_one_of<T, empty_t, bool, double, std::string, blob_t, array_t, objectref_t, value_t>::value, T>::type& get_value(value_t &v)
    {
        if (!std::holds_alternative<objectref_t>(v))
            throw std::runtime_error("not an object");

        auto objref = std::get<objectref_t>(v);
        if(detail::simple_type_info<T>::name() == objref->type() || objref->is_convertible(detail::simple_type_info<T>::name()))
            return (*reinterpret_cast<T*>(objref->value()));

        throw std::runtime_error("bad type conversion");
    }

}
    struct member_visitor
    {
    public:
        member_visitor(const std::string &member)
            :member(member), value(nullptr)
        {
        }
        // empty_t, bool, double, std::string, blobref_t, arrayref_t, objectref_t
        void operator()(empty_t &)
        {
            throw std::runtime_error("invalid access");
        }
        void operator()(bool &)
        {
            throw std::runtime_error("invalid access");
        }
        void operator()(double &)
        {
            throw std::runtime_error("invalid access");
        }
        void operator()(std::string &)
        {
            throw std::runtime_error("invalid access");
        }
        void operator()(arrayref_t &)
        {
            throw std::runtime_error("invalid access");
        }
        void operator()(blobref_t &blob)
        {
            value = &blob->values.at(member);
        }
        void operator()(objectref_t &obj)
        {
            auto inst = std::dynamic_pointer_cast<simpl_object_t>(obj);
            if(inst == nullptr)
                throw std::runtime_error("invalid access on type");
            value = &inst->members.at(member);
        }
        const std::string member;
        value_t *value;
    };


}

#endif //__simpl_value_h__