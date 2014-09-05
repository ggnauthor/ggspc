/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/addr_jump_dialog.h"
#include "debugger/gui/window/code_window.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

vector<u32> AddrJumpDialog::addr_jump_log_;

bool AddrJumpDialog::open(HWND p_parent_hwnd) {
  bool ret = MODALDIALOG(
               IDD_ADDRJUMP_DIALOG,
               p_parent_hwnd,
               dlgProc) == IDOK;
  SetFocus(p_parent_hwnd);
  SetActiveWindow(p_parent_hwnd);
  return ret;
}

void AddrJumpDialog::handleInitDialogMsg(HWND p_dlg) {
  RECT rect;
  GetWindowRect(p_dlg, &rect);
  SetWindowPos(p_dlg, NULL,
    rect.left,
    rect.top - (rect.bottom - rect.top + 20),
    0, 0, SWP_NOSIZE | SWP_NOZORDER);

  HWND cmb = GetDlgItem(p_dlg, IDC_CMB_ADDRESS);
  for (vector<u32>::iterator itr = addr_jump_log_.begin();
      itr != addr_jump_log_.end();
      ++itr) {
    char str[256];
    sprintf(str, "%08x", *itr);
    SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)str);
  }
  SendMessage(cmb, CB_SETCURSEL, 0L, 0L);
  g_dbg->setFontToFixed(cmb);
  
  SetFocus(GetDlgItem(p_dlg, IDC_CMB_ADDRESS));
}

void AddrJumpDialog::handleOK(HWND p_dlg) {
  char str[1024];
  GetDlgItemText(p_dlg, IDC_CMB_ADDRESS, str, 1023);

  u32 va = strtoul(str, NULL, 16);
  g_dbg->code_window()->AutoJumpModule(va);
  g_dbg->code_window()->SelectAddress(va);

  // óöóÇ…í«â¡Ç∑ÇÈÅBä˘Ç…Ç†ÇÍÇŒàÍíUçÌèúÇµÇƒçƒí«â¡
  for (vector<u32>::iterator itr = addr_jump_log_.begin();
      itr != addr_jump_log_.end();
      ++itr) {
    if (*itr == va) {
      addr_jump_log_.erase(itr);
    }
  }
  addr_jump_log_.insert(addr_jump_log_.begin(), vector<u32>::value_type(va));

  EndDialog(p_dlg, IDOK);
}

void AddrJumpDialog::handleCancel(HWND p_dlg) {
  EndDialog(p_dlg, IDCANCEL);
}

LRESULT CALLBACK AddrJumpDialog::dlgProc(HWND hDlg,
                                         UINT message,
                                         WPARAM wParam,
                                         LPARAM lParam) {
  switch (message) {
    case WM_INITDIALOG:
      handleInitDialogMsg(hDlg);
      return FALSE;
    case WM_DESTROY:
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK:     handleOK(hDlg); break;
        case IDCANCEL: handleCancel(hDlg); break;
      }
      return FALSE;
    default:
      return FALSE; 
  }
  return TRUE;
}
