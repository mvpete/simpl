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
				char d = delim[0];
				std::vector<simpl::value_t> res;
				res.emplace_back(std::string{});
				for (const auto& c : s)
				{
					if (c == d)
					{
						res.emplace_back(std::string{});
						continue;
					}
					std::get<std::string>(res.back()).push_back(c);
				}
				return make_array(std::move(res));
			});
			vm.reg_fn("join", [](const array_t& arr, const std::string& delim)
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
			vm.reg_fn("concat", [](const std::string& s1, const std::string& s2)
			{
				return detail::format("{0}{1}", s1, s2);
			});

		}
	};
}

#endif //__simpl_string_h__

