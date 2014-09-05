#ifndef _debugger_gui_findbinarydialog
#define _debugger_gui_findbinarydialog

#include "main.h"

class FindBinaryDialog {
  enum { kFindInstMaxCount = 5000 };

public:
  FindBinaryDialog() {}
  ~FindBinaryDialog() {}

  static void handleInitDialogMsg(HWND p_dlg);
  static void handleOK(HWND p_dlg);
  static void handleCancel(HWND p_dlg);

  static LRESULT CALLBACK dlgProc(
    HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  static bool ignore_code_section_;
};

#endif // _debugger_gui_findbinarydialog
