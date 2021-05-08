#ifndef __simpl_file_h__
#define __simpl_file_h__

#include <simpl/value.h>
#include <simpl/library.h>

#include <fstream>

namespace simpl
{
	template<>
	struct detail::is_valid_arg_type<std::fstream> : std::true_type {};

	template<>
	struct detail::simple_type_info<std::fstream>
	{
		const char* name() const noexcept
		{
			return "file";
		};
	};

	using file = std::fstream;

    class file_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "file";
		}

		void load(vm &vm) override
		{
			vm.reg_fn("open_f", [](const std::string &name)
			{
				return make_ref<file>(name);
			});
			vm.reg_fn("close_f", [](file &fs)
			{
				fs.close();
			});
			vm.reg_fn("write", [](file &fs, const std::string &s) -> void
			{
				fs << s;
			});
			vm.reg_fn("writeln", [](file &fs, std::string &s) -> void
			{
				fs << s << '\n';
			});

		}
	};
}

#endif //__simpl_file_h__