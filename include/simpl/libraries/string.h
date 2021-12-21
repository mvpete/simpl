#ifndef __simpl_io_h_
#define __simpl_io_h_

#include <simpl/library.h>

namespace simpl
{
	class string_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "string_lib";
		}

		void load(vm &vm) override
		{
			vm.reg_fn("split", [](const std::string &s, const std::string &delim)
			{

			});
		}
	};
}

#endif //__simpl_io_h__

