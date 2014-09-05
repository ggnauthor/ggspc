/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/memory_dump_dialog.h"
#include "debugger/gui/window/memory_window.h"

#include <utils/ds_util.h>

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
MemDumpLabel*  last_memdumplabel = NULL;
};

/*----------*/
/* function */
/*----------*/
LRESULT CALLBACK MemDumpDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG: {
    RECT rect;
    GetWindowRect(hDlg, &rect);
    SetWindowPos(hDlg, NULL, rect.left, rect.top - (rect.bottom - rect.top + 20), 0, 0, SWP_NOSIZE | SWP_NOZORDER);

    HWND cmb = GetDlgItem(hDlg, IDC_CMB_DUMPLABEL);
    for (int i = 0; i < g_dbg->getMemDumpLabelCount(); i++) {
      SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)g_dbg->getMemDumpLabel(i)->label());

      if (last_memdumplabel == g_dbg->getMemDumpLabel(i)) {
        SendMessage(cmb, CB_SETCURSEL, i, 0L);
      }
    }
    
    g_dbg->setFontToFixed(GetDlgItem(hDlg, IDC_EDT_START_ADDR));
    g_dbg->setFontToFixed(GetDlgItem(hDlg, IDC_EDT_END_ADDR));

    SetFocus(GetDlgItem(hDlg, IDC_CMB_DUMPLABEL));

    PostMessage(hDlg, WM_COMMAND, CBN_SELCHANGE << 16 | IDC_CMB_DUMPLABEL, 0);
    return FALSE; }
  case WM_DESTROY:
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK: {
      char str[1024];
      
      GetDlgItemText(hDlg, IDC_EDT_START_ADDR, str, 1023);
      ds_util::strtrim(str);
      if (str[0] == '\0') {
        MessageBox(hDlg, "終点アドレスを入力してください", NULL, NULL);
        break;
      }
      u32 start_va = strtoul(str, NULL, 16);

      GetDlgItemText(hDlg, IDC_EDT_END_ADDR, str, 1023);
      ds_util::strtrim(str);
      if (str[0] == '\0') {
        MessageBox(hDlg, "終点アドレスを入力してください", NULL, NULL);
        break;
      }
      u32 end_va = strtoul(str, NULL, 16);

      GetDlgItemText(hDlg, IDC_CMB_DUMPLABEL, str, 1023);
      ds_util::strtrim(str);
      if (str[0] != '\0') {
        // 履歴に追加する。既にあれば消して再追加
        MemDumpLabel* new_memdumplabel = NULL;
        for (int i = 0; i < g_dbg->getMemDumpLabelCount(); i++) {
          if (strcmp(g_dbg->getMemDumpLabel(i)->label(), str) == 0) {
            new_memdumplabel = g_dbg->getMemDumpLabel(i);
            break;
          }
        }
        if (new_memdumplabel == NULL) {
          new_memdumplabel = new MemDumpLabel(str, start_va, end_va);
          g_dbg->addMemDumpLabel(new_memdumplabel);
        }
        last_memdumplabel = new_memdumplabel;
      }

      // ファイルに出力
      s8 defstr[256];
      sprintf(defstr, "%s.dump", str);
      ds_filewindow file_window;
      if (file_window.savefile("ダンプファイル(*.dump)\0*.dump\0\0", defstr, "dump")) {
        FILE* fp = fopen(file_window.getSelectFileName(), "wb");
        if (fp) {
          fwrite((void*)VA2PA(start_va), end_va - start_va, 1, fp);
          fclose(fp);
        }
      }
      EndDialog(hDlg, IDOK);
      break; }
    case IDCANCEL:
      EndDialog(hDlg, IDCANCEL);
      break;
    case IDC_CMB_DUMPLABEL:
      switch (HIWORD(wParam)) {
      case CBN_SELCHANGE:
        // 指定したラベルのアドレスを設定する
        HWND cmb = GetDlgItem(hDlg, IDC_CMB_DUMPLABEL);
        int selidx = SendMessage(cmb, CB_GETCURSEL, 0, 0);
        if (selidx >= 0 && selidx < g_dbg->getMemDumpLabelCount()) {
          s8 str[256];
          _itoa(g_dbg->getMemDumpLabel(selidx)->start_va(), str, 16);
          SetDlgItemText(hDlg, IDC_EDT_START_ADDR, str);
          _itoa(g_dbg->getMemDumpLabel(selidx)->end_va(), str, 16);
          SetDlgItemText(hDlg, IDC_EDT_END_ADDR, str);
        }
        break;
      }
      break;
    default:
      return FALSE;
    }
    return FALSE; 
  default:
    return FALSE; 
  }
  return TRUE;
}
