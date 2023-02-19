#ifndef __simpl_string_h_
#define __simpl_string_h_

#include <simpl/library.h>
#include <simpl/value.h>

namespace simpl
{
	class string_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "string";
		}

		void load(vm &vm) override
		{
			vm.reg_fn("length", [](const std::string& s)
			{
				return (double)s.length();
			});
			vm.reg_fn("split", [](const std::string &s, const std::string &delim)
			{

			});
			vm.reg_fn("at", [](const std::string& s, number i)
			{
				return std::string{ s.at((int)i) };
			});
			vm.reg_fn("substr", [](const std::string &s, number offset)
			{
				return s.substr((int)offset);
			});
			vm.reg_fn("substr", [](const std::string& s, number offset, number count)
			{
				return s.substr((int)offset, (int)count);
			});

		}
	};
}

#endif //__simpl_string_h__

