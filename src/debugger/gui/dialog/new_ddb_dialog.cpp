/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/dialog/new_ddb_dialog.h"
#include "debugger/gui/window/code_window.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

/*---------*/
/* structs */
/*---------*/

/*--------*/
/* global */
/*--------*/
s8 g_ddb_name[1024];

/*----------*/
/* function */
/*----------*/
LRESULT CALLBACK NewDDBDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
  switch (message) {
  case WM_INITDIALOG:
    SetFocus(GetDlgItem(hDlg, IDC_EDIT_FILENAME));
    return FALSE;
  case WM_DESTROY:
    break;
  case WM_COMMAND:
    switch (LOWORD(wParam)) {
    case IDOK: {
      GetDlgItemText(hDlg, IDC_EDIT_FILENAME, g_ddb_name, 1023);
      EndDialog(hDlg, (INT_PTR)g_ddb_name);  
    } break;
    case IDCANCEL:
      EndDialog(hDlg, NULL);
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
