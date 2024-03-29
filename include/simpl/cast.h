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
        throw invalid_cast();
    }

    template <typename T>
    struct cast_
    {
        T value;
        void operator()(const empty_t &v)
        {
            value = to<T>(v);
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


    // specialize casts here. to<to,from>
    CAST(std::string, empty_t)
    {
        return "undefined";
    }
    
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

    CAST(bool, empty_t)
    {
        return false;
    }

    CAST(bool, double)
    {
        return from != 0;
    }

    CAST(bool, std::string)
    {
        if (from == "0" || from == "false" || from.empty())
            return false;
        return true;
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
        for (const auto &i : from->values)
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

      
#else

#define CAST(To, From) \
template<>\
To to(const From &from);

// specialize casts here. to<to,from>
CAST(std::string, empty_t)
CAST(std::string, double)
CAST(std::string, std::string)
CAST(double, std::string)
CAST(double, double)
CAST(bool, empty_t)
CAST(bool, double)
CAST(bool, std::string)
CAST(size_t, std::string)
CAST(bool, blobref_t)
CAST(blobref_t, blobref_t)
CAST(bool, arrayref_t)
CAST(arrayref_t, arrayref_t)
CAST(std::string, blobref_t)
CAST(std::string, arrayref_t)


#endif


}

#endif // __simpl_cast_h__