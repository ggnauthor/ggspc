/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "debugger/debugger.h"
#include "debugger/patch_entry.h"
#include "debugger/gui/dialog/patch_info_dialog.h"
#include "debugger/gui/window/patch_window.h"
#include "debugger/gui/window/code_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

PatchEntry* PatchInfoDialog::patch_ = NULL;

bool PatchInfoDialog::open(HWND p_parent_hwnd, PatchEntry* p_patch) {
  patch_ = p_patch;

  bool ret = MODALDIALOG(
               IDD_PATCHEDIT_DIALOG,
               p_parent_hwnd,
               dlgProc) == IDOK;
  SetFocus(p_parent_hwnd);
  SetActiveWindow(p_parent_hwnd);
  return ret;
}

void PatchInfoDialog::handleInitDialogMsg(HWND p_dlg) {
  RECT rect;
  GetWindowRect(p_dlg, &rect);
  SetWindowPos(p_dlg, NULL,
    rect.left,
    rect.top - (rect.bottom - rect.top + 20),
    0, 0, SWP_NOSIZE | SWP_NOZORDER);

  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_ADDR));
  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_EDT_DATA));
  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_LENGTH));

  char str[1024];
  sprintf(str, "%08x", patch_->va());
  SetDlgItemText(p_dlg, IDC_ADDR, str);

  sprintf(str, "%d", patch_->size());
  SetDlgItemText(p_dlg, IDC_LENGTH, str);

  SendMessage(GetDlgItem(p_dlg, IDC_CHK_ENABLE), BM_SETCHECK, patch_->state() ? BST_CHECKED : BST_UNCHECKED, 0);

  SetDlgItemText(p_dlg, IDC_EDT_NAME, patch_->name());

  ds_util::hex2str(patch_->new_data(), patch_->size(), str);
  SetDlgItemText(p_dlg, IDC_EDT_DATA, str);

  SetFocus(GetDlgItem(p_dlg, IDC_EDT_DATA));
}

void PatchInfoDialog::handleOK(HWND p_dlg) {
  int cmd_id = g_dbg->command_id();
  // リコンパイルコードが破棄されるので一旦止める
  if (cmd_id != -1) {
    SendMessage(g_dbg->code_window()->dlg()->getHwnd(), WM_COMMAND, ID_DEBUG_PAUSE, 0);
  }

  patch_->SetDisable();

  char str[1024];
  GetDlgItemText(p_dlg, IDC_EDT_NAME, str, 1024);
  patch_->set_name(str);

  GetDlgItemText(p_dlg, IDC_ADDR, str, 1024);
  patch_->set_addr(strtol(str, NULL, 16));

  GetDlgItemText(p_dlg, IDC_EDT_DATA, str, 1024);
  int size = strlen(str) / 2;
  s8* buf = new s8[size];
  ds_util::str2hex(buf, size, str, false);
  patch_->set_size(size);
  patch_->update(0, size, buf);
  delete[] buf;

  bool state = SendMessage(GetDlgItem(p_dlg, IDC_CHK_ENABLE), BM_GETCHECK, 0, 0) == BST_CHECKED;
  if (state) {
    patch_->SetEnable();
  }

  // 再開
  if (cmd_id != -1) {
    SendMessage(g_dbg->code_window()->dlg()->getHwnd(), WM_COMMAND, cmd_id, 0);
  }
  EndDialog(p_dlg, IDOK);
}

void PatchInfoDialog::handleCancel(HWND p_dlg) {
  EndDialog(p_dlg, IDCANCEL);
}

void PatchInfoDialog::handleChangeEditData(HWND p_dlg) {
  char str[256];
  GetDlgItemText(p_dlg, IDC_EDT_DATA, str, 256);

  int size = strlen(str) / 2;
  _itoa(size, str, 10);
  SetDlgItemText(p_dlg, IDC_LENGTH, str);
}

LRESULT CALLBACK PatchInfoDialog::dlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_INITDIALOG:
      handleInitDialogMsg(hDlg);
      return FALSE;
    case WM_DESTROY:
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
        case IDOK:
          handleOK(hDlg);
          break;
        case IDCANCEL:
          handleCancel(hDlg);
          break;
        case IDC_EDT_DATA:
          if (HIWORD(wParam) == EN_CHANGE) {
            handleChangeEditData(hDlg);
          }
          break;
      }
      return FALSE;
    default:
      return FALSE;
  }
  return TRUE;
}
