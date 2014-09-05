#include "main.h"
#include "elf.h"
#include "utils/ini.h"
#include "stlp_wrapper.h"
#include "cpu/r5900.h"
#include "thread/thread.h"
#include "thread/semaphore.h"
#include "memory/memory.h"
#include "device/device.h"
#include "device/setting.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#include "debugger/gui/window/screen_window.h"
#include "debugger/gui/window/code_window.h"
#else  // defined _DEBUGGER
#include "window.h"
#endif // defined _DEBUGGER
#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

App g_app;

volatile bool g_vbl_thread_end = false;
HANDLE        g_vbl_thread_hdl = NULL;

#if !defined _DEBUGGER
volatile bool g_execute_thread_end = false;
HANDLE        g_execute_thread_hdl = NULL;
#endif // defined _DEBUGGER

u32           g_elf_code_addr = 0;
u32           g_elf_code_size = 0;
u32           g_code_addr = 0;
u32           g_code_size = 0;

IniFileInfo*    g_ini = NULL;
//SettingFileMgr* setting_file_mgr_ = NULL;

s8            g_base_dir[_MAX_PATH];
s8            g_elf_name[_MAX_PATH];
s8            g_elf_dir[_MAX_PATH];

PS2ThreadMgr*    thread_mgr_ = NULL;
PS2SemaphoreMgr* sema_mgr_ = NULL;

#if !defined _DEBUGGER
DWORD CALLBACK execute_proc(LPVOID lpParameter) {

  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));
  
  g_execute_thread_end = false;
  while (g_execute_thread_end == false ||
         g_dev_emu->isRenderingNow()) { // レンダリングを完了してから終了する
    g_cpu->execute(true, false);
  }
  g_execute_thread_end = false;

  ConvertFiberToThread();

  return 0;
}
#endif

#if defined _UNIT_TEST
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved) {
    switch (fdwReason) {
    case DLL_PROCESS_ATTACH:
    // カレントディレクトリをプロジェクトルートにする
    char fname[_MAX_PATH];
    GetModuleFileName(hinstDLL, fname, _MAX_PATH);
    for (int i = 0; i < 3; i++) {
      char* p = strrchr(fname, '\\');
      if (p) *p = '\0';
    }
    SetCurrentDirectory(fname);
        break;
    case DLL_PROCESS_DETACH:
    _CrtDumpMemoryLeaks();
    break;
    case DLL_THREAD_ATTACH: break;
    case DLL_THREAD_DETACH: break;
    }
    return TRUE;
}
#elif defined _DEBUGGER
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
  // メモリリーク検出
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  GetCurrentDirectory(_MAX_PATH, g_base_dir);

  char** argv;
  int argc = ds_util::createArgString(szCmdLine, argv);

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();

  // iniファイル読み込み
  g_ini = new IniFileInfo();
  g_ini->read();

  g_scrn = new ScreenWindow(ScreenWindow::VIEW_X, ScreenWindow::VIEW_Y, ScreenWindow::VIEW_W, ScreenWindow::VIEW_H);
  
  g_dbg = new Debugger;
  g_dbg->init(NULL);
  SetFocus(g_scrn->wnd_->getHwnd());

  g_dev_emu = new Device();

  // デバッグ時は作業ディレクトリを「..\.」に設定すること
  if (g_app.openCnf(g_ini->last_open())) {
    g_app.openElf(g_elf_dir, g_elf_name);
    g_scrn->update_title_text();
  }

  thread_mgr_ = new PS2ThreadMgr();
  thread_mgr_->init();

  sema_mgr_ = new PS2SemaphoreMgr();
  sema_mgr_->init();

  // メインスレッドをファイバに変換
  g_app.set_main_fiber(ConvertThreadToFiber(NULL));

  // 実行ファイバを作成
  g_app.set_execute_fiber(g_dbg->createExecuteFiber());
  g_dbg->setupParserVars();

  // VBLスレッドを作成
  DWORD tid;
  g_vbl_thread_hdl = CreateThread(NULL, 0, vbl_proc,
                                  NULL, CREATE_SUSPENDED, &tid);
  if (g_vbl_thread_hdl == NULL) {
    return false;
  }
  SetThreadPriority(g_vbl_thread_hdl, THREAD_PRIORITY_NORMAL);
  ResumeThread(g_vbl_thread_hdl);

  try {
    while (!g_app.done()) {
      MSG msg;
      while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        if (TranslateAccelerator(g_scrn->wnd_->getHwnd(), g_scrn->accel_, &msg)) continue;

        if (g_dbg == NULL || g_dbg->preTranslateMessage(&msg) == false) {
          if (msg.message == WM_QUIT) {
            g_app.set_done(true);
          } else {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
          }
        }
      }
      
      // 実行中ならば実行ファイバに戻る
      if (g_dbg->command_id() != -1) {
        SwitchToFiber(thread_mgr_->cur_thread()->fiber());
        continue;
      }
      Sleep(1);
    }
  } catch (char* msg) {
    g_scrn->errMsgBox(msg);
  }

  // VBLスレッドを削除
  if (g_vbl_thread_hdl) {
    // VBLスレッド終了するまで待機
    g_vbl_thread_end = true;
    WaitForSingleObject(g_vbl_thread_hdl, 2000);
    
    // VBLスレッドを破棄
    CloseHandle(g_vbl_thread_hdl);
    g_vbl_thread_hdl = NULL;
  }

  // 実行ファイバを削除
  DeleteFiber(g_app.execute_fiber());
  // 元に戻す
  ConvertFiberToThread();

  g_dbg->cleanupParserVars();
  delete g_dbg;
  g_dbg = NULL;

  if (sema_mgr_) {
    sema_mgr_->clean();
    delete sema_mgr_;
    sema_mgr_ = NULL;
  }

  if (thread_mgr_) {
    thread_mgr_->clean();
    delete thread_mgr_;
    thread_mgr_ = NULL;
  }

  g_app.cleanSetting();

  delete g_scrn;

  g_dev_emu->clean();
  delete g_dev_emu;

  g_ini->write();
  delete g_ini;

  delete g_memory;
  delete g_cpu;

  ds_util::deleteArgString(argc, argv);

  return 0;
}
#else // defined _UNIT_TEST defined _DEBUGGER
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR szCmdLine, int iCmdShow) {
  // メモリリーク検出
  _CrtSetDbgFlag(_CRTDBG_ALLOC_MEM_DF | _CRTDBG_LEAK_CHECK_DF);

  GetCurrentDirectory(_MAX_PATH, g_base_dir);

  char** argv;
  int argc = ds_util::createArgString(szCmdLine, argv);

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();

  // iniファイル読み込み
  g_ini = new IniFileInfo();
  g_ini->read();

  g_scrn = new Window();
  SetFocus(g_scrn->hwnd());

  g_dev_emu = new Device();

  // デバッグ時は作業ディレクトリを「..\.」に設定すること
  if (g_app.openCnf(g_ini->last_open())) {
    g_app.openElf(g_elf_dir, g_elf_name);
    g_scrn->update_title_text();
  }

  thread_mgr_ = new PS2ThreadMgr();
  thread_mgr_->init();

  sema_mgr_ = new PS2SemaphoreMgr();
  sema_mgr_->init();

  // パッチファイルを適用
  s8 patch_path[_MAX_PATH];
  sprintf(patch_path, "data/%s/%s.pat", g_elf_name, g_elf_name);
  g_app.applyBinaryPatch(patch_path);

  DWORD tid;
  // VBLスレッドを作成
  g_vbl_thread_hdl = CreateThread(NULL, 0, vbl_proc,
                                  NULL, CREATE_SUSPENDED, &tid);
  if (g_vbl_thread_hdl == NULL) {
    return false;
  }
  SetThreadPriority(g_vbl_thread_hdl, THREAD_PRIORITY_NORMAL);
  ResumeThread(g_vbl_thread_hdl);

  // 実行スレッドを作成
  g_execute_thread_hdl = CreateThread(NULL, 0, execute_proc, NULL, CREATE_SUSPENDED, &tid);
  if (g_execute_thread_hdl == NULL) {
    return false;
  }
  SetThreadPriority(g_execute_thread_hdl, THREAD_PRIORITY_ABOVE_NORMAL);
  ResumeThread(g_execute_thread_hdl);

  while (!g_app.done()) {
    // デバイスがロストしていたら復元
    g_dev_emu->restoreDevice();

    if (g_scrn->active() ||
        g_app.setting_file_mgr() &&
        g_app.setting_file_mgr()->get_data()->input_in_background()) {
      bool alt    = !!(GetAsyncKeyState(VK_MENU)   & 0x8000);
      bool enter  = !!(GetAsyncKeyState(VK_RETURN) & 0x0001);
      bool escape = !!(GetAsyncKeyState(VK_ESCAPE) & 0x0001);
      if (alt && enter) {
        g_dev_emu->toggleScreenMode();
      } else if (escape) {
        // アクセラレータなくてもESCAPEで終了できるように
        PostQuitMessage(0);
      }
    }
    MSG msg;
    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
      if (msg.message == WM_QUIT) {
        g_app.set_done(true);
      } else {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
      }
    }
    Sleep(10);
  }

  // ウインドウモードに戻してから終了する
  g_dev_emu->changeWindowMode();

  // 実行スレッドを削除
  if (g_execute_thread_hdl) {
    // 実行スレッド終了するまで待機
    g_execute_thread_end = true;
    WaitForSingleObject(g_execute_thread_hdl, 2000);
    
    // 実行スレッドを破棄
    CloseHandle(g_execute_thread_hdl);
    g_execute_thread_hdl = NULL;
  }

  // VBLスレッドを削除
  if (g_vbl_thread_hdl) {
    // VBLスレッド終了するまで待機
    g_vbl_thread_end = true;
    WaitForSingleObject(g_vbl_thread_hdl, 2000);
    
    // VBLスレッドを破棄
    CloseHandle(g_vbl_thread_hdl);
    g_vbl_thread_hdl = NULL;
  }

  if (sema_mgr_) {
    sema_mgr_->clean();
    delete sema_mgr_;
    sema_mgr_ = NULL;
  }

  if (thread_mgr_) {
    thread_mgr_->clean();
    delete thread_mgr_;
    thread_mgr_ = NULL;
  }

  g_app.cleanSetting();

  delete g_scrn;

  g_dev_emu->clean();
  delete g_dev_emu;

  g_ini->write();
  delete g_ini;

  delete g_memory;
  delete g_cpu;

  ds_util::deleteArgString(argc, argv);

  return 0;
}
#endif // defined _UNIT_TEST

void App::openElf(s8* p_elf_dir, s8* p_elf_name) {
#if defined _DEBUGGER
  if (g_dbg) {
    g_dbg->clear();
  }
#endif
  g_memory->clear();
  g_cpu->init();

  s8 elf_path[1024];
  sprintf(elf_path, "%s/%s", g_elf_dir, g_elf_name);

  u32 entrypoint = Elf::load(elf_path, &g_elf_code_addr, &g_elf_code_size,
    (u8*)VA2PA(Memory::kMainMemoryBase), Memory::kMainMemorySize);
  if (entrypoint == 0xffffffff) throw "ファイルの読み込みに失敗しました！";

  // 拡張コード領域も含めたコード領域全体のアドレスとサイズを算出
  const int extend_code_size = 0x80000;
  if (g_elf_code_addr >= extend_code_size) {
    // 本来のコードセクションの前に拡張コード領域の分を付加する
    // 通常コードセクションは0x100000～のはず
    g_code_addr = g_elf_code_addr - extend_code_size;
    g_code_size = g_elf_code_size + extend_code_size;
  } else {
    g_code_addr = g_elf_code_addr;
    g_code_size = g_elf_code_size;
  }

  g_cpu->m_pc = entrypoint;
  g_cpu->create_recompile_cache();

  g_app.cleanSetting();
  g_app.initSetting();

  g_dev_emu->clean();
  g_dev_emu->init();

#if !defined _DEBUGGER
  if (setting_file_mgr_->get_data()->graph_fullscreen() != SettingFileData::kDisplayMode_Window) {
    g_dev_emu->toggleScreenMode();
  }
#endif // defined _DEBUGGER

#if defined _DEBUGGER
  if (g_dbg) {
    // elfのコードセクションのみ
    g_dbg->module_insert(0,
      new ModuleInfo(
        g_elf_name,
        g_elf_code_addr,
        g_elf_code_size,
        entrypoint,
        false));
    // elfのコードセクション+0x080000-0x100000を拡張領域として利用する
    g_dbg->module_insert(1,
      new ModuleInfo(
        "User Code",
        g_code_addr,
        g_code_size,
        entrypoint,
        false));
    g_dbg->init(elf_path);
    g_dbg->loadAllDdb();
    g_dbg->doStaticCodeAnalyze();
  }
#endif
}

bool App::openCnf(s8* p_cnf_file) {
  FILE* fp = fopen(p_cnf_file, "r");
  if (fp == NULL) return false;

  s8  elf_name[256] = "";
  s8  buf[1024];
  while (fgets(buf, 1024, fp)) {
    s8* value = buf;
    s8* label = ds_util::strdiv(value, '=', '\"');
    ds_util::strtrim(label);
    ds_util::strtrim(value);
    
    if (strcmp(label, "BOOT2") == 0 &&
        strncmp(value, "cdrom0:\\", 8) == 0) {
      strncpy_s(g_elf_name, 256, &value[8], strlen(value) - 11);
      ds_util::getParentDirectory(p_cnf_file, g_elf_dir);
      break;
    }
  }
  return true;
}

void App::initSetting() {
  setting_file_mgr_ = new SettingFileMgr();
  s8  setting_path[_MAX_PATH];
  sprintf_s(setting_path, _MAX_PATH, "%s/data/%s/%s",
            g_base_dir, g_elf_name, kSettingFileName);
  setting_file_mgr_->read(setting_path);
}

void App::cleanSetting() {
  if (setting_file_mgr_) {
    s8  setting_path[_MAX_PATH];
    sprintf_s(setting_path, _MAX_PATH, "%s/data/%s/%s",
              g_base_dir, g_elf_name, kSettingFileName);
    setting_file_mgr_->write(setting_path);

    delete setting_file_mgr_;
    setting_file_mgr_ = NULL;
  }
}

void App::applyBinaryPatch(s8* p_patch_file) {
  FILE* fp = fopen(p_patch_file, "rb");
  if (fp == NULL) return;
  
  while (1) {
    u32 addr;
    fread(&addr, 4, 1, fp);
    u32 size;
    fread(&size, 4, 1, fp);
    if (size == 0) break;
    fread((void*)VA2PA(addr), 1, size, fp);
  }
  fclose(fp);
}
