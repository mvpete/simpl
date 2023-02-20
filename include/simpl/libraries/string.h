#ifndef __simpl_string_h_
#define __simpl_string_h_

#include <simpl/cast.h>
#include <simpl/value.h>
#include <simpl/library.h>
#include <simpl/detail/format.h>

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
				std::stringstream ss;
				bool init = true;
				for (const auto& v : arr.values)
				{
					if (!init)
						ss << delim;
					ss << cast<std::string>(v);
					init = false;
				}
				return ss.str();
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
			vm.reg_fn("format", [](const std::string& fmt, const array_t& arr)
			{
				return detail::format_any(fmt, [&arr](std::ostream& ss, size_t index)
				{
					if (index >= arr.values.size())
						throw std::runtime_error("format index out of range");
					ss << simpl::cast<std::string>(arr.values[index]);
				});
			});


		}
	};
}

#endif //__simpl_string_h__

