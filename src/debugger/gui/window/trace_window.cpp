/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/trace_window.h"
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

/*----------*/
/* function */
/*----------*/
TraceWindow::TraceWindow() : ListWindowBase() {
  m_category = kCat_CallStack;
  m_small_font = NULL;
}

TraceWindow::~TraceWindow() {
}

void TraceWindow::Init() {
  ListWindowBase::Init(IDD_CALLSTACK, (DLGPROC)ListWindowBase::DlgProc, kViewX, kViewY, kViewW, kViewH);

  SetScrollMax(0);

  clearColumns();
  AddCol("addr", 52, true, NULL);
  AddCol("from", 52, true, NULL);
  AddCol("label", 120, false, NULL);

  if (m_small_font == NULL) {
    LOGFONT  lf;
    memset(&lf, 0, sizeof(LOGFONT));
    lf.lfHeight      = kFontSize - 2;
    lf.lfWidth      = 0;
    lf.lfEscapement    = 0;
    lf.lfOrientation  = 0;
    lf.lfWeight      = FW_NORMAL;
    lf.lfItalic      = FALSE;
    lf.lfUnderline    = FALSE;
    lf.lfStrikeOut    = FALSE;
    lf.lfCharSet    = SHIFTJIS_CHARSET;
    lf.lfOutPrecision  = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision  = CLIP_DEFAULT_PRECIS;
    lf.lfQuality    = DEFAULT_QUALITY;
    lf.lfPitchAndFamily  = DEFAULT_PITCH | FF_DONTCARE;
    strcpy(lf.lfFaceName, "ＭＳ ゴシック");
    m_small_font = CreateFontIndirect(&lf);
  }
}

bool TraceWindow::PreTranslateMessage(MSG* p_msg) {
  return false;
}

void TraceWindow::Draw() {
  SetScrollMax(GetAllLine());

  DrawListFrame();

  HDC hdc = m_dlg->getBackDC();
  switch (m_category) {
  case kCat_CallStack:  Draw_CallStack(hdc); break;
  case kCat_JumpLog:    Draw_JumpLog(hdc); break;
  }

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);
  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

void TraceWindow::Draw_CallStack(HDC p_hdc) {
  SetScrollMax(GetAllLine());

  HDC hdc = m_dlg->getBackDC();

  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbFixedBG = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  SetBkColor(p_hdc, RGB(0, 0, 0));
  SetBkMode(p_hdc, TRANSPARENT);
  g_dbg->setFontToFixed(p_hdc);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  DrawListFrame();

  SetTextColor(p_hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (i + m_head < GetAllLine() && m_sel == i + m_head) FillRect(hdc, &selrect, hbSelect);
      else FillRect(hdc, &selrect, hbBG);

      if (i + m_head < GetAllLine()) {
        switch (j) {
        case 0:
          sprintf(str, "%08x", g_dbg->callstack(i + m_head)->jva());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 1:
          sprintf(str, "%08x", g_dbg->callstack(i + m_head)->va());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 2: {
            Label *label = g_dbg->find_codelabel(g_dbg->callstack(i + m_head)->jva());
            if (label) {
              TextOut(hdc, left + 4, kFontSize * i + GetListTop(), label->text(), strlen(label->text()));
            }
          }
          break;
        }
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hbSelect);
  DeleteObject(hbFixedBG);
  DeleteObject(hbBG);

  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

void TraceWindow::Draw_JumpLog(HDC p_hdc) {
  SetScrollMax(GetAllLine());

  HDC hdc = m_dlg->getBackDC();

  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbFixedBG = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  SetBkColor(p_hdc, RGB(0, 0, 0));
  SetBkMode(p_hdc, TRANSPARENT);
  g_dbg->setFontToFixed(p_hdc);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  DrawListFrame();

  SetTextColor(p_hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (i + m_head < GetAllLine() && m_sel == i + m_head) FillRect(hdc, &selrect, hbSelect);
      else FillRect(hdc, &selrect, hbBG);

      if (i + m_head < GetAllLine()) {
        switch (j) {
        case 0:
          sprintf(str, "%08x", g_dbg->jumplog(i + m_head)->va());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 1:
          //if (g_dbg->jumplog(i + m_head)->m_repeat.size() > 0) {
          //  int ridx = g_dbg->jumplog(i + m_head)->m_repeat.size() - 1;
          //  sprintf(str, "    cnt=%d len=%d lv=%d",
          //    g_dbg->jumplog(i + m_head)->m_repeat[ridx]->count,
          //    g_dbg->jumplog(i + m_head)->m_repeat[ridx]->len,
          //    g_dbg->getJumpLogNestLevel(i + m_head, ridx) + ridx - 1);
          //  TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          //}
          break;
        }
      }
      left += m_col_array[j]->width;
    }
  }

  // ヘッダ行を上書きしてしまうのでクリッピング
  IntersectClipRect(hdc, clientrect.left, GetListTop(), clientrect.right, clientrect.bottom);
  
  // ネストブロックを図で示す
  HPEN hpPen = CreatePen(PS_SOLID, 1, RGB(255, 255, 64));
  SelectObject(hdc, hpPen);
  SelectObject(p_hdc, m_small_font);
  for (int i = 0; i < g_dbg->jumplog_count(); i++) {
    for (u32 j = 0; j < g_dbg->jumplog(i)->m_repeat.size(); j++) {
      const int kNestIndent = 12;
      int level = g_dbg->getJumpLogNestLevel(i, j) + j;
      int range = g_dbg->jumplog(i)->m_repeat[j]->len - 1;
      int count = g_dbg->jumplog(i)->m_repeat[j]->count;
      int left = m_col_array[0]->width;
      int top  = GetListTop() + kFontSize * (i - m_head);
      
      s8 cnt_str[256];
      if (count > 999) {
        strcpy(cnt_str, "*");
      } else {
        _itoa(count, cnt_str, 10);
      }

      if (g_dbg->jumplog(i)->m_repeat[j]->len == 1) {
        MoveToEx(hdc, left, top + kFontSize / 2, NULL);
        LineTo(hdc,   left + 4, top + kFontSize / 2);

        TextOut(hdc, left + 5, top, cnt_str, strlen(cnt_str));
      } else {
        MoveToEx(hdc, left, top + 2, NULL);
        LineTo(hdc,   left + level * kNestIndent, top + 2);

        MoveToEx(hdc, left, top - 2 + kFontSize * (1 + range), NULL);
        LineTo(hdc,   left + level * kNestIndent, top - 2 + kFontSize * (1 + range));

        MoveToEx(hdc, left + level * kNestIndent, top + 3, NULL);
        LineTo(hdc,   left + level * kNestIndent, top - 2 + kFontSize * (1 + range));

        TextOut(hdc, left + 1 + level * kNestIndent, top + range * kFontSize / 2, cnt_str, strlen(cnt_str));
      }
    }
  }
  DeleteObject(hpPen);

  DeleteObject(hbSelect);
  DeleteObject(hbFixedBG);
  DeleteObject(hbBG);

  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

int TraceWindow::GetAllLine() {
  if (g_dbg) {
    if (m_category == kCat_CallStack) {
      return g_dbg->callstack_count();
    } else if (m_category == kCat_JumpLog) {
      return g_dbg->jumplog_count();
    }
  }
  return 0;
}

//-------------------------------------------------------------------------------------------------
// event handler
void TraceWindow::OnCommand(int p_ctrlID, int p_notify) {
  switch (p_ctrlID) {
  case IDC_CMB_CATEGORY:
    switch (p_notify) {
    case CBN_SELCHANGE:
      HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_CATEGORY);
      m_category = SendMessage(cmb, CB_GETCURSEL, 0L, 0L);
      
      clearColumns();
      switch (m_category) {
      case TraceWindow::kCat_CallStack:
        AddCol("addr", 52, true, NULL);
        AddCol("from", 52, true, NULL);
        AddCol("label", 120, false, NULL);
        break;
      case TraceWindow::kCat_JumpLog:
        AddCol("addr", 52, true, NULL);
        AddCol("count", 170, true, NULL);
        break;
      }
      SetScrollMax(GetAllLine());

      m_head = 0;
      SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
      
      Draw();
      break;
    }
    break;
  }
}

void TraceWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  HWND cmb;
  cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_CATEGORY);
  for (int i = SendMessage(cmb, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) SendMessage(cmb, CB_DELETESTRING, 0, 0);
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"CallStack");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Jump Log");
  SendMessage(cmb, CB_SETCURSEL, 0L, 0L);
  SendMessage(m_dlg->getHwnd(), WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_CATEGORY, 0L);

  SetFocus(m_dlg->getHwnd());  // フォーカスをダイアログにセット
}

void TraceWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {
  switch (p_vk) {
  case VK_RETURN:
    if (m_sel >= 0 && m_sel < GetAllLine()) {
      if (m_category == kCat_CallStack) {
        g_dbg->code_window()->AutoJumpModule(g_dbg->callstack(m_sel)->jva());
        g_dbg->code_window()->SelectAddress(g_dbg->callstack(m_sel)->jva());
      } else if (m_category == kCat_JumpLog) {
        g_dbg->code_window()->AutoJumpModule(g_dbg->jumplog(m_sel)->va());
        g_dbg->code_window()->SelectAddress(g_dbg->jumplog(m_sel)->va());
      }
    }
    break;
  case VK_UP:    CursorUp(1); break;
  case VK_PRIOR:  CursorUp(GetPageLine()); break;    // PageUp
  case VK_DOWN:  CursorDown(1); break;
  case VK_NEXT:  CursorDown(GetPageLine()); break;  // PageDown
  }
}
