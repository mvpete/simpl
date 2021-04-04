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
            T& push(const T &v)
            {
                if (sptr_ >= Size)
                    throw std::runtime_error("stack overflow");
                stack_[sptr_++] = v;
                return stack_[sptr_];
            }

            void pop(size_t s=1)
            {
                if (s > sptr_)
                    throw std::runtime_error("stack underflow");
                sptr_ = sptr_ - s;
            }

            T &top()
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