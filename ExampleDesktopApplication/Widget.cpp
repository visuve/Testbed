#include "PCH.hpp"
#include "Widget.hpp"
#include <Resource.h>

namespace Example
{
	UINT_PTR Identifier = 0;

	LRESULT CALLBACK SubClassProcedure(HWND window, UINT message, WPARAM wParam, LPARAM lParam, UINT_PTR uid, DWORD_PTR data)
	{
		if (message == WM_NCDESTROY)
		{
			RemoveWindowSubclass(window, SubClassProcedure, uid);
		}
		else
		{
			Widget* self = reinterpret_cast<Widget*>(data);

			if (self)
			{
				self->HandleMessage(message, wParam, lParam, uid, data);
			}
		}

		return DefSubclassProc(window, message, wParam, lParam);
	}

	Widget::Widget(
		DWORD extraStyle,
		const wchar_t* className,
		const wchar_t* windowName,
		DWORD style,
		int x,
		int y,
		int w,
		int h,
		HWND parent,
		HMENU menu,
		HINSTANCE instance,
		void* data) :
		_window(CreateWindowExW(
			extraStyle,
			className,
			windowName,
			style,
			x,
			y,
			w,
			h,
			parent,
			menu,
			instance,
			data))
	{
	}

	bool Widget::Intercept(HWND window)
	{
		return SetWindowSubclass(window, SubClassProcedure, ++Identifier, reinterpret_cast<DWORD_PTR>(this));
	}

	bool Widget::Intercept(Widget* widget)
	{
		return SetWindowSubclass(widget->_window, SubClassProcedure, ++Identifier, reinterpret_cast<DWORD_PTR>(this));
	}

	std::wstring Widget::Text() const
	{
		std::wstring buffer;
		int length = GetWindowTextLength(_window);

		if (length > 0)
		{
			buffer.resize(length);
			GetWindowText(_window, &buffer.front(), length + 1);
		}

		return buffer;
	}

	LRESULT Widget::Send(UINT message, WPARAM wParam, LPARAM lParam) const
	{
		return SendMessage(_window, message, wParam, lParam);
	}

	void Widget::HandleMessage(UINT message, WPARAM, LPARAM, UINT_PTR, DWORD_PTR)
	{
		if (message == WM_COMMAND) // Just as an example
		{
			OutputDebugStringW(L"DING!\n");
		}
	}
}