#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/dialog/breakpoint_info_dialog.h"
#include "debugger/gui/window/breakpoint_window.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/window/memory_window.h"

#include <algorithm>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

bool Compare_Addr_Ascend(const void *p_a, const void *p_b) {
  const BreakPointBase* a = static_cast<const BreakPointBase*>(p_a);
  const BreakPointBase* b = static_cast<const BreakPointBase*>(p_b);
  return a->va() < b->va();
}
bool Compare_Addr_Descend(const void *p_a, const void *p_b) {
  return !Compare_Addr_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Addr(bool p_ascend) {
  return p_ascend ? Compare_Addr_Ascend : Compare_Addr_Descend;
}

bool Compare_Comment_Ascend(const void *p_a, const void *p_b) {
  const BreakPointBase* a = static_cast<const BreakPointBase*>(p_a);
  const BreakPointBase* b = static_cast<const BreakPointBase*>(p_b);
  Comment* ca = g_dbg->find_comment(a->va());
  Comment* cb = g_dbg->find_comment(b->va());
  if (!ca && !cb) goto _same_comment;
  if (!ca) goto _less;
  if (!cb) goto _not_less;
  int scmp = strcmp(ca->text(), cb->text());
  if (scmp < 0) goto _less;
  else if (scmp == 0) goto _same_comment;
_not_less:     return false;
_less:         return true;
_same_comment: return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_Comment_Descend(const void *p_a, const void *p_b) {
  return !Compare_Comment_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Comment(bool p_ascend) {
  return p_ascend ? Compare_Comment_Ascend : Compare_Comment_Descend;
}

};

BreakPointWindow::BreakPointWindow() : ListWindowBase() {
}

BreakPointWindow::~BreakPointWindow() {
  m_sorted_ary.clear();
}

void BreakPointWindow::Init() {
  ListWindowBase::Init(IDD_BREAKPOINT, (DLGPROC)ListWindowBase::DlgProc, kViewX, kViewY, kViewW, kViewH);

  SetScrollMax(0);

  clearColumns();
  AddCol("addr", 52, true, Compare_Addr);
  AddCol("type", 38, true, NULL);
  AddCol("log", 24, true, NULL);
  AddCol("condition", 120, false, NULL);
  AddCol("comment", 120, false, Compare_Comment);

  m_head = 0;
  update_list();
}

bool BreakPointWindow::PreTranslateMessage(MSG* p_msg) {
  return false;
}

void BreakPointWindow::Draw() {
  SetScrollMax(GetAllLine());

  HDC hdc = m_dlg->getBackDC();

  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbFixedBG = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);
  HBRUSH hbBreak = CreateSolidBrush(COLOR_BREAK);

  SetBkColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  g_dbg->setFontToFixed(hdc);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  DrawListFrame();

  SetTextColor(hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom = (i + 1) * kFontSize + GetListTop();
      selrect.left   = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (j == 0) {
        if (i + m_head < GetAllLine() && m_sorted_ary[i + m_head]->enable()) {
          FillRect(hdc, &selrect, hbBreak);
        } else {
          FillRect(hdc, &selrect, hbBG);
        }
      } else {
        if (i + m_head < GetAllLine() && m_sel == i + m_head) {
          FillRect(hdc, &selrect, hbSelect);
        } else {
          FillRect(hdc, &selrect, hbBG);
        }
      }

      if (i + m_head < GetAllLine()) {
        switch (j) {
        case 0:
          sprintf(str, "%08x", m_sorted_ary[i + m_head]->va());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 1: {
            CodeBreakPoint* codeBreak = dynamic_cast<CodeBreakPoint*>(m_sorted_ary[i + m_head]);
            MemoryBreakPoint* memoryBreak = dynamic_cast<MemoryBreakPoint*>(m_sorted_ary[i + m_head]);

            if (codeBreak) {
              TextOut(hdc, left + 4, kFontSize * i + GetListTop(), "code", 4);
            } else if (memoryBreak) {
              char str[] = "mem  ", *ptr = str + 3;
              if (memoryBreak->read_enable()) *ptr++ = 'R';
              if (memoryBreak->write_enable()) *ptr++ = 'W';
              TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, 5);
            }
          break; }
        case 2: {
            bool log = m_sorted_ary[i + m_head]->output_log();
            TextOut(hdc, left + 4, kFontSize * i + GetListTop(), log ? "on " : "off", 3);
          break; }
        case 3: {
            const char* exprstr = m_sorted_ary[i + m_head]->expr();
            TextOut(hdc, left + 4, kFontSize * i + GetListTop(), exprstr, strlen(exprstr));
          break; }
        case 4:
          if (m_sorted_ary[i + m_head]->type() == BreakPointBase::kType_Code) {
            Comment* comment = g_dbg->find_comment(m_sorted_ary[i + m_head]->va());
            if (comment) TextOut(hdc, left + 4, kFontSize * i + GetListTop(), comment->text(), strlen(comment->text()));
          } else {
            int idx;
            if (g_dbg->getMemLabelIdx(m_sorted_ary[i + m_head]->va(), &idx) >= 0) {
              if (idx >= 0) TextOut(hdc, left + 4, kFontSize * i + GetListTop(), g_dbg->getMemLabel(idx)->comment(), strlen(g_dbg->getMemLabel(idx)->comment()));
            }
          }
          break;
        }
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hbBreak);
  DeleteObject(hbSelect);
  DeleteObject(hbFixedBG);
  DeleteObject(hbBG);

  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

int BreakPointWindow::GetAllLine() {
  return m_sorted_ary.size();
}

void BreakPointWindow::SortListData(QSORT_FUNC p_sort_func) {
  std::sort(m_sorted_ary.begin(), m_sorted_ary.end(), p_sort_func);
}

void BreakPointWindow::OnCommand(int p_ctrlID, int p_notify) {
  switch (p_ctrlID) {
  case kPopupMenu_OpenDialog:
    OpenBreakInfoDialog();
    break;
  case kPopupMenu_Delete:
    if (m_sel < GetAllLine()) {
      for (int i = 0; i < g_dbg->get_breakpoint_count(); i++) {
        if (g_dbg->get_breakpoint(i) == m_sorted_ary[m_sel]) {
          g_dbg->delete_breakpoint(i);
          break;
        }
      }
      g_dbg->drawAllWindow();
    }
    break;
  case kPopupMenu_ToggleState:
    if (m_sel < GetAllLine()) {
      m_sorted_ary[m_sel]->set_enable(!m_sorted_ary[m_sel]->enable());
      g_dbg->drawAllWindow();
    }
    break;
  }
}

void BreakPointWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  if (p_wp) {
    HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_ADDR);
    g_dbg->setFontToFixed(cmb);
    SendMessage(cmb, CB_SETEDITSEL, 0L, MAKELONG(0, 0));    // コンボテキスト選択解除
    SetFocus(m_dlg->getHwnd());                  // フォーカスをダイアログにセット
  }
}

void BreakPointWindow::OnRButtonDown(int p_x, int p_y) {
  if (p_y < GetListTop()) return;

  int line = (p_y - GetListTop()) / kFontSize;

  if (line >= 0 && line <= GetPageLine()) {
    if (line < GetAllLine()) {
      m_sel = m_head + line;
      Draw();

      POINT gpt = { p_x, p_y };
      ClientToScreen(m_dlg->getHwnd(), &gpt);

      HMENU menu = CreatePopupMenu();
      AppendMenu(menu, MF_STRING | MF_ENABLED, kPopupMenu_OpenDialog, (LPCTSTR)"編集ダイアログを開く...");
      AppendMenu(menu, MF_STRING | MF_SEPARATOR, 0, (LPCTSTR)"");
      AppendMenu(menu, MF_STRING | MF_ENABLED, kPopupMenu_ToggleState, (LPCTSTR)"Enable/Disable切り替え\t<Space>");
      AppendMenu(menu, MF_STRING | MF_SEPARATOR, 0, (LPCTSTR)"");
      AppendMenu(menu, MF_STRING | MF_ENABLED, kPopupMenu_Delete, (LPCTSTR)"削除\t<Del>");
      TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON, gpt.x, gpt.y, 0, m_dlg->getHwnd(), NULL);
    }
  }
}

void BreakPointWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {
  switch (p_vk) {
  case VK_SPACE:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, kPopupMenu_ToggleState, 0); break;
  case VK_DELETE:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, kPopupMenu_Delete, 0); break;
  case VK_RETURN:
    if (m_sel < GetAllLine()) {
      if (m_sorted_ary[m_sel]->type() == BreakPointBase::kType_Code) {
        g_dbg->code_window()->AutoJumpModule(m_sorted_ary[m_sel]->va());
        g_dbg->code_window()->SelectAddress(m_sorted_ary[m_sel]->va());
      } else if (m_sorted_ary[m_sel]->type() == BreakPointBase::kType_Memory) {
        g_dbg->memory_window()->SelectAddress(m_sorted_ary[m_sel]->va());
      }
    }
    break;
  case VK_UP:    CursorUp(1); break;
  case VK_PRIOR:  CursorUp(GetPageLine()); break;    // PageUp
  case VK_DOWN:  CursorDown(1); break;
  case VK_NEXT:  CursorDown(GetPageLine()); break;  // PageDown
  }
}

void BreakPointWindow::add_entry(BreakPointBase* p_data) {
  m_sorted_ary.push_back(p_data);
  Draw();
}

void BreakPointWindow::remove_entry(BreakPointBase* p_data) {
  for (u32 i = 0; i < m_sorted_ary.size(); i++) {
    if (m_sorted_ary[i] == p_data) {
      m_sorted_ary.erase(&m_sorted_ary[i]);
      break;
    }
  }
  Draw();
}

void BreakPointWindow::update_list() {
  m_sorted_ary.clear();

  if (g_dbg == NULL) return;

  int count = g_dbg->get_breakpoint_count();
  for (int i = 0; i < count; i++) {
    if (g_dbg->get_breakpoint(i)->is_cur()) {
      m_sorted_ary.push_back(g_dbg->get_breakpoint(i));
    }
  }
}

void BreakPointWindow::OpenBreakInfoDialog() {
  if (m_sel >= 0 && m_sel < GetAllLine()) {

    HWND hwnd = GetBreakPointInfoDlg();
    if (hwnd) {
      SetForegroundWindow(hwnd);
    } else {
      RECT rect;
      GetWindowRect(m_dlg->getHwnd(), &rect);
      BreakPointInfoDlgSetup(rect.left - 525, rect.top, m_sorted_ary[m_sel]);

      if (MODALDIALOG(IDD_BREAKPOINT_INFO_DIALOG, m_dlg->getHwnd(), BreakPointInfoDlgProc) == IDOK) {
        g_dbg->drawAllWindow();
      }
    } 
  }
}
