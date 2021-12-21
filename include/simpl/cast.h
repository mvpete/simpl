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

    template <typename To, typename From>
    To to(const From &value)
    {
        static_assert(false, "you shall not pass; in other words, you have a bad typecast");
    }

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

#ifdef SIMPL_DEFINES
#define CAST(To, From) \
template<>\
To to(const From &from)\


#define INVALID_CAST(To, From) \
template<>\
To to(const From &f)\
{\
    throw invalid_cast(); \
}\

    // specialize casts here. to<to,from>
    CAST(std::string, double)
    {
        std::stringstream ss;
        ss << from;
        return ss.str();
    }

    CAST(std::string, std::string)
    {
        return from;
    }

    CAST(double, std::string)
    {
        // for now.
        double rv = -1;
        std::stringstream ss;
        ss << from;
        ss >> rv;
        return rv;
    }

    CAST(double,double)
    {
        return from;
    }

    CAST(bool, double)
    {
        return from > 0;
    }

    CAST(bool, std::string)
    {
        return from == "true" || from == "1" || from == "t" || from == "TRUE" || from == "T";
    }

    CAST(size_t, std::string)
    {
        std::stringstream ss;
        ss << from;
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

    CAST(bool, arrayref_t)
    {
        return from != nullptr;
    }

    CAST(arrayref_t, arrayref_t)
    {
        return from;
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

    INVALID_CAST(double, blobref_t)
    INVALID_CAST(empty_t, blobref_t)
    INVALID_CAST(arrayref_t, blobref_t)

    INVALID_CAST(double, arrayref_t)
    INVALID_CAST(empty_t, arrayref_t)
    INVALID_CAST(blobref_t, arrayref_t)
      
#else

#define CAST(To, From) \
template<>\
To to(const From &from);


#define INVALID_CAST(To, From) \
template<>\
To to(const From &f);

// specialize casts here. to<to,from>
CAST(std::string, double)
CAST(std::string, std::string)
CAST(double, std::string)
CAST(double, double)
CAST(bool, double)
CAST(bool, std::string)
CAST(size_t, std::string)
CAST(bool, blobref_t)
CAST(blobref_t, blobref_t)
CAST(bool, arrayref_t)
CAST(arrayref_t, arrayref_t)
CAST(std::string, blobref_t)
CAST(std::string, arrayref_t)

INVALID_CAST(double, blobref_t)
INVALID_CAST(empty_t, blobref_t)
INVALID_CAST(arrayref_t, blobref_t)

INVALID_CAST(double, arrayref_t)
INVALID_CAST(empty_t, arrayref_t)
INVALID_CAST(blobref_t, arrayref_t)

#endif


}

#endif // __simpl_cast_h__