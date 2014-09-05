/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/asm.h"
#include "debugger/disasm.h"
#include "debugger/patch_entry.h"
#include "debugger/gui/dialog/line_edit_dialog.h"
#include "debugger/gui/window/code_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

bool LineEditDialog::open(HWND p_parent_hwnd) {
  bool ret = MODALDIALOG(
               IDD_LINEEDIT_DIALOG,
               p_parent_hwnd,
               dlgProc) == IDOK;
  SetFocus(p_parent_hwnd);
  SetActiveWindow(p_parent_hwnd);
  return ret;
}

void LineEditDialog::handleInitDialogMsg(HWND p_dlg) {
  s8 str[1024];

  ModuleInfo* info = g_dbg->code_window()->GetCurModule();
  if (info == NULL) return;

  RECT rect;
  GetWindowRect(p_dlg, &rect);
  SetWindowPos(p_dlg, NULL, rect.left, rect.top - (rect.bottom - rect.top + 20), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

  u32 va = info->addr() + g_dbg->code_window()->select() * 4;
  u32 opcode = *((u32*)VA2PA(va));

  sprintf(str, "%08x", opcode);
  SetDlgItemText(p_dlg, IDC_OPCODE, str);
  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_OPCODE));

  disasm_plain(va, opcode, str);
  SetDlgItemText(p_dlg, IDC_ASM, str);
  g_dbg->setFontToFixed(GetDlgItem(p_dlg, IDC_ASM));

  Comment* comment = g_dbg->find_comment(va);
  SetDlgItemText(p_dlg, IDC_COMMENT, comment ? comment->text() : "");

  Label* label = g_dbg->find_codelabel(va);
  SetDlgItemText(p_dlg, IDC_LABEL, label ? label->text() : "");

  SetFocus(GetDlgItem(p_dlg, IDC_LABEL));
}

void LineEditDialog::handleAssemblyChange(HWND p_dlg) {
  ModuleInfo* info = g_dbg->code_window()->GetCurModule();
  if (info == NULL) return;

  u32 va = info->addr() + g_dbg->code_window()->select() * 4;

  s8 str[1024];
  s8 err[1024] = "";
  GetDlgItemText(p_dlg, IDC_ASM, str, 255);
  u32 opcode = asmR5900(str, va, err);
  if (err[0] != '\0') {
    SetDlgItemText(p_dlg, IDC_MESSAGE, err);
  } else {
    sprintf(str, "%08x", opcode);
    SetDlgItemText(p_dlg, IDC_OPCODE, str);
    SetDlgItemText(p_dlg, IDC_MESSAGE, "");
  }
}

void LineEditDialog::handleOK(HWND p_dlg) {
  s8 str[1024];

  ModuleInfo* info = g_dbg->code_window()->GetCurModule();
  if (info == NULL) return;

  u32 va = info->addr() + g_dbg->code_window()->select() * 4;

  // オペコードを反映
  GetDlgItemText(p_dlg, IDC_OPCODE, str, 255);
  u32 opcode = strtoul(str, NULL, 16);
  if (*((u32*)VA2PA(va)) != opcode) {
    int cmd_id = g_dbg->command_id();
    // リコンパイルコードが破棄されるので一旦止める
    if (cmd_id != -1) {
      SendMessage(g_dbg->code_window()->dlg()->getHwnd(), WM_COMMAND, ID_DEBUG_PAUSE, 0);
    }

    PatchEntry* patch = g_dbg->findPatch(va, 4, false);

    if (patch) {
      patch->SetDisable();

      // パッチを更新
      patch->update(va - patch->va(), 4, reinterpret_cast<const s8*>(&opcode));

      patch->SetEnable();
    } else {
      // パッチを作成
      g_dbg->addPatch(new PatchEntry(va, 4, true, reinterpret_cast<const s8*>(&opcode)));
    }
    g_dbg->doStaticCodeAnalyze();

    // 再開
    if (cmd_id != -1) {
      SendMessage(g_dbg->code_window()->dlg()->getHwnd(), WM_COMMAND, cmd_id, 0);
    }
  }

  // コメントを反映
  GetDlgItemText(p_dlg, IDC_COMMENT, str, 255);
  str[255] = '\0';
  ds_util::strtrim(str);

  Comment* comment = g_dbg->find_comment(va);
  if (comment == NULL && str[0] != '\0') {
    // 新規追加
    g_dbg->add_comment(new Comment(g_dbg->get_current_ddb(), va, str));
  } else if (comment && str[0] != '\0') {
    // 更新
    comment->set_text(str);
  } else if (comment && str[0] == '\0') {
    // 削除
    int idx = g_dbg->get_comment_idx(comment->va());
    g_dbg->delete_comment(idx);
  }

  // ラベルを反映
  GetDlgItemText(p_dlg, IDC_LABEL, str, 255);
  str[255] = '\0';
  ds_util::strtrim(str);

  Label* label = g_dbg->find_codelabel(va);
  if (label == NULL && str[0] != '\0') {
    // 新規追加
    g_dbg->add_codelabel(new Label(g_dbg->get_current_ddb(), va, str));
  } else if (label && str[0] != '\0') {
    // 更新
    label->set_text(str);
  } else if (label && str[0] == '\0') {
    // 削除
    int idx = g_dbg->get_codelabel_idx(label->va());
    g_dbg->delete_codelabel(idx);
  }
  EndDialog(p_dlg, IDOK);
}

void LineEditDialog::handleCancel(HWND p_dlg) {
  EndDialog(p_dlg, IDCANCEL);
}

LRESULT CALLBACK LineEditDialog::dlgProc(HWND hDlg,
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
        case IDC_ASM:
          if (HIWORD(wParam) == EN_CHANGE) {
            handleAssemblyChange(hDlg);
          }
          break;
        case IDOK:
          handleOK(hDlg);
          break;
        case IDCANCEL:
          handleCancel(hDlg);
          break;
      }
      return FALSE;
    default:
      return FALSE; 
  }
  return TRUE;
}
