#ifndef __simpl_static_stack_h__
#define __simpl_static_stack_h__

#include <array>
#include <stdexcept>

namespace simpl
{
    namespace detail
    {
        template <typename T, size_t Size>
        class static_stack
        {
            std::array<T, Size> stack_;
            size_t sptr_;

        public:
            static_stack()
                :sptr_(0)
            {
            }

            ~static_stack()
            {
                while (sptr_ > 0)
                {
                    pop();
                }
            }

            T& push(const T &v)
            {
                if (sptr_ >= Size)
                    throw std::runtime_error("stack overflow");
                stack_[sptr_++] = v;
                return top();
            }

            T& push(T &&v)
            {
                if (sptr_ >= Size)
                    throw std::runtime_error("stack overflow");
                stack_[sptr_++] = std::move(v);
                return top();
            }

            void pop(size_t s=1)
            {
                if (s > sptr_)
                    throw std::runtime_error("stack underflow");
              
                while (s > 0)
                {
                    stack_[--sptr_] = T{}; // this will destruct the object
                    --s;
                }
            }

            T &top()
            {
                return offset(0);
            }

            const T &top() const
            {
                return offset(0);
            }

            T &offset(size_t s)
            {
                auto idx = s + 1;
                if (idx > sptr_)
                    throw std::runtime_error("stack underflow");
                return stack_[sptr_ - idx];
            }

            const T &offset(size_t s) const
            {
                auto idx = s + 1;
                if (idx > sptr_)
                    throw std::runtime_error("stack underflow");
                return stack_[sptr_ - idx];
            }

            bool empty() const
            {
                return sptr_ == 0;
            }

            size_t size() const
            {
                return sptr_;
            }

        };
    }
}

#endif //__simpl_static_stack_h__