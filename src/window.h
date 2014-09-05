#ifndef _window
#define _window

#include "main.h"

class Window {
public:
  Window();
  ~Window();

  void update_title_text();
  void errMsgBox(const s8* p_fmt, ...);

  HWND hwnd() { return hwnd_; }
  bool active() { return active_; }

private:
  static LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp);

  s8     class_name_[256];
  HWND   hwnd_;
  int    width_;
  int    height_;
  HACCEL accel_;
  bool   active_;
};

extern Window*  g_scrn;

#endif // _window
