#include "main.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/find_binary_dialog.h"
#include "debugger/gui/window/code_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

bool FindBinaryDialog::ignore_code_section_ = true;

void FindBinaryDialog::handleInitDialogMsg(HWND p_dlg) {
  RECT rect;
  GetWindowRect(p_dlg, &rect);
  SetWindowPos(p_dlg, NULL,
    rect.left,
    rect.top - (rect.bottom - rect.top + 20),
    0, 0, SWP_NOSIZE | SWP_NOZORDER);

  SetDlgItemText(p_dlg, IDC_EDT_BINARY, "");
  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_EDT_BINARY));

  HWND chk = GetDlgItem(p_dlg, IDC_CHK_IGNORE_OUTOFREF);
  SendMessage(chk, BM_SETCHECK,
    ignore_code_section_ ? BST_CHECKED : BST_UNCHECKED, 0);

  HWND cmb = GetDlgItem(p_dlg, IDC_CMB_TYPE);
  for (int i = SendMessage(cmb, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
    SendMessage(cmb, CB_DELETESTRING, 0, 0);
  }
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Binary");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Text");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Integer(Hex)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Integer(Dec)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Float");
  SendMessage(cmb, CB_SETCURSEL, 0L, 0L);
  SendMessage(p_dlg, WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_CATEGORY, 0L);

  SetFocus(GetDlgItem(p_dlg, IDC_EDT_BINARY));

  g_dbg->clearFindItem();
}

void FindBinaryDialog::handleOK(HWND p_dlg) {
  ModuleInfo* module = g_dbg->code_window()->GetCurModule();
  if (module == NULL) return;

  char str[1024];
  GetDlgItemText(p_dlg, IDC_EDT_BINARY, str, 1024);

  s8 data[512];
  u32 size = 0;

  HWND cmb = GetDlgItem(p_dlg, IDC_CMB_TYPE);
  int selidx = SendMessage(cmb, CB_GETCURSEL, 0L, 0L);
  switch (selidx) {
    case 0: // Binary
      size = ds_util::str2hex(data, 512, str);
      break;
    case 1: // Text
      size = strlen(str);
      memcpy(data, str, size);
      break;
    case 2: // Integer(Hex)
      break;
    case 3: // Integer(Dec)
      break;
    case 4: // Float
      break;
  }

  HWND chk = GetDlgItem(p_dlg, IDC_CHK_IGNORE_OUTOFREF);
  ignore_code_section_ = SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (size == 0) {
    EndDialog(p_dlg, IDOK);
    return;
  }

  bool over = false;
  // ÉÅÉCÉìÉÅÉÇÉäëSëÃ
  for (u32 i = 0; i < 0x2000001 - size; i++) {
    if (ignore_code_section_ &&
        i >= module->addr() && i < module->addr() + module->size()) {
      continue;
    }

    if (memcmp((const void*)VA2PA(i), data, size) != 0) continue;

    if (g_dbg->finditem_ary_.size() > kFindInstMaxCount) {
      over = true;
      break;
    }

    FindItem* finditem = new FindItem(i, FindItem::kLocation_Memory);
    s8 str[1024];
    sprintf(str, "%08x", i);
    finditem->set_text(str);
    g_dbg->finditem_ary_.push_back(FindItemAryValue(finditem));
  }
  if (over) {
    MessageBox(p_dlg, kMsg_FindItemOver, NULL, NULL);
  }
  if (g_dbg->finditem_ary_.size() == 0) {
    MessageBox(p_dlg, kMsg_FindItemNone, NULL, NULL);
  }
  EndDialog(p_dlg, IDOK);
}

void FindBinaryDialog::handleCancel(HWND p_dlg) {
  EndDialog(p_dlg, IDCANCEL);
}

LRESULT CALLBACK FindBinaryDialog::dlgProc(HWND hDlg,
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
