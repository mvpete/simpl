#ifndef __simpl_engine_h__
#define __simpl_engine_h__

#include <simpl/vm.h>
#include <simpl/vm_execution_context.h>

#include <simpl/libraries/io.h>
#include <simpl/libraries/file.h>
#include <simpl/libraries/array.h>
#include <simpl/libraries/gui.h>

namespace simpl
{
    class engine
    {
    public:
        engine()
            :ctx_(vm_)
        {
            io_lib{}.load(vm_);
            file_lib{}.load(vm_);
            array_lib{}.load(vm_);
            gui_lib{}.load(vm_);
        }

        vm_execution_context &context()
        {
            return ctx_;
        }

        vm &machine()
        {
            return vm_;
        }

    private:
        vm vm_;
        vm_execution_context ctx_;
    };
}

#endif // __simpl_vm__h__