/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/breakpoint_info_dialog.h"
#include "debugger/gui/window/breakpoint_window.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

/*---------*/
/* structs */
/*---------*/

/*--------*/
/* global */
/*--------*/
namespace {
int        g_left, g_top;
BreakPointBase* g_breakpoint = NULL;
};

/*----------*/
/* function */
/*----------*/
HWND GetBreakPointInfoDlg() {
  return FindWindow(MAKEINTRESOURCE(32770), "BreakPoint Info Dialog");
}

void BreakPointInfoDlgSetup(int p_left, int p_top, BreakPointBase* p_breakpoint) {
  g_left = p_left;
  g_top = p_top;
  g_breakpoint = p_breakpoint;
}

LRESULT CALLBACK BreakPointInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {

  assert(g_breakpoint);

  switch (message) {
  case WM_INITDIALOG: {
    SetWindowPos(hDlg, NULL, g_left, g_top, 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    char str[1024];
    sprintf(str, "%08x", g_breakpoint->va());
    SetDlgItemText(hDlg, IDC_ADDR, str);
    g_dbg->setFontToFixed(GetDlgItem(hDlg, IDC_OPCODE));

    if (g_breakpoint->type() == BreakPointBase::kType_Memory) {
      MemoryBreakPoint* memory_breakpoint = dynamic_cast<MemoryBreakPoint*>(g_breakpoint);

      sprintf(str, "%d", memory_breakpoint->length());
      SetDlgItemText(hDlg, IDC_LENGTH, str);
      g_dbg->setFontToFixed(GetDlgItem(hDlg, IDC_LENGTH));
      SendMessage(GetDlgItem(hDlg, IDC_CHK_READ), BM_SETCHECK, memory_breakpoint->read_enable() ? BST_CHECKED : BST_UNCHECKED, 0);
      SendMessage(GetDlgItem(hDlg, IDC_CHK_WRITE), BM_SETCHECK, memory_breakpoint->write_enable() ? BST_CHECKED : BST_UNCHECKED, 0);
    } else if (g_breakpoint->type() == BreakPointBase::kType_Code) {
      ShowWindow(GetDlgItem(hDlg, IDC_CHK_READ), SW_HIDE);
      ShowWindow(GetDlgItem(hDlg, IDC_CHK_WRITE), SW_HIDE);
      ShowWindow(GetDlgItem(hDlg, IDC_LENGTH), SW_HIDE);
      ShowWindow(GetDlgItem(hDlg, IDC_STA_LENGTH), SW_HIDE);
      ShowWindow(GetDlgItem(hDlg, IDC_STA_COND), SW_HIDE);
    }

    SendMessage(GetDlgItem(hDlg, IDC_CHK_ENABLE), BM_SETCHECK, g_breakpoint->enable() ? BST_CHECKED : BST_UNCHECKED, 0);

    SendMessage(GetDlgItem(hDlg, IDC_CHK_LOG), BM_SETCHECK, g_breakpoint->output_log() ? BST_CHECKED : BST_UNCHECKED, 0);

    SetDlgItemText(hDlg, IDC_CMB_EXPR, g_breakpoint->expr());

    SetFocus(GetDlgItem(hDlg, IDC_CMB_EXPR));
    return FALSE; }
  case WM_DESTROY:
    break;
  case WM_COMMAND: {
    switch (LOWORD(wParam)) {
    case IDOK: {
      char tmp[256];
      GetDlgItemText(hDlg, IDC_LENGTH, tmp, 255);
      int len = atoi(tmp);
      if (len < 1) len = 1;
      
      HWND chk;
      chk = GetDlgItem(hDlg, IDC_CHK_READ);
      bool read = SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED;

      chk = GetDlgItem(hDlg, IDC_CHK_WRITE);
      bool write = SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED;

      chk = GetDlgItem(hDlg, IDC_CHK_ENABLE);
      bool enable = SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED;

      chk = GetDlgItem(hDlg, IDC_CHK_LOG);
      bool log = SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED;

      char expr[1024];
      GetDlgItemText(hDlg, IDC_CMB_EXPR, expr, 1023);

      if (g_breakpoint->type() == BreakPointBase::kType_Memory) {
        if (!read && !write) {
          MessageBox(hDlg, "Select break condition from read, write or both.", NULL, NULL);
          return TRUE;
        }
        MemoryBreakPoint* memory_breakpoint = dynamic_cast<MemoryBreakPoint*>(g_breakpoint);
        memory_breakpoint->set_length(len);
        memory_breakpoint->set_read_enable(read);
        memory_breakpoint->set_write_enable(write);
      }
      g_breakpoint->set_enable(enable);
      g_breakpoint->set_output_log(log);
      g_breakpoint->set_expr(expr);
      EndDialog(hDlg, IDOK);
      break; }
    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;
    default:
      return FALSE;
    }
    return FALSE; }
  default:
    return FALSE;
  }
  return TRUE;
}
