#ifndef __simpl_gui_h__
#define __simpl_gui_h__

#include <simpl/value.h>
#include <simpl/library.h>

#include <simpl/libraries/gui.window.h>
#include <simpl/libraries/gui.button.h>
#include <simpl/libraries/gui.text.h>

#include <Windows.h>

namespace simpl
{

	class gui_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "gui_lib";
		}

		void load(vm &vm) override
		{
			window::register_class();

			// This is to participate w/ function def pattern matching.
			vm.register_type<window>();
			vm.register_type<button>();
			vm.register_type<text>();

			vm.reg_fn("create_wnd", [&vm](const std::string &name, number x, number y, number width, number height)
			{
				return make_ref<window>(vm, name, x,y,width,height);
			});

			vm.reg_fn("create_btn", [](window &w, const std::string &name, number x, number y, number width, number height)
			{
				return make_ref<button>(w, name, x, y, width, height);
			});
			vm.reg_fn("create_text", [](window &w, const std::string &name, number x, number y, number width, number height)
			{
				return make_ref<text>(w, name, x, y, width, height);
			});


			vm.reg_fn("show", [](window &w)
			{
				w.show();
				MSG msg = { };
				while (GetMessage(&msg, NULL, 0, 0))
				{
					TranslateMessage(&msg);
					DispatchMessage(&msg);
				}
			});
			
			vm.reg_fn("set_text", [](window &w, const value_t &val)
			{
				w.set_text(val);
			});

			vm.reg_fn("on_click", [](button &b, const std::string &method)
			{
				b.set_action_method(button::actions::on_click_action, method);
			});



		}
	};
}

#endif //__simpl_gui_h__