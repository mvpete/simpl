#ifndef __simpl_gui_edit_h__
#define __simpl_gui_edit_h__

#include <simpl/detail/types.h>
#include <simpl/detail/type_traits.h>
#include <simpl/libraries/gui.window.h>

#include <map>
#include <string>

namespace simpl
{
	class edit : public window
	{
	public:
		edit(window& parent, const std::string& initial_value, number x, number y, number width, number height)
			:window(parent.machine()), down_(false)
		{
			hwnd_ = ::CreateWindowExA(
				0,
				"EDIT",
				initial_value.c_str(),
				WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_BORDER | ES_LEFT | ES_AUTOHSCROLL,
				static_cast<int>(x),
				static_cast<int>(y),
				static_cast<int>(width),
				static_cast<int>(height),
				parent.handle(),
				nullptr,
				nullptr,
				this);

			::SetWindowLongPtrA(hwnd_, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));
			og_wproc_ = reinterpret_cast<WNDPROC>(::SetWindowLongPtrA(hwnd_, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(&edit::wnd_proc)));
		}

	public:
		enum class actions { on_click_action, on_focus_action, on_blur_action, on_change_action };

		void set_action_method(actions a, const std::string& method)
		{
			action_map_[a] = method;
		}

	private:
		void invoke(actions action)
		{
			const auto found = action_map_.find(action);
			if (found != action_map_.end())
				machine().invoke(found->second);
		}

		void on_click()
		{
			invoke(actions::on_click_action);
		}

		void on_change()
		{
			invoke(actions::on_change_action);
		}

	private:
		static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			auto self = get_self(hwnd);
			if (self == nullptr)
			{
				return ::DefWindowProcA(hwnd, uMsg, wParam, lParam);
			}

			switch (uMsg)
			{
			case WM_LBUTTONDOWN:
			{
				self->down_ = true;
				break;
			}
			case WM_LBUTTONUP:
			{
				if (self->down_)
				{
					self->on_click();
				}
				self->down_ = false;
				break;
			}
			case WM_SETFOCUS:
			{
				self->invoke(actions::on_focus_action);
				break;
			}
			case WM_KILLFOCUS:
			{
				self->invoke(actions::on_blur_action);
				break;
			}
			case WM_CHAR:
			case WM_CUT:
			case WM_PASTE:
			case WM_CLEAR:
			case WM_SETTEXT:
			{
				self->on_change();
				break;
			}
			}

			return ::CallWindowProcA(self->og_wproc_, hwnd, uMsg, wParam, lParam);
		}

		static edit* get_self(HWND hwnd)
		{
			return reinterpret_cast<edit*>(::GetWindowLongPtrA(hwnd, GWLP_USERDATA));
		}

		WNDPROC og_wproc_;
		bool down_;
		std::map<actions, std::string> action_map_;
	};

	template<>
	struct detail::is_valid_arg_type<edit> : std::true_type {};

	template<>
	struct detail::simple_type_info<edit>
	{
		static const char* name() noexcept
		{
			return "edit";
		};

		static bool is_convertible(const std::string& t) noexcept
		{
			return t == "window";
		}

		static const char* inherits() noexcept
		{
			return "window";
		}
	};
}

#endif // __simpl_gui_edit_h__
