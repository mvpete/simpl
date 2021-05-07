#ifndef __simpl_object_h__
#define __simpl_object_h__

#include <memory>

namespace simpl
{
    class object
    {
    public:
        virtual ~object() = default;
        virtual std::string type() const = 0;
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
            return typeid(T).name();
        }

        T &value()
        {
            return val_;
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