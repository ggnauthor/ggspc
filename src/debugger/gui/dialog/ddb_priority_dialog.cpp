#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/ddb_priority_dialog.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/dialog/new_ddb_dialog.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

LRESULT CALLBACK DDBPriorityDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
    case WM_INITDIALOG: {
      HWND lst = GetDlgItem(hDlg, IDC_LST_DDBFILE);
      for (u32 i = 0; i < g_dbg->ddb_ary_.size(); i++) {
        SendMessage(lst, LB_ADDSTRING, 0, (LPARAM)g_dbg->ddb_ary_[i]->file_name());
      }
      SendMessage(lst, LB_SETCURSEL, g_dbg->cur_ddb_idx_, 0L);
      g_dbg->setFontToFixed(lst);
      
      SetFocus(GetDlgItem(hDlg, IDC_LST_DDBFILE));
      return FALSE; }
    case WM_DESTROY:
      break;
    case WM_COMMAND:
      switch (LOWORD(wParam)) {
      case IDOK: {
        HWND lst = GetDlgItem(hDlg, IDC_LST_DDBFILE);
        s32 count = SendMessage(lst, LB_GETCOUNT, 0, 0);
        s8 str[1024];
        for (s32 i = count - 1; i >= 0; i--) {
          SendMessage(lst, LB_GETTEXT, i, (LPARAM)str);
          s32 idx = -1;
          for (u32 j = 0; j < g_dbg->ddb_ary_.size(); j++) {
            if (strcmp(g_dbg->ddb_ary_[j]->file_name(), str) == 0) {
              idx = j;
              break;
            }
          }
          if (idx >= 0) {
            g_dbg->ddb_ary_[idx]->setPriority(i);
          }
        }
        // ƒŠƒXƒg‚Ì‡‚Å—Dæ“x‚ðÄÝ’è
        g_dbg->orderByPriority();
        EndDialog(hDlg, IDOK);
        break; }
      case IDCANCEL:
        EndDialog(hDlg, IDCANCEL);
        break;
      case IDC_BTN_UP: {
        HWND lst = GetDlgItem(hDlg, IDC_LST_DDBFILE);
        s8 str[1024];
        s32 cur_idx = SendMessage(lst, LB_GETCURSEL, 0, 0);
        s32 count = SendMessage(lst, LB_GETCOUNT, 0, 0);
        if (count > 1) {
          SendMessage(lst, LB_GETTEXT, cur_idx, (LPARAM)str);
          SendMessage(lst, LB_DELETESTRING, cur_idx, 0);
          if (cur_idx > 0) cur_idx--;
          SendMessage(lst, LB_INSERTSTRING, cur_idx, (LPARAM)str);
          SendMessage(lst, LB_SETCURSEL, cur_idx, 0);
        }
      } break;
      case IDC_BTN_DOWN: {
        HWND lst = GetDlgItem(hDlg, IDC_LST_DDBFILE);
        s8 str[1024];
        s32 cur_idx = SendMessage(lst, LB_GETCURSEL, 0, 0);
        s32 count = SendMessage(lst, LB_GETCOUNT, 0, 0);
        if (count > 1) {
          SendMessage(lst, LB_GETTEXT, cur_idx, (LPARAM)str);
          SendMessage(lst, LB_DELETESTRING, cur_idx, 0);
          if (cur_idx < count - 1) cur_idx++;
          SendMessage(lst, LB_INSERTSTRING, cur_idx, (LPARAM)str);
          SendMessage(lst, LB_SETCURSEL, cur_idx, 0);
        }
      } break;
      case IDC_BTN_DEL: {
        HWND lst = GetDlgItem(hDlg, IDC_LST_DDBFILE);
        s32 cur_idx = SendMessage(lst, LB_GETCURSEL, 0, 0);
        SendMessage(lst, LB_DELETESTRING, cur_idx, 0);
        if (cur_idx > 0) cur_idx--;
        SendMessage(lst, LB_SETCURSEL, cur_idx, 0);
      } break;
      case IDC_BTN_ADD: {
        s8* ddb_name = (s8*)MODALDIALOG(IDD_NEWDDB_DIALOG, hDlg, NewDDBDlgProc);
        if (ddb_name) {
          s8 str[1024];
          sprintf(str, "%s/data/%s/%s.ddb", g_base_dir, g_elf_name, ddb_name);
          HWND lst = GetDlgItem(hDlg, IDC_LST_DDBFILE);
          SendMessage(lst, LB_ADDSTRING, 0, (LPARAM)str);
          g_dbg->loadDdb(str);
        }
      } break;
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
