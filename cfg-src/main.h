#ifndef _main
#define _main

#define WIN32_LEAN_AND_MEAN
#include <windows.h>

#include <assert.h>

#include <xmmintrin.h>

#include "message.h"

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

class SettingFileMgr;

class App {
public:
  App() :
      setting_file_mgr_(NULL) {
  }

  ~App() {
  }

  void init();
  void clean();

  SettingFileMgr* setting_file_mgr() { return setting_file_mgr_; }

private:
  SettingFileMgr* setting_file_mgr_;
};

extern App g_app;

#endif // _main
