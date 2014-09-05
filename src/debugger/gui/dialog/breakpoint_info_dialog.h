#ifndef _debugger_gui_breakpointinfodialog
#define _debugger_gui_breakpointinfodialog

/*---------*/
/* include */
/*---------*/
#include "main.h"

/*--------*/
/* define */
/*--------*/

/*-------*/
/* class */
/*-------*/

/*----------*/
/* function */
/*----------*/
HWND GetBreakPointInfoDlg();
void BreakPointInfoDlgSetup(int p_left, int p_top, BreakPointBase* p_breakpoint);
LRESULT CALLBACK BreakPointInfoDlgProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);

#endif // _debugger_gui_breakpointinfodialog
