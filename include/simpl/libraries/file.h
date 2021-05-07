#ifndef __simpl_file_h__
#define __simpl_file_h__

#include <simpl/value.h>
#include <simpl/library.h>

namespace simpl
{
    class file
    {
	public:
		file() {};
		file(const std::string &name)
			:name_(name)
		{
		}
		std::string get() {
			return name_;
		}

	private:
		std::string name_;
    };

	template<>
	struct detail::is_valid_arg_type<file> : std::true_type {};


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
			vm.reg_fn("close_f", [](file &v)
			{
				int i = 0;
			});
		}
	};
}

#endif //__simpl_file_h__