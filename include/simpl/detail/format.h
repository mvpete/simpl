#ifndef __simpl_detail_format_h__
#define __simpl_detail_format_h__

#include <string>
#include <sstream>
#include <tuple>
#include <vector>

namespace simpl
{
namespace detail
{
   
    template<typename CharT, typename IteratorT>
    size_t parse_index(IteratorT &b, IteratorT e)
    {
        std::basic_stringstream<CharT> ss;
        while (b != e)
        {
            if (*b == '}') 
            {
                break;
            }
            if (!isdigit(*b))
                throw std::runtime_error("invalid format identifier");
            ss << *b;
            ++b;
        }
        if (b == e)
            throw std::runtime_error("invalid fmt format");

        size_t index{ 0 };
        ss >> index;
        return index;
    }

    template <size_t Idx>
    struct visitor_impl
    {
        template <typename T, typename F>
        static void visit(const T &tup, size_t idx, F fun)
        {
            constexpr size_t cidx = Idx - 1;
            if (cidx == idx) 
                fun(std::get<cidx>(tup));
            else 
                visitor_impl<cidx>::visit(tup, idx, fun);
        }
    };

    template <>
    struct visitor_impl<0>
    {
        template <typename T, typename F>
        static void visit(const T &tup, size_t idx, F fun) 
        { 
            throw std::runtime_error("index out of bounds"); 
        }
    };

    template <typename F, typename... Ts>
    void visit_at(std::tuple<Ts...> &tup, size_t idx, F fun)
    {
        visitor_impl<sizeof...(Ts)>::visit(tup, idx, fun);
    }

    template <typename F, typename... Ts>
    void visit_at(const std::tuple<Ts...>& tup, size_t idx, F fun)
    {
        visitor_impl<sizeof...(Ts)>::visit(tup, idx, fun);
    }

    template<typename CharT, typename VisitFn>
    std::basic_string<CharT> format_impl(const std::basic_string<CharT>& fmt, VisitFn fmtfn)
    {
        bool escape = false;
        std::basic_stringstream<CharT> ss;
        auto b = fmt.begin();
        auto e = fmt.end();

        for (; b != e; ++b)
        {
            if (escape)
            {
                ss << *b;
                escape = false;
                continue;
            }

            if (*b == '\\')
            {
                escape = true;
            }
            else if (*b == '{')
            {
                auto index = parse_index<CharT>(++b, e);
                fmtfn(ss, index);
            }
            else
                ss << *b;
        }

        return ss.str();
    }
        
    template <typename CharT, typename ...VArgs>
    std::basic_string<CharT> format(const std::basic_string<CharT> &fmt, VArgs&&... args)
    {
        auto values = std::make_tuple(args...);
        return format_impl(fmt, [&values](std::ostream &ss, size_t index)
        {
            visit_at(values, index, [&ss](const auto& v)
            {
                ss << v;
            });
        });
    }

    template <typename CharT, typename ...VArgs>
    std::basic_string<CharT> format(const CharT *fmt, VArgs&&... args)
    {
        return format(std::basic_string<CharT>{fmt}, std::forward<VArgs>(args)...);
    }

    template <typename CharT, typename T>
    std::basic_string<CharT> format_v(const std::basic_string<CharT>& fmt, const std::vector<T>& values)
    {
        return format_impl(fmt, [&values](std::ostream& ss, size_t index)
        {
            ss << values[index];
        });
    }

    template <typename CharT, typename FmtFn>
    std::basic_string<CharT> format_any(const std::basic_string<CharT>& fmt, FmtFn &&fn)
    {
        return format_impl(fmt, std::forward<FmtFn>(fn));
    }

    template <typename CharT, typename T>
    std::basic_string<CharT> format_v(const CharT* fmt, const std::vector<T> &values)
    {
        return format(std::basic_string<CharT>{fmt}, values);
    }

}
}

#endif // __simpl_detail_format_h__