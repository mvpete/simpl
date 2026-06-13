#ifndef __simpl_gui_window_h__
#define __simpl_gui_window_h__

#include <simpl/detail/type_traits.h>
#include <simpl/detail/types.h>
#include <simpl/vm.h>

#include <string>
#include <vector>
#include <Windows.h>

namespace simpl
{
	struct rect { int x; int y; int width; int height; };

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

		void close()
		{
			if (hwnd_ != nullptr && ::IsWindow(hwnd_))
			{
				::DestroyWindow(hwnd_);
				hwnd_ = nullptr;
			}
		}

		HWND handle()
		{
			return hwnd_;
		}

		vm &machine()
		{
			return vm_;
		}

		void bind_self(const objectref_t& self)
		{
			self_ = self;
		}

		void set_text(const value_t &value)
		{
			const auto text = cast<std::string>(value);
			::SendMessage(hwnd_, WM_SETTEXT, 0, reinterpret_cast<LPARAM>(text.c_str()));
		}

		std::string get_text() const
		{
			const auto len = ::GetWindowTextLengthA(hwnd_);
			if (len <= 0)
			{
				return {};
			}

			std::vector<char> buffer(static_cast<size_t>(len) + 1);
			::GetWindowTextA(hwnd_, buffer.data(), len + 1);
			return std::string{ buffer.data() };
		}

		void set_pos(number x, number y, number cx, number cy)
		{
			::SetWindowPos(hwnd_, nullptr, (int)x, (int)y, (int)cx, (int)cy, 0);
		}

		rect get_pos() const
		{
			RECT r = {0};
			::GetWindowRect(hwnd_, &r);
			MapWindowPoints(HWND_DESKTOP, GetParent(hwnd_), (LPPOINT)&r, 2);
			return rect{ r.left, r.top, r.right-r.left, r.bottom-r.top };
		}

		static bool poll_one()
		{
			MSG msg = {};
			const auto has_message = ::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
			if (has_message == 0)
			{
				return false;
			}

			if (msg.message == WM_QUIT)
			{
				return false;
			}

			::TranslateMessage(&msg);
			::DispatchMessage(&msg);
			return true;
		}

		static void run_loop()
		{
			MSG msg = {};
			while (::GetMessage(&msg, nullptr, 0, 0) > 0)
			{
				::TranslateMessage(&msg);
				::DispatchMessage(&msg);
			}
		}

	protected:
		window(vm &vm)
			:hwnd_(0), vm_(vm) {}

		void invoke_bound_method(const std::string& method)
		{
			if (self_ != nullptr && vm_.can_invoke_dynamic(method, { value_t{ self_ } }))
			{
				vm_.invoke_dynamic(method, { value_t{ self_ } });
				return;
			}

			if (vm_.can_invoke(method))
			{
				vm_.invoke(method);
				return;
			}

			if (self_ != nullptr)
			{
				vm_.invoke_dynamic(method, { value_t{ self_ } });
				return;
			}

			vm_.invoke(method);
		}


		HWND hwnd_;
		vm &vm_;
		objectref_t self_;

	public:
		static LRESULT CALLBACK wnd_proc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
		{
			switch (uMsg)
			{
			case WM_DESTROY:
			{
				PostQuitMessage(0);
				return 0;
			}

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
