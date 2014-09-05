#ifndef _main
#define _main

#include <xmmintrin.h>

#define WINVER       0x0501
#define _WIN32_WINNT 0x0501
//#define _WIN32_IE 0x0501

typedef char             s8;
typedef short            s16;
typedef long             s32;
typedef __int64          s64;
typedef unsigned char    u8;
typedef unsigned short   u16;
typedef unsigned long    u32;
typedef unsigned __int64 u64;
typedef __m128           u128;
typedef float            f32;
typedef double           f64;

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include "../src/network/network.h"

struct ArgumentParams {
  ArgumentParams() :
      port(NetworkMgr::kDefaultPort),
      window_x(0),
      window_y(0) {
  }
  u16 port;
  int window_x;
  int window_y;
};

class App {
public:
  enum {
    kWindowW = 300,
    kWindowH = 120,
  };
  App() :
      done_(false),
      inst_(NULL),
      win_dc_(NULL),
      back_dc_(NULL),
      back_bmp_(NULL),
      back_old_bmp_(NULL),
      fixed_font_(NULL) {
    memset(&arg_params_, 0, sizeof(arg_params_));
  }

  ~App() {
  }

  void init(HINSTANCE p_inst);
  void clean();

  void readArguments(const s8* p_cmdline);
  void createWindow();
  HFONT createFixedFont();
  static LRESULT CALLBACK wndProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp);
  void updateWindowTitle();
  void eventLoop();
  void draw();
  void entryLobby(bool p_busy);

  bool done() { return done_; }
  void set_done(bool p_done) { done_ = p_done; }
  
private:
  bool      done_;
  HINSTANCE inst_;
  ArgumentParams arg_params_;
  s8      window_class_[256];
  HWND    hwnd_;
  HDC     win_dc_;
  HDC     back_dc_;
  HBITMAP	back_bmp_;
  HBITMAP	back_old_bmp_;
  HFONT   fixed_font_;
  HGDIOBJ old_font_;

  NetworkMgr* net_mgr_;
  u16 port_;
};

extern App g_app;

#endif // _main
