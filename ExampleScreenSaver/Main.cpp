#include "PCH.hpp"

#pragma comment(lib, "Scrnsavw.lib")
#pragma comment(lib, "ComCtl32.lib")
#pragma comment(lib, "Gdiplus.lib")

#ifndef _WIN64
using GdiPlusToken = ULONG_PTR;
#else	
using GdiPlusToken = UINT_PTR;
#endif

#ifdef _DEBUG
class Logger : public std::stringstream
{
public:
	Logger() :
		std::stringstream()
	{
		*this << std::chrono::system_clock::now() << ' ';
	}
	
	~Logger()
	{
		const std::string& message = str();
		OutputDebugStringA(message.c_str());
	}
};
#else
class Logger
{
public:
	template <typename T>
	Logger& operator << (T)
	{
		return *this;
	}
};
#endif

std::ostream& operator << (std::ostream& os, const Gdiplus::RectF& r)
{
	return os << r.X << ", " << r.Y << ", " << r.Width << ", " << r.Height;
}

class ScreenSaver
{
public:
	ScreenSaver(HWND window) :
		_window(window)
	{
		Gdiplus::GdiplusStartupInput gdiPlusStartupInput;
		Gdiplus::GdiplusStartup(&_token, &gdiPlusStartupInput, nullptr);

		constexpr UINT framesPerSecond = 60;
		constexpr UINT interval = 1000 / framesPerSecond;
		_timer = SetTimer(window, 1, interval, nullptr);

		Logger() << "Created\n";
	}

	~ScreenSaver()
	{
		_bitmap.reset();

		if (_timer)
		{
			KillTimer(_window, _timer);
			_timer = 0;
		}

		if (_token)
		{
			Gdiplus::GdiplusShutdown(_token);
			_token = 0;
		}

		Logger() << "Destroyed\n";
	}

	uint8_t FunkyByte(uint8_t offset) const
	{
		float derp = std::sinf(0.01f * _elapsed + offset);
		return static_cast<uint8_t>(derp * 0x80 + 0x7F);
	}

	uint32_t FunkyColor() const
	{
		struct ColorBGRA
		{
			uint8_t Blue;
			uint8_t Green;
			uint8_t Red;
			uint8_t Alpha;
		};

		union Color
		{
			uint32_t Value;
			ColorBGRA Components;
		} color;

		color.Components.Alpha = 0xFF;
		color.Components.Red = FunkyByte(0);
		color.Components.Green = FunkyByte(2);
		color.Components.Blue = FunkyByte(4);

		return color.Value;
	}

	Gdiplus::PointF TextPosition(const RECT& paintArea) const
	{
		float canvasWidth = float(paintArea.right - paintArea.left);
		float canvasHeight = float(paintArea.bottom - paintArea.top);

		Gdiplus::PointF position(
			(canvasWidth - _textRect.Width) / 2,
			(canvasHeight - _textRect.Height) / 2);

		float radius = min(position.X, position.Y);

		float delay = 50.0f;

		position.X += radius * std::sinf(_elapsed / delay);
		position.Y -= radius * std::cosf(_elapsed / delay);

		return position;
	}

	void MeasureText(std::wstring_view text, const Gdiplus::Font& font)
	{
		Gdiplus::Graphics graphics(_window);

		graphics.MeasureString(
			text.data(),
			static_cast<INT>(text.size()),
			&font,
			Gdiplus::PointF(),
			&_textRect);
	}

	void Resize(WORD screenWidth, WORD screenHeight)
	{
		const float fontSize = std::abs(screenWidth - screenHeight) / 2.0f;
		const Gdiplus::Font font(L"Segoe", fontSize, Gdiplus::FontStyleBold, Gdiplus::UnitPixel);
		const std::wstring message = L"Example!";

		MeasureText(message, font);

		Logger() << "Resized\n"
			<< "\tScreen width: " << screenWidth << "px\n"
			<< "\tScreen height: " << screenHeight << "px\n"
			<< "\tFont size: " << fontSize << "px\n"
			<< "\tText rectangle: " << _textRect << '\n';

		_bitmap = std::make_unique<Gdiplus::Bitmap>(
			static_cast<INT>(_textRect.Width),
			static_cast<INT>(_textRect.Height));

		Gdiplus::Graphics graphics(_bitmap.get());

		graphics.SetTextRenderingHint(Gdiplus::TextRenderingHint::TextRenderingHintAntiAlias);

		const Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255));

		graphics.DrawString(
			message.c_str(),
			static_cast<int>(message.size()),
			&font,
			_textRect,
			nullptr,
			&textBrush);
	}

	void Update()
	{
		++_elapsed;

		_backgroundColor.SetValue(FunkyColor());

		InvalidateRect(_window, nullptr, false);
	}

	void Paint()
	{
		HDC hdc = BeginPaint(_window, &_paint);

		Gdiplus::Graphics graphics(hdc);
		graphics.Clear(_backgroundColor);

		if (_bitmap)
		{
			Gdiplus::PointF position = TextPosition(_paint.rcPaint);

			Gdiplus::CachedBitmap cached(_bitmap.get(), &graphics);

			graphics.DrawCachedBitmap(&cached,
				static_cast<INT>(position.X),
				static_cast<INT>(position.Y));

		}
	
		EndPaint(_window, &_paint);
	}

private:
	const HWND _window;
	UINT_PTR _timer = 0;
	DWORD _elapsed = 0;

	PAINTSTRUCT _paint;
	GdiPlusToken _token = 0;
	Gdiplus::Color _backgroundColor;
	Gdiplus::RectF _textRect;

	std::unique_ptr<Gdiplus::Bitmap> _bitmap;
};

std::unique_ptr<ScreenSaver> screenSaver;

BOOL WINAPI ScreenSaverConfigureDialog(HWND, UINT, WPARAM, LPARAM)
{
	Logger() << "ScreenSaverConfigureDialog\n";
	return FALSE;
}

BOOL WINAPI RegisterDialogClasses(HANDLE)
{
	Logger() << "RegisterDialogClasses\n";
	return TRUE;
}

LRESULT WINAPI ScreenSaverProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		case WM_CREATE:
		{
			_ASSERTE(!screenSaver);
			screenSaver = std::make_unique<ScreenSaver>(window);
			break;
		}
		case WM_DESTROY:
		{
			_ASSERTE(screenSaver);
			screenSaver.reset();
			break;
		}
		case WM_SIZE:
		{
			_ASSERTE(screenSaver);
			screenSaver->Resize(LOWORD(lParam), HIWORD(lParam));
			break;
		}
		case WM_ERASEBKGND:
		{
			return TRUE;
		}
		case WM_PAINT:
		{
			_ASSERTE(screenSaver);
			screenSaver->Paint();
			break;
		}
		case WM_TIMER:
		{
			_ASSERTE(screenSaver);
			screenSaver->Update();
			break;
		}
	}

	return DefScreenSaverProc(window, message, wParam, lParam);
}