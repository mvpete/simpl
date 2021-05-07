#ifndef __simpl_library_h__
#define __simpl_library_h__

#include <simpl/vm.h>

namespace simpl
{
    class library
    {
    public:
        virtual ~library() = default;
        virtual void load(vm &vm) = 0;
        virtual const char* name() const = 0;
    };
}

#endif //__simpl_library_h__