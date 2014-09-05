#define _CRTDBG_MAP_ALLOC

#include "main.h"
#include "stlp_wrapper.h"
#include "network/network.h"
#include "utils/misc.h"

#include <stdio.h>
#include <crtdbg.h>
#include "debug_malloc.h"

App g_app;

void App::init(HINSTANCE p_inst) {
  inst_ = p_inst;
  
  net_mgr_ = new NetworkMgr();
  net_mgr_->init(arg_params_.port);

  net_mgr_->entryLobby(false);
  net_mgr_->entryLobbyServer();

  net_mgr_->lookupLobby();
}

void App::clean() {
  if (net_mgr_) {
    net_mgr_->clean();
    delete net_mgr_;
    net_mgr_ = NULL;
  }

  SelectObject(back_dc_, old_font_);
  DeleteObject(fixed_font_);

  SelectObject(back_dc_, back_old_bmp_);
  DeleteDC(back_dc_);
  DeleteObject(back_bmp_);
  
  ReleaseDC(hwnd_, win_dc_);
  
  DestroyWindow(hwnd_);
  UnregisterClass(window_class_, inst_);
}

void App::readArguments(const s8* p_cmdline) {
  bool find;

  s8 value[256];
  find = getArgValue(p_cmdline, "-p", 0, value);
  if (find) arg_params_.port = atoi(value);

  find = getArgValue(p_cmdline, "-x", 0, value);
  if (find) arg_params_.window_x = atoi(value);

  find = getArgValue(p_cmdline, "-y", 0, value);
  if (find) arg_params_.window_y = atoi(value);
}

void App::createWindow() {
  strcpy(window_class_, "netsim");

  WNDCLASS  wc;
  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = wndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = inst_;
  wc.hIcon         = LoadIcon(inst_, NULL);
  wc.hCursor       = LoadCursor(NULL, IDC_ARROW);
  wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
  wc.lpszMenuName  = "";
  wc.lpszClassName = window_class_;
  if (RegisterClass(&wc) == FALSE) return;
  
  hwnd_ = CreateWindowEx(
            WS_EX_TOOLWINDOW | WS_EX_APPWINDOW, window_class_, "",
            WS_OVERLAPPEDWINDOW, 0, 0, 0, 0,
            NULL, NULL, inst_, NULL);
  if (hwnd_ == NULL) return;

  RECT work_area_rect;
  SystemParametersInfo(SPI_GETWORKAREA, 0, &work_area_rect, 0);
  RECT rect = { 0, 0, kWindowW, kWindowH };
  AdjustWindowRectEx(&rect,
    GetWindowLong(hwnd_, GWL_STYLE),
    false,
    GetWindowLong(hwnd_, GWL_EXSTYLE));
  SetWindowPos(
    hwnd_,
    NULL,
    work_area_rect.left + arg_params_.window_x,
    work_area_rect.top  + arg_params_.window_y,
    rect.right - rect.left,
    rect.bottom - rect.top,
    SWP_NOZORDER);
  ShowWindow(hwnd_, SW_SHOW);

  win_dc_ = GetDC(hwnd_);

  // BITMAPINFO設定
  BITMAPINFO bm_info;
  bm_info.bmiHeader.biSize        = sizeof(BITMAPINFOHEADER);
  bm_info.bmiHeader.biWidth       =  kWindowW;
  bm_info.bmiHeader.biHeight      = -kWindowH;
  bm_info.bmiHeader.biPlanes      = 1;
  bm_info.bmiHeader.biBitCount    = 32;
  bm_info.bmiHeader.biCompression = BI_RGB;
  
  // バックバッファ作成
  void *back_bits;
  back_dc_      = CreateCompatibleDC(win_dc_);
  back_bmp_     = CreateDIBSection(NULL, &bm_info, DIB_RGB_COLORS, &back_bits, NULL, 0);
  back_old_bmp_ = (HBITMAP)SelectObject(back_dc_, back_bmp_);

  fixed_font_ = createFixedFont();
  HGDIOBJ old_font_ = SelectObject(back_dc_, fixed_font_);
}

HFONT App::createFixedFont() {
  LOGFONT log_font;
  log_font.lfHeight         = 10;
  log_font.lfWidth          = 0;
  log_font.lfEscapement     = 0;
  log_font.lfOrientation    = 0;
  log_font.lfWeight         = FW_NORMAL;
  log_font.lfItalic         = FALSE;
  log_font.lfUnderline      = FALSE;
  log_font.lfStrikeOut      = FALSE;
  log_font.lfCharSet        = SHIFTJIS_CHARSET;
  log_font.lfOutPrecision   = OUT_DEFAULT_PRECIS;
  log_font.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
  log_font.lfQuality        = DEFAULT_QUALITY;
  log_font.lfPitchAndFamily = DEFAULT_PITCH || FF_DONTCARE;
  strcpy(log_font.lfFaceName, "ＭＳ ゴシック");
  HFONT hfont = CreateFontIndirect(&log_font);
  return hfont;
}

LRESULT CALLBACK App::wndProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  switch (p_msg) {
    case WM_CLOSE:
    case WM_DESTROY:
      PostQuitMessage(0);
      break;
    case WM_ACTIVATE:
      //m_active = ((wp & 0xFFFF) != WA_INACTIVE);
      break;
    case WM_KEYDOWN:
      if (p_wp == VK_ESCAPE) PostQuitMessage(0);
      break;
    case WM_TIMER:
      g_app.draw();
      break;
  }
  return DefWindowProc(p_hwnd, p_msg, p_wp, p_lp);
}

void App::updateWindowTitle() {
  char str[32] = "";
  sprintf(str, "port:%d", port_);
  SetWindowText(hwnd_, str);
}

void App::eventLoop() {
  net_mgr_->startThread();
  SetTimer(hwnd_, 0, 100, NULL);

  while (!done_) {
    tagMSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        done_ = true;
      } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
  }
  KillTimer(hwnd_, 0);
}

void App::draw() {
  RECT client_rect;
  GetClientRect(hwnd_, &client_rect);

  HBRUSH brush = (HBRUSH)GetStockObject(WHITE_BRUSH);
  FillRect(back_dc_, &client_rect, brush);

  SetBkColor(back_dc_, RGB(255, 255, 255));
  SetBkMode(back_dc_, TRANSPARENT);
  SetTextColor(back_dc_, RGB(0, 127, 0));
  DrawText(back_dc_, "F1:connect F2:disconnect F3:w_ignore F4:watch F5:n/a", -1, &client_rect, 0);

  BitBlt(win_dc_, 0, 0, kWindowW, kWindowH, back_dc_, 0, 0, SRCCOPY);
}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
  // メモリリーク検出
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  // 引数解析
  g_app.readArguments(szCmdLine);

  g_app.init(hInstance);

  g_app.createWindow();

  g_app.eventLoop();

  g_app.clean();
}
