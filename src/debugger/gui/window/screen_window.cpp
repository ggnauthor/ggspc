#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <mmsystem.h>

#include "main.h"
#include "utils/ini.h"
#include "device/device.h"
#include "debugger/debugger.h"
#include "debugger/gui/window/screen_window.h"
#include "debugger/gui/window/breakpoint_window.h"
#include "debugger/gui/window/register_window.h"
#include "debugger/gui/window/reference_window.h"
#include "debugger/gui/dialog/new_ddb_dialog.h"
#include "debugger/gui/dialog/set_cur_ddb_dialog.h"
#include "debugger/gui/dialog/ddb_priority_dialog.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

ScreenWindow* g_scrn = NULL;

extern IniFileInfo*  g_ini;

LRESULT CALLBACK wnd_proc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  ds_window* dlg = (ds_window*)GetWindowLong(p_hwnd, GWL_USERDATA);

  ScreenWindow* scrnwin = dlg ? (ScreenWindow*)dlg->getUserData() : NULL;

  switch (p_msg) {
  case WM_CLOSE:
    DestroyWindow(p_hwnd);
    break;
  case WM_DESTROY:
    PostQuitMessage(0);
    break;
  case WM_ACTIVATEAPP:
    if (scrnwin) {
      scrnwin->active_ = (p_wp & 0xFFFF) != WA_INACTIVE;
      if (g_dbg && scrnwin->active_) {
        g_dbg->foreAllWindow();
      }
      g_dev_emu->activate();
    }
    break;
  case WM_ACTIVATE:
    // モーダルダイアログがあったら制御を切り替えない
    if ((p_wp & 0xffff) != WA_INACTIVE) {
      HWND hwnd = g_dbg->findModalDialog();
      if (hwnd) {
        SetActiveWindow(hwnd);
        break;
      }
    }
    break;
  case WM_COMMAND:
    switch (LOWORD(p_wp)) {
    case ID_FILE_OPENELF: {
      ds_filewindow  filewindow;
      if (filewindow.openfile("ps2 boot files\0SYSTEM.CNF\0\0")) {
        if (g_app.openCnf(filewindow.getSelectFileName())) {
          strcpy(g_ini->last_open(), filewindow.getSelectFileName());
          g_app.openElf(g_elf_dir, g_elf_name);
          g_scrn->update_title_text();
        }
      }
    } break;
    case ID_FILE_NEWDDB:
      if (dlg) {
        s8* ddb_name = (s8*)MODALDIALOG(IDD_NEWDDB_DIALOG, dlg->getHwnd(), NewDDBDlgProc);
        if (ddb_name) {
          SetFocus(dlg->getHwnd());
          SetActiveWindow(dlg->getHwnd());
          g_dbg->newDdb(ddb_name);
          scrnwin->update_title_text();
          g_dbg->reference_window()->update_list();
          g_dbg->reference_window()->ScrollTo(0);
          g_dbg->drawAllWindow();
        }
      }
      break;
    case ID_FILE_SETCURDDB:
      if (dlg) {
        bool result = MODALDIALOG(IDD_SETCURDDB_DIALOG,
                                  dlg->getHwnd(),
                                  SetCurDDBDlgProc) == IDOK;
        SetFocus(dlg->getHwnd());
        SetActiveWindow(dlg->getHwnd());
        scrnwin->update_title_text();
        g_dbg->breakpoint_window()->update_list();
        g_dbg->breakpoint_window()->ScrollTo(0);
        g_dbg->reference_window()->update_list();
        g_dbg->reference_window()->ScrollTo(0);
        g_dbg->drawAllWindow();
      }
      break;
    case ID_FILE_DDBPRIO:
      if (dlg) {
        bool result = MODALDIALOG(IDD_SELDDB_DIALOG,
                                  dlg->getHwnd(),
                                  DDBPriorityDlgProc) == IDOK;
        SetFocus(dlg->getHwnd());
        SetActiveWindow(dlg->getHwnd());
        scrnwin->update_title_text();
        g_dbg->reference_window()->update_list();
        g_dbg->reference_window()->ScrollTo(0);
        g_dbg->drawAllWindow();
      }
      break;
    case ID_FILE_SAVEDDB:
      g_dbg->saveDdb();
      g_cpu->save_recompile_hint();
      break;
    case ID_FILE_EXIT:
      PostQuitMessage(0);
      break;
    case ID_WINDOW_ACTUAL_SIZE_SCREEN:
      g_dbg->set_actual_size_screen(!g_dbg->actual_size_screen());
      if (g_dbg->actual_size_screen()) {
        g_scrn->width_ = ScreenWindow::VIEW_ACTW;
        g_scrn->height_ = ScreenWindow::VIEW_ACTH;
      } else {
        g_scrn->width_ = ScreenWindow::VIEW_W;
        g_scrn->height_ = ScreenWindow::VIEW_H;
      }
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->actual_size_screen() ? MF_CHECKED : MF_UNCHECKED);
      g_scrn->wnd_->size(g_scrn->width_, g_scrn->height_ + GetSystemMetrics(SM_CYMENU));
      break;
    case ID_LOG_CPU:
      g_dbg->set_enable_log_cpu(!g_dbg->enable_log_cpu());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_cpu() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_GS:
      g_dbg->set_enable_log_gs(!g_dbg->enable_log_gs());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_gs() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_MEMORY:
      g_dbg->set_enable_log_mem(!g_dbg->enable_log_mem());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_mem() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_ELF:
      g_dbg->set_enable_log_elf(!g_dbg->enable_log_elf());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_elf() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_DMA:
      g_dbg->set_enable_log_dma(!g_dbg->enable_log_dma());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_dma() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_RECOMPILER:
      g_dbg->set_enable_log_rec(!g_dbg->enable_log_rec());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_rec() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_APP:
      g_dbg->set_enable_log_app(!g_dbg->enable_log_app());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_app() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_DDB:
      g_dbg->set_enable_log_ddb(!g_dbg->enable_log_ddb());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_ddb() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_SYMBOL:
      g_dbg->set_enable_log_symbol(!g_dbg->enable_log_symbol());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_symbol() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_ANALYZE:
      g_dbg->set_enable_log_analyze(!g_dbg->enable_log_analyze());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_analyze() ? MF_CHECKED : MF_UNCHECKED);
      break;
    case ID_LOG_GGS:
      g_dbg->set_enable_log_ggs(!g_dbg->enable_log_ggs());
      CheckMenuItem(GetMenu(p_hwnd), LOWORD(p_wp), g_dbg->enable_log_ggs() ? MF_CHECKED : MF_UNCHECKED);
      break;
    }
    break;
  }
  return DefWindowProc(p_hwnd, p_msg, p_wp, p_lp);
}

ScreenWindow::ScreenWindow(int p_x, int p_y, int p_width, int p_height) {
  width_ = p_width;
  height_ = p_height;
  wnd_ = new ds_window(APPNAME, (WNDPROC)wnd_proc, width_, height_);

  HMENU menu = LoadMenu(wnd_->getInstance(), MAKEINTRESOURCE(IDR_MENU_SCREEN));
  SetMenu(wnd_->getHwnd(), menu);

  accel_ = LoadAccelerators(wnd_->getInstance(), MAKEINTRESOURCE(IDR_ACC_SCREEN));
  assert(accel_);
  wnd_->move(p_x, p_y);
  wnd_->size(p_width, height_ + GetSystemMetrics(SM_CYMENU));
  wnd_->setUserData((DWORD)this);
  wnd_->show();

  active_ = true;

  // タイマー精度向上
  timeBeginPeriod(1);
}

ScreenWindow::~ScreenWindow() {
  // タイマー精度向上解除
  timeEndPeriod(1);

  delete wnd_;
}

bool ScreenWindow::preTranslateMessage(MSG* p_msg) {
  return false;
}

void ScreenWindow::draw() {
}

void ScreenWindow::update_title_text() {
  s8 title[1024];
  if (g_dbg && g_dbg->get_current_ddb_name()) {
    sprintf(title, "%s - [%s]", APPNAME, g_dbg->get_current_ddb_name());
  } else {
    sprintf(title, "%s - (%s)", APPNAME, kText_TitleStr_NoDDBFile);
  }
  SetWindowText(wnd_->getHwnd(), title);
}

void ScreenWindow::errMsgBox(const s8* p_fmt, ...) {
	va_list ap;
	va_start(ap, p_fmt);
	
	s8 buf[1024];
	vsprintf(buf, p_fmt, ap);
  MessageBox(wnd_->getHwnd(), buf, NULL, NULL);

	va_end(ap);
}
