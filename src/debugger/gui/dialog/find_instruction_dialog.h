#ifndef _debugger_gui_findinstdialog
#define _debugger_gui_findinstdialog

#include "main.h"

class FindInstructionDialog {
  enum { kFindInstMaxCount = 5000 };

public:
  FindInstructionDialog() {}
  ~FindInstructionDialog() {}

  static bool open(HWND p_parent_hwnd);

  static void handleInitDialogMsg(HWND p_dlg);
  static void handleOK(HWND p_dlg);
  static void handleCancel(HWND p_dlg);

  static LRESULT CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  static bool ignore_out_of_reference_;
};

#endif // _debugger_gui_findinstdialog
