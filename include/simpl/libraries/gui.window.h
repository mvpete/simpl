#ifndef __simpl_gui_window_h__
#define __simpl_gui_window_h__

#include <simpl/detail/type_traits.h>
#include <simpl/detail/types.h>
#include <simpl/vm.h>

#include <string>
#include <Windows.h>

namespace simpl
{
	class window
	{
	public:
		static const char *class_name() { return "simpl.window"; };

		static void register_class()
		{
			WNDCLASSA wc = {};
			wc.lpfnWndProc = &window::wnd_proc;
			wc.lpszClassName = class_name();

			RegisterClassA(&wc);
		}

		window(vm& vm, const std::string &title, number x, number y, number width, number height)
			:vm_(vm)
		{
			hwnd_ = ::CreateWindowExA(
				0,
				class_name(),
				title.c_str(),
				WS_OVERLAPPED | WS_SYSMENU,
				static_cast<int>(x),
				static_cast<int>(y),
				static_cast<int>(width),
				static_cast<int>(height),
				nullptr,
				nullptr,
				nullptr,
				this);
		}

		void show()
		{
			::ShowWindow(hwnd_, SW_SHOW);
		}

		HWND handle()
		{
			return hwnd_;
		}

		vm &machine()
		{
			return vm_;
		}

		void set_text(const value_t &value)
		{
			const auto text = cast<std::string>(value);
			::SendMessage(hwnd_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
		}

	protected:
		window(vm &vm)
			:hwnd_(0), vm_(vm) {}


		HWND hwnd_;
		vm &vm_;

	public:
		static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_DESTROY:
				PostQuitMessage(0);
				return 0;

			case WM_PAINT:
			{
				PAINTSTRUCT ps = {};
				HDC hdc = BeginPaint(hwnd, &ps);
				FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));
				EndPaint(hwnd, &ps);
			}
			return 0;

			}
			return DefWindowProc(hwnd, uMsg, wParam, lParam);
		}

	};

	template<>
	struct detail::is_valid_arg_type<window> : std::true_type {};

	template<>
	struct detail::simple_type_info<window>
	{
		static const char *name() noexcept
		{
			return "window";
		};

		static bool is_convertible(const std::string &t) 
		{
			return false;
		}

		static const char *inherits() noexcept
		{
			return nullptr;
		}
	};

}

#endif // __simpl_gui_window_h__

