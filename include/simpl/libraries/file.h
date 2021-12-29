#ifndef __simpl_file_h__
#define __simpl_file_h__

#include <simpl/value.h>
#include <simpl/library.h>

#include <fstream>


namespace simpl
{
	// This is so we can use the compile time type. i.e. binding
	// C++ functions into the runtime.
	template<>
	struct detail::is_valid_arg_type<std::fstream> : std::true_type {};

	template<>
	struct detail::simple_type_info<std::fstream>
	{
		static const char* name() noexcept
		{
			return "file";
		};

		static bool is_convertible(const std::string &t)
		{
			return false;
		}
	};

	using file = std::fstream;

    class file_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "file_lib";
		}

		void load(vm &vm) override
		{
			// This is to participate w/ function def pattern matching.
			vm.register_type<std::fstream>("file");

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
			vm.reg_fn("writeln", [](file &fs, const std::string &s) -> void
			{
				fs << s << '\n';
			});
			vm.reg_fn("getln", [](file &f)
			{
				std::string line;
				std::getline(f, line);
				return line;
			});

		}
	};
}

#endif //__simpl_file_h__