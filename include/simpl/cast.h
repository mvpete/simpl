#ifndef __simpl_cast_h__
#define __simpl_cast_h__

#include <stdexcept>
#include <simpl/value.h>

namespace simpl
{
    class invalid_cast : public std::runtime_error
    {
    public:
        invalid_cast(const char *msg)
            :std::runtime_error(msg)
        {
        }
        invalid_cast()
            :std::runtime_error("invalid cast")
        {
        }
    };


#define CAST(To, From) \
template<>\
To to(const From &from)\

#define INVALID_CAST(To, From) \
template<>\
To to(const From &f)\
{\
    throw invalid_cast(); \
}\


    template <typename To, typename From>
    To to(const From &value)
    {
        static_assert(false, "you shall not pass; in other words, you have a bad typecast");
    }

    // specialize casts here. to<to,from>
    template <>
    std::string to<std::string, double>(const double &value)
    {
        std::stringstream ss;
        ss << value;
        return ss.str();
    }

    template <>
    std::string to<std::string, std::string>(const std::string &v)
    {
        return v;
    }

    template <>
    double to<double, std::string>(const std::string &v)
    {
        // for now.
        double rv = -1;
        std::stringstream ss;
        ss << v;
        ss >> rv;
        return rv;
    }

    template <>
    double to<double, double>(const double &v)
    {
        return v;
    }

    template <>
    bool to<bool, double>(const double &v)
    {
        return v > 0;
    }

    template <>
    bool to<bool, std::string>(const std::string &strval)
    {
        return strval == "true" || strval == "1" || strval == "t" || strval == "TRUE" || strval == "T";
    }

    template <>
    size_t to<size_t, std::string>(const std::string &strval)
    {
        std::stringstream ss;
        ss << strval;
        size_t val(0);
        ss >> val;
        return val;
    }

    CAST(bool, blobref_t)
    {
        return from != nullptr;
    }
    
    CAST(blobref_t, blobref_t)
    {
        return from;
    }

    INVALID_CAST(double, blobref_t);
    INVALID_CAST(empty_t, blobref_t);
    INVALID_CAST(arrayref_t, blobref_t);

    CAST(bool, arrayref_t)
    {
        return from != nullptr;
    }

    CAST(arrayref_t, arrayref_t)
    {
        return from;
    }

    INVALID_CAST(double, arrayref_t);
    INVALID_CAST(empty_t, arrayref_t);
    INVALID_CAST(blobref_t, arrayref_t);

    template <typename T>
    struct cast_
    {
        T value;
        void operator()(const empty_t &v)
        {
            throw invalid_cast("value is undefined; did you forget to return?");
        }

        void operator()(const blobref_t &v)
        {
            value = to<T>(v);
        }

        void operator()(const arrayref_t &v)
        {
           value = to<T>(v);
        }

        void operator()(double lv)
        {
            value = to<T>(lv);
        }

        void operator()(const std::string &lv)
        {
            value = to<T>(lv);
        }

        void operator()(const objectref_t &lv)
        {
            throw invalid_cast("cannot cast objectref_t");
        }

    };    

    template <typename T>
    const T cast(const value_t &value)
    {
        cast_<T> ct;
        std::visit(ct, value);
        return ct.value;
    }
        
    CAST(std::string, blobref_t)
    {
        std::stringstream ss;
        size_t count = 0;
        size_t size = from->values.size();
        ss << "{ ";
        for (const auto i : from->values)
        {
            ss << i.first << " : " << cast<std::string>(i.second);
            ++count;
            if (count != size)
                ss << ", ";
        }
        ss << " }";
        return ss.str();
    }

    CAST(std::string, arrayref_t)
    {
        if (from == nullptr)
            return "null";
        std::stringstream ss;
        size_t count = 0;
        size_t size = from->values.size();
        ss << "[ ";
        for (const auto &i : from->values)
        {
            ss << cast<std::string>(i);
            ++count;
            if (count != size)
                ss << ", ";
        }
        ss << " ]";
        return ss.str();
    }

}


#endif // __simpl_cast_h__