#ifndef _debugger_gui_screen
#define _debugger_gui_screen

#include "main.h"
#include <utils/ds_window.h>

class ScreenWindow {
public:
  enum {
#ifdef _ENV2
    VIEW_X = 854 + 150,
    VIEW_Y = 564 + 40,
    VIEW_W = 320,
    VIEW_H = 240,
#else
    VIEW_X = 50,
    VIEW_Y = 50,
    VIEW_W = 320,
    VIEW_H = 240,
#endif
    VIEW_ACTW = 640,
    VIEW_ACTH = 480,
  } Const;

  ScreenWindow(int p_x, int p_y, int p_width, int p_height);
  ~ScreenWindow();

  bool preTranslateMessage(MSG* p_msg);
  void draw();

  void update_title_text();
  void errMsgBox(const s8* p_fmt, ...);

  HWND hwnd() { return wnd_->getHwnd(); }
  bool active() { return active_; }

  ds_window* wnd_;
  int        width_;
  int        height_;
  HACCEL     accel_;
  bool       active_;
};

extern ScreenWindow*  g_scrn;

#endif // _debugger_gui_screen
