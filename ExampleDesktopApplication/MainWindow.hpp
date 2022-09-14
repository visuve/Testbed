#pragma once

#include "Window.hpp"

namespace Example
{
	class MainWindow : public Window<MainWindow>
	{
	public:
		MainWindow(HINSTANCE instance);
		~MainWindow();

		WNDCLASSEXW RegisterInfo() const override;
		CREATESTRUCTW CreateInfo() const override;
		bool HandleMessage(UINT, WPARAM, LPARAM) override;

	private:
		static INT_PTR CALLBACK About(HWND, UINT, WPARAM, LPARAM);

		HWND _textBox = nullptr;
		HWND _button = nullptr;
		HWND _progressBar = nullptr;
		UINT_PTR _timer = 0;
		UINT _elapsedTime = 0;
		COLORREF _backgroundColor = { 0 };
	};
}