#include "main.h"
#include "debugger/debugger.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/profiler_window.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/window/memory_window.h"

#include <algorithm>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

bool Compare_Addr_Ascend(const void *p_a, const void *p_b) {
  const ProfileEntry* a = static_cast<const ProfileEntry*>(p_a);
  const ProfileEntry* b = static_cast<const ProfileEntry*>(p_b);
  return a->va() < b->va();
}
bool Compare_Addr_Descend(const void *p_a, const void *p_b) {
  return !Compare_Addr_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Addr(bool p_ascend) {
  return p_ascend ? Compare_Addr_Ascend : Compare_Addr_Descend;
}

bool Compare_FuncLabel_Ascend(const void *p_a, const void *p_b) {
  const ProfileEntry* a = static_cast<const ProfileEntry*>(p_a);
  const ProfileEntry* b = static_cast<const ProfileEntry*>(p_b);
  return strcmp(a->label(), b->label()) < 0;
}
bool Compare_FuncLabel_Descend(const void *p_a, const void *p_b) {
  return !Compare_FuncLabel_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_FuncLabel(bool p_ascend) {
  return p_ascend ? Compare_FuncLabel_Ascend : Compare_FuncLabel_Descend;
}

bool Compare_FuncTime_Ascend(const void *p_a, const void *p_b) {
  const ProfileEntry* a = static_cast<const ProfileEntry*>(p_a);
  const ProfileEntry* b = static_cast<const ProfileEntry*>(p_b);
  return a->func_time() < b->func_time();
}
bool Compare_FuncTime_Descend(const void *p_a, const void *p_b) {
  return !Compare_FuncTime_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_FuncTime(bool p_ascend) {
  return p_ascend ? Compare_FuncTime_Ascend : Compare_FuncTime_Descend;
}

bool Compare_FDTime_Ascend(const void *p_a, const void *p_b) {
  const ProfileEntry* a = static_cast<const ProfileEntry*>(p_a);
  const ProfileEntry* b = static_cast<const ProfileEntry*>(p_b);
  return a->fd_time() < b->fd_time();
}
bool Compare_FDTime_Descend(const void *p_a, const void *p_b) {
  return !Compare_FDTime_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_FDTime(bool p_ascend) {
  return p_ascend ? Compare_FDTime_Ascend : Compare_FDTime_Descend;
}

bool Compare_Count_Ascend(const void *p_a, const void *p_b) {
  const ProfileEntry* a = static_cast<const ProfileEntry*>(p_a);
  const ProfileEntry* b = static_cast<const ProfileEntry*>(p_b);
  return a->count() < b->count();
}
bool Compare_Count_Descend(const void *p_a, const void *p_b) {
  return !Compare_Count_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Count(bool p_ascend) {
  return p_ascend ? Compare_Count_Ascend : Compare_Count_Descend;
}

};

ProfilerWindow::ProfilerWindow() : ListWindowBase() {
}

ProfilerWindow::~ProfilerWindow() {
  m_sorted_ary.clear();
}

void ProfilerWindow::Init() {
  ListWindowBase::Init(IDD_PROFILER, (DLGPROC)ListWindowBase::DlgProc, kViewX, kViewY, kViewW, kViewH);

  SetScrollMax(0);

  clearColumns();
  AddCol("addr", 52, false, Compare_Addr);
  AddCol("func label", 100, false, Compare_FuncLabel);
  AddCol("count", 52, false, Compare_Count);
  AddCol("f time", 64, false, Compare_FuncTime);
  AddCol("f+d time", 64, false, Compare_FDTime);
}

bool ProfilerWindow::PreTranslateMessage(MSG* p_msg) {
  return false;
}

void ProfilerWindow::Draw() {
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
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      FillRect(hdc, &selrect, hbBG);
      
      if (i + m_head < GetAllLine() && m_sel == i + m_head) {
        FillRect(hdc, &selrect, hbSelect);
      } else {
        FillRect(hdc, &selrect, hbBG);
      }


      if (i + m_head < GetAllLine()) {
        switch (j) {
        case 0:
          sprintf(str, "%08x", g_dbg->getProfile(i + m_head)->va());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
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

int ProfilerWindow::GetAllLine() {
  return g_dbg->getProfileCount();
}

void ProfilerWindow::SortListData(QSORT_FUNC p_sort_func) {
  std::sort(m_sorted_ary.begin(), m_sorted_ary.end(), p_sort_func);
}

void ProfilerWindow::OnCommand(int p_ctrlID, int p_notify) {
//  switch (p_ctrlID) {
//  }
}

void ProfilerWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  if (p_wp) {
    HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_ADDR);
    g_dbg->setFontToFixed(cmb);
    SendMessage(cmb, CB_SETEDITSEL, 0L, MAKELONG(0, 0));    // コンボテキスト選択解除
    SetFocus(m_dlg->getHwnd());                  // フォーカスをダイアログにセット
  }
}

void ProfilerWindow::OnRButtonDown(int p_x, int p_y) {
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

void ProfilerWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {
  switch (p_vk) {
  case VK_SPACE:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, kPopupMenu_ToggleState, 0); break;
  case VK_DELETE:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, kPopupMenu_Delete, 0); break;
  case VK_RETURN:
    if (m_sel < GetAllLine()) {
      if (g_dbg->get_breakpoint(m_sel)->type() == BreakPointBase::kType_Code) {
        g_dbg->code_window()->AutoJumpModule(g_dbg->get_breakpoint(m_sel)->va());
        g_dbg->code_window()->SelectAddress(g_dbg->get_breakpoint(m_sel)->va());
      } else if (g_dbg->get_breakpoint(m_sel)->type() == BreakPointBase::kType_Memory) {
        g_dbg->memory_window()->SelectAddress(g_dbg->get_breakpoint(m_sel)->va());
      }
    }
    break;
  case VK_UP:    CursorUp(1); break;
  case VK_PRIOR:  CursorUp(GetPageLine()); break;    // PageUp
  case VK_DOWN:  CursorDown(1); break;
  case VK_NEXT:  CursorDown(GetPageLine()); break;  // PageDown
  }
}

void ProfilerWindow::add_entry(ProfileEntry* p_data) {
  m_sorted_ary.push_back(p_data);
  Draw();
}

void ProfilerWindow::remove_entry(ProfileEntry* p_data) {
  for (u32 i = 0; i < m_sorted_ary.size(); i++) {
    if (m_sorted_ary[i] == p_data) {
      m_sorted_ary.erase(&m_sorted_ary[i]);
      break;
    }
  }
  Draw();
}

void ProfilerWindow::update_list() {
  m_sorted_ary.clear();

  if (g_dbg == NULL) return;

  int count = g_dbg->getProfileCount();
  for (int i = 0; i < count; i++) {
    m_sorted_ary.push_back(g_dbg->getProfile(i));
  }
}
