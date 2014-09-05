#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/ddb_priority_dialog.h"
#include "debugger/gui/window/code_window.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

LRESULT CALLBACK SetCurDDBDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_INITDIALOG: {
      HWND cmb = GetDlgItem(hDlg, IDC_CMB_DDBFILE);
      for (u32 i = 0; i < g_dbg->ddb_ary_.size(); i++) {
        SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)g_dbg->ddb_ary_[i]->file_name());
      }
      SendMessage(cmb, CB_SETCURSEL, g_dbg->cur_ddb_idx_, 0L);
      g_dbg->setFontToFixed(cmb);
      
      SetFocus(GetDlgItem(hDlg, IDC_CMB_DDBFILE));
      return FALSE; }
    case WM_DESTROY:
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDOK: {
        HWND cmb = GetDlgItem(hDlg, IDC_CMB_DDBFILE);
        int selidx = SendMessage(cmb, CB_GETCURSEL, 0, 0);

        g_dbg->selectDdb(selidx);

        EndDialog(hDlg, IDOK);
        break; }
      case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        break;
      default:
        return FALSE;
      }
      return FALSE;
    //case WM_KEYDOWN:
    //  if (wParam == VK_ESCAPE) {
    //    EndDialog(hDlg, IDCANCEL);
    //  }
    //  break;
    default:
      return FALSE; 
  }
  return TRUE;
}
