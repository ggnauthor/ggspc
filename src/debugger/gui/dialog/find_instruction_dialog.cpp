#include "main.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/disasm.h"
#include "debugger/analyze_info.h"
#include "debugger/gui/dialog/find_instruction_dialog.h"
#include "debugger/gui/window/code_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

bool FindInstructionDialog::ignore_out_of_reference_ = true;

bool FindInstructionDialog::open(HWND p_parent_hwnd) {
  bool ret = MODALDIALOG(
               IDD_FINDINST_DIALOG,
               p_parent_hwnd,
               dlgProc) == IDOK;
  SetFocus(p_parent_hwnd);
  SetActiveWindow(p_parent_hwnd);
  return ret;
}

void FindInstructionDialog::handleInitDialogMsg(HWND p_dlg) {
  RECT rect;
  GetWindowRect(p_dlg, &rect);
  SetWindowPos(p_dlg, NULL,
    rect.left,
    rect.top - (rect.bottom - rect.top + 20),
    0, 0, SWP_NOSIZE | SWP_NOZORDER);

  SetDlgItemText(p_dlg, IDC_EDT_KEYWORD, "");
  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_EDT_KEYWORD));

  HWND chk = GetDlgItem(p_dlg, IDC_CHK_IGNORE_OUTOFREF);
  SendMessage(chk, BM_SETCHECK,
    ignore_out_of_reference_ ? BST_CHECKED : BST_UNCHECKED, 0);

  SetFocus(GetDlgItem(p_dlg, IDC_EDT_KEYWORD));

  g_dbg->clearFindItem();
}

void FindInstructionDialog::handleOK(HWND p_dlg) {
  ModuleInfo* module = g_dbg->code_window()->GetCurModule();
  if (module == NULL) return;

  char pattern[1024];
  GetDlgItemText(p_dlg, IDC_EDT_KEYWORD, pattern, 1024);
  ds_util::strtrim(pattern);

  HWND chk = GetDlgItem(p_dlg, IDC_CHK_IGNORE_OUTOFREF);
  ignore_out_of_reference_ =
    SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED;

  if (strlen(pattern) == 0) {
    EndDialog(p_dlg, IDOK);
    return;
  }

  bool over = false;
  for (u32 i = 0; i < module->size(); i += 4) {
    u32 va = module->addr() + i;

    AnalyzeInfoMapItr itr = g_dbg->analyze_map_.find(va);
    if (itr == g_dbg->analyze_map_.end()) continue;

    if (ignore_out_of_reference_ && itr->second.enable() == false) {
      continue;
    }

    char inst_name[1024];
    disasm_plain(va, *((u32*)VA2PA(va)), inst_name);
    if (ds_util::strmatch(inst_name, pattern) == false) continue;

    if (g_dbg->finditem_ary_.size() > kFindInstMaxCount) {
      over = true;
      break;
    }

    FindItem* finditem = new FindItem(va, FindItem::kLocation_Code);
    finditem->set_text(inst_name);
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

void FindInstructionDialog::handleCancel(HWND p_dlg) {
  EndDialog(p_dlg, IDCANCEL);
}

LRESULT CALLBACK FindInstructionDialog::dlgProc(HWND hDlg,
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
