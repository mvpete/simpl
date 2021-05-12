#ifndef __simpl_array_h__
#define __simpl_array_h__

#include <simpl/library.h>

namespace simpl
{
	class array_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "array";
		}

		void load(vm &vm) override
		{
			vm.reg_fn("size", [](const array_t &arr)
			{
				return (double)arr.values.size();
			});

			vm.reg_fn("push", [](array_t &arr, const value_t &v)
			{
				arr.values.push_back(v);
			});

			vm.reg_fn("pop", [](array_t &arr)
			{
				arr.values.pop_back();
			});

		}
	};
}



#endif // __simpl_array_h__