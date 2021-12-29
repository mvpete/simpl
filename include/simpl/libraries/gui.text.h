#ifndef __simpl_gui_text_h__
#define __simpl_gui_text_h__

#include <simpl/detail/types.h>
#include <simpl/detail/type_traits.h>
#include <simpl/libraries/gui.window.h>

#include <string>

#include <windowsx.h>

namespace simpl
{
	class text : public window
	{
	public:
		text(window &parent, const std::string &title, number x, number y, number width, number height)
			:window(parent.machine()), down_(false)
		{
			hwnd_ = ::CreateWindowExA(
				0,
				"STATIC",
				title.c_str(),
				WS_CHILD | WS_VISIBLE,
				static_cast<int>(x),
				static_cast<int>(y),
				static_cast<int>(width),
				static_cast<int>(height),
				parent.handle(),
				nullptr,
				nullptr,
				this);

			::SetWindowLong(hwnd_, GWL_USERDATA, reinterpret_cast<long>(this));
			og_wproc_ = reinterpret_cast<WNDPROC>(::SetWindowLong(hwnd_, GWL_WNDPROC, reinterpret_cast<long>(&text::wnd_proc)));

		}

	public:
		enum actions { on_click_action };
		void set_action_method(actions a, const std::string &method)
		{
			action_map_[a] = method;
		}

	private:
		void on_click()
		{
			if (action_map_.find(on_click_action) != action_map_.end())
				machine().invoke(action_map_.at(on_click_action));
		}


	private:

		static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			auto self = get_self(hwnd);

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
			return 0;

			}
			return ::CallWindowProc(self->og_wproc_, hwnd, uMsg, wParam, lParam);
		}

		static text *get_self(HWND hwnd)
		{
			return reinterpret_cast<text *>(::GetWindowLong(hwnd, GWL_USERDATA));
		}

		WNDPROC og_wproc_;
		bool down_;

		std::map<actions, std::string> action_map_;

	};

	// This is so we can use the compile time type. i.e. binding
	// C++ functions into the runtime.
	template<>
	struct detail::is_valid_arg_type<text> : std::true_type {};

	template<>
	struct detail::simple_type_info<text>
	{
		static const char *name() noexcept
		{
			return "text";
		};

		static bool is_convertible(const std::string &t) noexcept
		{
			return t == "window";
		}

		static const char *inherits() noexcept
		{
			return "window";
		}
	};
}

#endif // __simpl_gui_text_h__
