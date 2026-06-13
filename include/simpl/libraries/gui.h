#ifndef __simpl_gui_h__
#define __simpl_gui_h__

#include <simpl/value.h>
#include <simpl/library.h>

#include <simpl/libraries/gui.window.h>
#include <simpl/libraries/gui.button.h>
#include <simpl/libraries/gui.text.h>
#include <simpl/libraries/gui.edit.h>

#include <Windows.h>

namespace simpl
{
	template<typename WidgetT, typename ...Args>
	objectref_t make_gui_ref(Args&&...args)
	{
		auto widget = make_ref<WidgetT>(std::forward<Args>(args)...);
		reinterpret_cast<WidgetT*>(widget->value())->bind_self(widget);
		return widget;
	}

	class gui_lib final : public library
	{
	public:

		const char *name() const override
		{
			return "gui";
		}

		void load(vm &vm) override
		{
			window::register_class();

			// This is to participate w/ function def pattern matching.
			vm.register_type<window>();
			vm.register_type<button>();
			vm.register_type<text>();
			vm.register_type<edit>();

			vm.reg_fn("create_wnd", [&vm](const std::string &name, number x, number y, number width, number height)
			{
				return make_gui_ref<window>(vm, name, x,y,width,height);
			});

			vm.reg_fn("create_btn", [](window &w, const std::string &name, number x, number y, number width, number height)
			{
				return make_gui_ref<button>(w, name, x, y, width, height);
			});
			vm.reg_fn("create_text", [](window &w, const std::string &name, number x, number y, number width, number height)
			{
				return make_gui_ref<text>(w, name, x, y, width, height);
			});
			vm.reg_fn("create_edit", [](window &w, const std::string &value, number x, number y, number width, number height)
			{
				return make_gui_ref<edit>(w, value, x, y, width, height);
			});

			vm.reg_fn("show", [](window &w)
			{
				w.show();
				window::run_loop();
			});

			vm.reg_fn("show_async", [](window &w)
			{
				w.show();
			});

			vm.reg_fn("poll", []()
			{
				return window::poll_one();
			});

			vm.reg_fn("run", []()
			{
				window::run_loop();
			});

			vm.reg_fn("quit", []()
			{
				::PostQuitMessage(0);
			});
			
			vm.reg_fn("set_text", [](window &w, const value_t &val)
			{
				w.set_text(val);
			});
			vm.reg_fn("get_text", [](window &w)
			{
				return w.get_text();
			});
			vm.reg_fn("close", [](window &w)
			{
				w.close();
			});

			vm.reg_fn("set_pos", [](window& w, number x, number y, number cx, number cy)
			{
				w.set_pos(x, y, cx, cy);
			});

			vm.reg_fn("get_pos", [](window& w)
			{
				auto pos = new_blob();
				auto wpos = w.get_pos();
				pos->values["x"] = number{ (double) wpos.x };
				pos->values["y"] = number{ (double) wpos.y };
				pos->values["width"] = number{ (double)wpos.width };
				pos->values["height"] = number{ (double)wpos.height };
				return pos;
			});

			vm.reg_fn("on_click", [](button &b, const std::string &method)
			{
				b.set_action_method(button::actions::on_click_action, method);
			});
			vm.reg_fn("on_click", [](text &t, const std::string &method)
			{
				t.set_action_method(text::actions::on_click_action, method);
			});
			vm.reg_fn("on_click", [](edit &e, const std::string &method)
			{
				e.set_action_method(edit::actions::on_click_action, method);
			});
			vm.reg_fn("on_focus", [](button &b, const std::string &method)
			{
				b.set_action_method(button::actions::on_focus_action, method);
			});
			vm.reg_fn("on_focus", [](text &t, const std::string &method)
			{
				t.set_action_method(text::actions::on_focus_action, method);
			});
			vm.reg_fn("on_focus", [](edit &e, const std::string &method)
			{
				e.set_action_method(edit::actions::on_focus_action, method);
			});
			vm.reg_fn("on_blur", [](button &b, const std::string &method)
			{
				b.set_action_method(button::actions::on_blur_action, method);
			});
			vm.reg_fn("on_blur", [](text &t, const std::string &method)
			{
				t.set_action_method(text::actions::on_blur_action, method);
			});
			vm.reg_fn("on_blur", [](edit &e, const std::string &method)
			{
				e.set_action_method(edit::actions::on_blur_action, method);
			});
			vm.reg_fn("on_change", [](edit &e, const std::string &method)
			{
				e.set_action_method(edit::actions::on_change_action, method);
			});



		}
	};
}

#endif //__simpl_gui_h__