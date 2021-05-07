#ifndef __simpl_io_h_
#define __simpl_io_h_

#include <simpl/library.h>

namespace simpl
{
    class io_lib final : public library
    {
    public:

		const char *name() const override 
		{
			return "io";
		}

        void load(vm &vm) override
        {
			vm.reg_fn("print", [](const value_t &v)
			{
				std::cout << cast<std::string>(v);
			});
			vm.reg_fn("println", [](const value_t &v)
			{
				std::cout << cast<std::string>(v) << "\n";
			});
			vm.reg_fn("getln", []()
			{
				std::string line;
				std::getline(std::cin, line);
				return line;
			});
        }
    };
}

#endif //__simpl_io_h__
