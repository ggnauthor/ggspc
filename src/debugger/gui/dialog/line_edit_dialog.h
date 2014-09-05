#ifndef _debugger_gui_lineeditdialog
#define _debugger_gui_lineeditdialog

#include "main.h"

class LineEditDialog {
public:
  LineEditDialog() {}
  ~LineEditDialog() {}

  static bool open(HWND p_parent_hwnd);

  static void handleInitDialogMsg(HWND p_dlg);
  static void handleAssemblyChange(HWND p_dlg);
  static void handleOK(HWND p_dlg);
  static void handleCancel(HWND p_dlg);

  static LRESULT CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  // アドレス入力によるジャンプの移動履歴
  static vector<u32> addr_jump_log_;
};

#endif // _debugger_gui_lineeditdialog
