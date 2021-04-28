#ifndef __simpl_detail_signature_h__
#define __simpl_detail_signature_h__

namespace simpl
{
    namespace detail
    {
    
        template <typename T>
        struct arity : arity<decltype(&T::operator())> {};

        // Free function
        template <typename R, typename... Args>
        struct arity<R(*)(Args...)> : std::integral_constant<size_t, sizeof...(Args)> {};

        // Pointer to member
        template <typename R, typename C, typename... Args>
        struct arity<R(C:: *)(Args...)> :
            std::integral_constant<size_t, sizeof...(Args)> {};

        // Pointer to const member
        template <typename R, typename C, typename... Args>
        struct arity<R(C:: *)(Args...) const> :
            std::integral_constant<size_t, sizeof...(Args)> {};


        template <typename Fn>
        struct signature
        {
            constexpr static size_t arity = arity<Fn>::value;
        };

        template <typename Fn>
        constexpr auto get_signature()->signature<Fn>
        {
            return signature<Fn>{};
        }
    }
}

#endif // __simpl_detail_signature_h__