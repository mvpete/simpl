#ifndef __simpl_io_h_
#define __simpl_io_h_

#include <simpl/library.h>
#include <simpl/cast.h>

namespace simpl
{
    class io_lib final : public library
    {
    public:

		const char *name() const override 
		{
			return "io_lib";
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
			vm.reg_fn("getnum", []()
			{
				double number;
				std::cin >> number;
				return number;
			});
        }
    };
}

#endif //__simpl_io_h__
