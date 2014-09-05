#ifndef _debugger_gui_patchinfodialog
#define _debugger_gui_patchinfodialog

#include "main.h"

class PatchInfoDialog {
public:
  PatchInfoDialog() {}
  ~PatchInfoDialog() {}

  static bool open(HWND p_parent_hwnd, PatchEntry* p_patch);

  static void handleInitDialogMsg(HWND p_dlg);
  static void handleOK(HWND p_dlg);
  static void handleCancel(HWND p_dlg);
  static void handleChangeEditData(HWND p_dlg);

  static LRESULT CALLBACK dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

private:
  static PatchEntry* patch_;
};

//void PatchInfoDlgSetup(int p_left, int p_top, PatchEntry* p_patch);
LRESULT CALLBACK PatchInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif // _debugger_gui_patchinfodialog
