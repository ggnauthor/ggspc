#ifndef _main
#define _main

#include "message.h"

#include <xmmintrin.h>

#include <utils/ds_debug.h>

#undef WINVER
#define WINVER       0x0501
#undef _WIN32_WINNT
#define _WIN32_WINNT 0x0501
//#define _WIN32_IE 0x0501

#define FLT_MAX 3.402823466e+38F
#define FLT_MIN 1.175494351e-38F

#define FLT_BIN1 0x3f800000 // floatの1.0fのバイナリ値
#define FLT_BIN0 0x00000000 // floatの0.0fのバイナリ値

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

#if defined _UNIT_TEST
#define WIN32_LEAN_AND_MEAN
#include <limits.h>
#include <WinUnit.h>
#else // defined _UNIT_TEST
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif // defined _UNIT_TEST

#include <stdlib.h>
#include <stdio.h>

#include "res/resource.h"

//-----------------------------------------------------------------------------
// define
#define APPNAME "ggspc"

#if defined _DEBUGGER
//#include "debugger/debugger.h"

  extern class Debugger* g_dbg;
  #define DBGOUT_CPU if (g_dbg && g_dbg->enable_log_cpu())     ds_util::DBGOUTA("[CPU] ");  if (g_dbg && g_dbg->enable_log_cpu())  ds_util::DBGOUTA
  #define DBGOUT_GS  if (g_dbg && g_dbg->enable_log_gs())      ds_util::DBGOUTA("[GS ] ");  if (g_dbg && g_dbg->enable_log_gs())    ds_util::DBGOUTA
  #define DBGOUT_MEM if (g_dbg && g_dbg->enable_log_mem())     ds_util::DBGOUTA("[MEM] ");  if (g_dbg && g_dbg->enable_log_mem())  ds_util::DBGOUTA
  #define DBGOUT_ELF if (g_dbg && g_dbg->enable_log_elf())     ds_util::DBGOUTA("[ELF] ");  if (g_dbg && g_dbg->enable_log_elf())  ds_util::DBGOUTA
  #define DBGOUT_DMA if (g_dbg && g_dbg->enable_log_dma())     ds_util::DBGOUTA("[DMA] ");  if (g_dbg && g_dbg->enable_log_dma())  ds_util::DBGOUTA
  #define DBGOUT_REC if (g_dbg && g_dbg->enable_log_rec())     ds_util::DBGOUTA("[REC] ");  if (g_dbg && g_dbg->enable_log_rec())  ds_util::DBGOUTA
  #define DBGOUT_APP if (g_dbg && g_dbg->enable_log_app())     ds_util::DBGOUTA("[APP] ");  if (g_dbg && g_dbg->enable_log_app())  ds_util::DBGOUTA
  #define DBGOUT_DDB if (g_dbg && g_dbg->enable_log_ddb())     ds_util::DBGOUTA("[DDB] ");  if (g_dbg && g_dbg->enable_log_ddb())  ds_util::DBGOUTA
  #define DBGOUT_SYM if (g_dbg && g_dbg->enable_log_symbol())  ds_util::DBGOUTA("[SYM] ");  if (g_dbg && g_dbg->enable_log_symbol())  ds_util::DBGOUTA
  #define DBGOUT_ANL if (g_dbg && g_dbg->enable_log_analyze()) ds_util::DBGOUTA("[ANL] ");  if (g_dbg && g_dbg->enable_log_analyze())ds_util::DBGOUTA
  #define DBGOUT_GGS if (g_dbg && g_dbg->enable_log_ggs())     ds_util::DBGOUTA
#else // defined _DEBUGGER
  #define DBGOUT_CPU
  #define DBGOUT_GS
  #define DBGOUT_MEM
  #define DBGOUT_ELF
  #define DBGOUT_DMA
  #define DBGOUT_REC
  #define DBGOUT_APP ds_util::DBGOUTA("[APP] ");  ds_util::DBGOUTA
  #define DBGOUT_DDB
  #define DBGOUT_SYM
  #define DBGOUT_ANL
  #define DBGOUT_GGS
#endif // defined _DEBUGGER

// x86コードテーブルのインデックスに変換する
#define VA2IDX(_ADDR) (((_ADDR) - g_code_addr) >> 2)

typedef bool (*QSORT_FUNC)(const void*, const void*);

class SettingFileMgr;
class PS2ThreadMgr;
class PS2SemaphoreMgr;

class App {
public:
  App() :
      done_(false),
      main_fiber_(NULL),
      execute_fiber_(NULL),
      setting_file_mgr_(NULL) {
  }

  ~App() {
  }

  void openElf(s8* p_elf_dir, s8* p_elf_name);
  bool openCnf(s8* p_cnf_file);

  void initSetting();
  void cleanSetting();
  void applyBinaryPatch(s8* p_patch_file);

  bool done() { return done_; }
  void* main_fiber() { return main_fiber_; }
  void* execute_fiber() { return execute_fiber_; }
  SettingFileMgr* setting_file_mgr() { return setting_file_mgr_; }

  void set_done(bool p_done) { done_ = p_done; }
  void set_main_fiber(void* p_fiber) { main_fiber_ = p_fiber; }
  void set_execute_fiber(void* p_fiber) { execute_fiber_ = p_fiber; }

private:
  bool done_;
  void* main_fiber_;
  void* execute_fiber_;
  SettingFileMgr* setting_file_mgr_;

//  PS2ThreadMgr*    thread_mgr_ = NULL;
//  PS2SemaphoreMgr* sema_mgr_ = NULL;
};

extern App g_app;

extern volatile bool g_vbl_thread_end;

#if !defined _DEBUGGER
extern HANDLE g_execute_thread_hdl;
#endif

extern u32    g_code_addr;
extern u32    g_code_size;

extern s8     g_base_dir[_MAX_PATH];
extern s8     g_elf_name[_MAX_PATH];
extern s8     g_elf_dir[_MAX_PATH];

/*----------*/
/* function */
/*----------*/
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow);

#endif // _main
