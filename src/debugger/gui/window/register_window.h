#ifndef _debugger_gui_registerwindow
#define _debugger_gui_registerwindow

#include "cpu/r5900.h"

#include <utils/ds_window.h>

class RegisterWindow {
// constant definition
public:
  enum {
#ifdef _ENV2
    kViewX = 10 + 150,
    kViewY = 70 + 40,
    kViewW = 304,
    kViewH = 633,
#else
    kViewX = 558,
    kViewY = 50,
    kViewW = 304,
    kViewH = 633,
#endif
  };

// constructor/destructor
public:
  RegisterWindow();
  ~RegisterWindow();

// override
public:
  void Init();

  void gpr_update();
  void cop0_update();
  void fpr_update();

  int reg2idc(int p_reg, int p_idx);
  int hwnd2idc(HWND p_hwnd, HWND p_parent = NULL);

// event handler
public:
  //void OnInitDialog(HWND p_hwnd);
  //void OnActivate(int p_active, int p_minimized, int p_hwnd_pre);
  //void OnCommand(int p_ctrlID, int p_notify);
  //void OnShowWindow(WPARAM p_wp, LPARAM p_lp);
  //void OnSize(WPARAM p_wp, LPARAM p_lp);
  //void OnVScroll(WPARAM p_wp, LPARAM p_lp);
  //void OnPaint(WPARAM p_wp, LPARAM p_lp);
  //void OnMouseWheel(WPARAM p_wp, LPARAM p_lp);
  //void OnLButtonDown(int p_x, int p_y);
  //void OnLButtonDblClk(int p_x, int p_y);
  //void OnRButtonDown(int p_x, int p_y);
  //void OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift);
  //void OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di);
  //void OnMenuSelect(int p_menu_id, int p_menu_flag, HMENU p_menu_handle);

  //void OnClear();

// member function
public:
  void OnPreExecute();

// static function
public:
  static LRESULT CALLBACK FrameProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp);
  static LRESULT CALLBACK GPRProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp);
  static LRESULT CALLBACK COP0Proc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp);
  static LRESULT CALLBACK FPRProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp);

// accessor
public:
  inline ds_dialog* dlg() { return m_dlg; }

// member variable
private:
  ds_dialog*  m_dlg;
  ds_dialog*  m_gpr_page;
  ds_dialog*  m_cop0_page;
  ds_dialog*  m_fpr_page;

  R5900REGS  m_save_r5900;  // 変更レジスタ比較用

  bool m_ignore_wm_command;
  bool m_user_modified;
};

#endif // _debugger_gui_registerwindow
