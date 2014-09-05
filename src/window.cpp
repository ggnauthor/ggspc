#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "main.h"
#include "window.h"
#include "device/device.h"
#include "device/graphic.h"
#include "utils/ini.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

Window* g_scrn = NULL;

extern IniFileInfo*  g_ini;

LRESULT CALLBACK Window::wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  Window* window = (Window*)GetWindowLong(p_hwnd, GWL_USERDATA);

  switch (p_msg) {
    case WM_CLOSE:
      DestroyWindow(p_hwnd);
      break;
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_ACTIVATEAPP:
      if (window) {
        window->active_ = (p_wp & 0xFFFF) != WA_INACTIVE;
        g_dev_emu->activate();
      }
    case WM_PAINT: {
      PAINTSTRUCT ps;
      HDC hdc = BeginPaint(window->hwnd(), &ps);
      FillRect(hdc, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));
      EndPaint(window->hwnd(), &ps);
      } break;
  }
  return DefWindowProc(p_hwnd, p_msg, p_wp, p_lp);
}

Window::Window() {
  HINSTANCE hinst = GetModuleHandle(NULL);

  strcpy(class_name_, APPNAME);

  width_  = kScreenWidth;
  height_ = kScreenHeight;

  WNDCLASSA	wc;
	wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = Window::wnd_proc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hinst;
	wc.hIcon         = LoadIcon(hinst, NULL);
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
	wc.lpszMenuName  = "";
	wc.lpszClassName = class_name_;
  if (RegisterClassA(&wc) == 0) {
  }

  RECT screen_rect;
	SystemParametersInfo(SPI_GETWORKAREA, 0, &screen_rect, 0);

  RECT src_rect = { 0, 0, width_, height_ };
  int style = WS_OVERLAPPED | WS_SYSMENU | WS_CAPTION;
  AdjustWindowRectEx(&src_rect, style, false, 0);

  int window_width  = src_rect.right - src_rect.left;
  int window_height = src_rect.bottom - src_rect.top;

	hwnd_ = CreateWindowA(class_name_, "", style,
    screen_rect.left + (screen_rect.right  - screen_rect.left - window_width) / 2,
		screen_rect.top  + (screen_rect.bottom - screen_rect.top - window_height) / 2,
    window_width, window_height,
    NULL, NULL, hinst, NULL);
	SetWindowTextA(hwnd_, class_name_);
	SetWindowLong(hwnd_, GWL_USERDATA, (LONG)this);
  ShowWindow(hwnd_, SW_SHOW);

  active_ = true;

  // タイマー精度向上
  timeBeginPeriod(1);
}

Window::~Window() {
  // タイマー精度向上解除
  timeEndPeriod(1);
}

void Window::update_title_text() {
  s8 title[1024];
  sprintf(title, "%s [%s]", APPNAME, g_dev_emu->product_name());
  SetWindowText(hwnd_, title);
}

void Window::errMsgBox(const s8* p_fmt, ...) {
	va_list ap;
	va_start(ap, p_fmt);
	
	s8 buf[1024];
	vsprintf(buf, p_fmt, ap);
  MessageBox(hwnd_, buf, NULL, NULL);

	va_end(ap);
}
