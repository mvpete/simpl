#ifndef __simpl_object_h__
#define __simpl_object_h__

#include <simpl/detail/type_traits.h>

#include <memory>

namespace simpl
{
    class object
    {
    public:
        virtual ~object() = default;
        virtual std::string type() const = 0;

        virtual bool is_convertible(const std::string &t) = 0;
        virtual void *value() = 0;
    };

    template <typename T>
    class ref : public object
    {
    public:
        template<typename ...Args>
        ref(Args &&...args)
            :val_(std::forward<Args>(args)...)
        {

        }
        std::string type() const
        {
            return detail::simple_type_info<T>::name();
        }

        bool is_convertible(const std::string &t)
        {
            return detail::simple_type_info<T>::is_convertible(t);
        }

        void *value()
        {
            return &val_;
        }

    private:
        T val_;
    };

    using objectref_t = std::shared_ptr<object>;

    template<typename T>
    using ref_t = std::shared_ptr<ref<T>>;

    template <typename T, typename ...Args>
    objectref_t make_ref(Args&&...args)
    {
        return std::make_shared<ref<T>>(std::forward<Args>(args)...);
    }
}


#endif // __simpl_object_h__