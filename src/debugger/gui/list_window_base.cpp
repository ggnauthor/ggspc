#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/analyze_info.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/list_window_base.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

int    g_dc_ref_count = 0;
HDC    g_dc_sort_ascend  = NULL;
HDC    g_dc_sort_descend = NULL;

};

ListWindowBase::ListWindowBase() :
    m_dlg(NULL),
    m_drag_separater(-1),
    m_head(0),
    m_sel(0) {
  if (g_dc_ref_count == 0) {
    HINSTANCE  hinst = GetModuleHandle(NULL);
    HDC      hdc = GetDC(NULL);
    HBITMAP    hbitmap;

    hbitmap = LoadBitmap(hinst, MAKEINTRESOURCE(IDB_BITMAP7));
    g_dc_sort_ascend = CreateCompatibleDC(hdc);
    SelectObject(g_dc_sort_ascend, hbitmap);
    DeleteObject(hbitmap);

    hbitmap = LoadBitmap(hinst, MAKEINTRESOURCE(IDB_BITMAP8));
    g_dc_sort_descend = CreateCompatibleDC(hdc);
    SelectObject(g_dc_sort_descend, hbitmap);
    DeleteObject(hbitmap);

    ReleaseDC(NULL, hdc);

    g_dc_ref_count++;
  }
}

ListWindowBase::~ListWindowBase() {
  clearColumns();
  if (m_dlg) delete m_dlg;

  if (--g_dc_ref_count == 0) {
    DeleteDC(g_dc_sort_ascend);
    DeleteDC(g_dc_sort_descend);
  }
}

void ListWindowBase::Init(int p_res_id, DLGPROC p_dlg_proc,
                          int p_x, int p_y, int p_w, int p_h) {
  if (m_dlg == NULL) {
    m_dlg = new ds_dialog(p_res_id, NULL, p_dlg_proc, reinterpret_cast<LPARAM>(this));
    m_dlg->setUserData((DWORD)this);
    m_dlg->setStyle(m_dlg->getStyle() | WS_VSCROLL);
    m_dlg->move(p_x, p_y);
    m_dlg->size(p_w, p_h);
    m_dlg->createDC();
  }
  InitScroll();
  ShowWindow(m_dlg->getHwnd(), SW_SHOW);
}


void ListWindowBase::clearColumns() {
  for (vector<ListWindowCol*>::iterator itr = m_col_array.begin();
      itr != m_col_array.end();
      ++itr) {
    delete *itr;
  }
  m_col_array.clear();
}

int ListWindowBase::GetPageLine() {
  // １ページあたりの行数を取得する
  RECT client_rect;
  GetClientRect(m_dlg->getHwnd(), &client_rect);
  return (client_rect.bottom - client_rect.top - GetListTop()) / kFontSize;
}

void ListWindowBase::InitScroll() {
  SetScrollRange(m_dlg->getHwnd(), SB_VERT, 0, 0, TRUE);
  EnableScrollBar(m_dlg->getHwnd(), SB_VERT, ESB_DISABLE_BOTH);
  SetScrollPos(m_dlg->getHwnd(), SB_VERT, 0, TRUE);
  m_head = 0;
  m_sel = 0;
  m_line_count = 0;
}

void ListWindowBase::SetScrollMax(int p_line_count) {
  // スクロール範囲を設定する。１ページに収まる場合、スクロールは非表示にする。
  m_line_count = p_line_count;
  if (p_line_count <= GetPageLine()) {
    SetScrollRange(m_dlg->getHwnd(), SB_VERT, 0, 0, TRUE);
    EnableScrollBar(m_dlg->getHwnd(), SB_VERT, ESB_DISABLE_BOTH);
  } else {
    SetScrollRange(m_dlg->getHwnd(), SB_VERT, 0, p_line_count - GetPageLine(), TRUE);
    EnableScrollBar(m_dlg->getHwnd(), SB_VERT, ESB_ENABLE_BOTH);
  }
}

void ListWindowBase::OnSize(WPARAM p_wp, LPARAM p_lp) {
  m_dlg->releaseDC();
  m_dlg->createDC();
  if (g_dbg) SetScrollMax(GetAllLine());
}

void ListWindowBase::OnVScroll(WPARAM p_wp, LPARAM p_lp) {
  SCROLLINFO si;
  si.cbSize = sizeof(si);
  si.fMask = SIF_ALL;
  GetScrollInfo(m_dlg->getHwnd(), SB_VERT, &si);

  switch (LOWORD(p_wp)) {
    case SB_LINEUP:     si.nPos--; break;
    case SB_LINEDOWN:   si.nPos++; break;
    case SB_THUMBTRACK: si.nPos = si.nTrackPos; break;
    case SB_PAGEUP:     si.nPos -= GetPageLine(); break;
    case SB_PAGEDOWN:   si.nPos += GetPageLine(); break;
    default: return;
  }

  int min, max;
  GetScrollRange(m_dlg->getHwnd(), SB_VERT, &min, &max);
  if (si.nPos < min) si.nPos = min;
  if (si.nPos > max) si.nPos = max;

  SetScrollPos(m_dlg->getHwnd(), SB_VERT, si.nPos, true);
  m_head = si.nPos;
  Draw();
}

void ListWindowBase::OnMouseWheel(WPARAM p_wp, LPARAM p_lp) {
  int pos;
  if (p_wp & 0x80000000) {
    pos = GetScrollPos(m_dlg->getHwnd(), SB_VERT) + 6;
  } else {
    pos = GetScrollPos(m_dlg->getHwnd(), SB_VERT) - 6;
  }

  int min, max;
  GetScrollRange(m_dlg->getHwnd(), SB_VERT, &min, &max);
  if (pos < min) pos = min;
  if (pos > max) pos = max;

  SetScrollPos(m_dlg->getHwnd(), SB_VERT, pos, true);
  m_head = pos;
  Draw();
}

void ListWindowBase::OnMouseMove(int p_x, int p_y) {
  if (m_drag_separater >= 0) {
    m_col_array[m_drag_separater]->width = p_x - GetColLeft(m_drag_separater);
    if (m_col_array[m_drag_separater]->width < 20) m_col_array[m_drag_separater]->width = 20;
    Draw();
  } else {
    int col = GetPointingColBorder(p_x, p_y);
    if (col != -1) {
      SetCursor(LoadCursor(NULL, IDC_SIZEWE));
    }
  }
}

void ListWindowBase::OnLButtonDown(int p_x, int p_y) {
  // セパレータ操作
  int col = GetPointingColBorder(p_x, p_y);
  if (col != -1) {
    m_drag_separater = col;
    SetCapture(m_dlg->getHwnd());
    SetCursor(LoadCursor(NULL, IDC_SIZEWE));
    return;
  }

  // ソート
  if (p_y >= GetListHeaderTop() && p_y < GetListTop()) {
    int left = 0;
    for (u32 i = 0; i < m_col_array.size(); i++) {
      if (m_col_array[i]->sort_func != NULL &&
          p_x >= left && p_x < left + m_col_array[i]->width) {
        // 他カラムのソートを解除
        for (u32 j = 0; j < m_col_array.size(); j++) {
          if (i != j) m_col_array[j]->sort_type = ListWindowCol::kSortNone;
        }
        if (m_col_array[i]->sort_type == ListWindowCol::kSortAscend) {
          m_col_array[i]->sort_type = ListWindowCol::kSortDescend;
        } else {
          m_col_array[i]->sort_type = ListWindowCol::kSortAscend;
        }
        SortListData(m_col_array[i]->sort_func(m_col_array[i]->sort_type == ListWindowCol::kSortAscend));
      }
      left += m_col_array[i]->width;
    }
    Draw();
    return;
  }

  // 行選択
  int line = (p_y - GetListTop()) / kFontSize;

  if (line >= 0 && line <= GetPageLine()) {
    m_sel = m_head + line;
    Draw();
  }
  // フォーカスをダイアログにセット
  SetFocus(GetDlgItem(m_dlg->getHwnd(), IDC_DUMMY));
}

void ListWindowBase::OnLButtonDblClk(int p_x, int p_y) {
  if (p_y >= GetListHeaderTop() && p_y < GetListTop()) return;

  int line = (p_y - GetListTop()) / kFontSize;

  if (line >= 0 && line <= GetPageLine()) {
    m_sel = m_head + line;
    Draw();
    PostMessage(m_dlg->getHwnd(), WM_KEYDOWN, VK_RETURN, 0L);
  }
}

void ListWindowBase::OnLButtonUp(int p_x, int p_y) {
  if (m_drag_separater >= 0) {
    m_drag_separater = -1;
    ReleaseCapture();
  }
}

void ListWindowBase::ScrollTo(u32 p_line) {
  // p_lineで指定した行が見える位置までスクロールする
  const int MARGIN = 4;
  if (p_line <= (u32)(m_head + MARGIN)) {
    m_head = p_line - MARGIN;
    if (m_head < 0) m_head = 0;

    SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
  } else if (p_line > (u32)(m_head + GetPageLine() - 1 - MARGIN)) {
    if (p_line < (u32)m_line_count) {
      m_head = p_line - GetPageLine() + 1 + MARGIN;
    } else {
      m_head = m_line_count - GetPageLine() + 1 + MARGIN;
    }
    SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
  }
}

void ListWindowBase::AddCol(const char* p_label, int p_width, bool p_fix, QSORT_FUNC (*p_sort_func)(bool)) {
  ListWindowCol* colinfo = new ListWindowCol;
  strcpy(colinfo->label, p_label);
  colinfo->width     = p_width;
  colinfo->fix       = p_fix;
  colinfo->sort_type = ListWindowCol::kSortNone;
  colinfo->sort_func = p_sort_func;
  m_col_array.push_back(colinfo);
}

int ListWindowBase::GetColLeft(char* p_label) {
  for (u32 i = 0; i < m_col_array.size(); i++) {
    if (strcmp(p_label, m_col_array[i]->label) == 0) return GetColLeft(i);
  }
  assert("label not found.");
  return 0;
}

int ListWindowBase::GetColLeft(int p_idx) {
  assert(p_idx < static_cast<int>(m_col_array.size()));
  int left = 0;
  for (int i = 0; i < p_idx; i++) {
    left += m_col_array[i]->width;
  }
  return left;
}

int ListWindowBase::GetPointingColBorder(int p_x, int p_y) {
  if (GetListHeaderTop() <= p_y) {
    int left = 0;
    for (u32 i = 0; i < m_col_array.size(); i++) {
      left += m_col_array[i]->width;
      if (abs(left - p_x) < 3 && m_col_array[i]->fix == false) {
        return i;
      }
    }
  }
  return -1;
}

void ListWindowBase::DrawListFrame() {
  // リストのタイトルと区切り線を描画
  HDC hdc = m_dlg->getBackDC();

  SetBkColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  g_dbg->setFontToFixed(hdc);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  // カラムタイトル表示
  RECT rect = clientrect;
  rect.bottom  = GetListTop();
  HBRUSH hbFixedBG = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));
  FillRect(hdc, &rect, hbFixedBG);
  SetTextColor(hdc, RGB(0, 0, 0));
  int w = 0;
  for (u32 j = 0; j < m_col_array.size(); j++) {
    int slen = strlen(m_col_array[j]->label);

    TextOut(hdc, w + 4, GetListHeaderTop() + 2, m_col_array[j]->label, slen);

    SIZE sz;
    GetTextExtentPoint32(hdc, m_col_array[j]->label, slen, &sz);
    if (m_col_array[j]->sort_type == ListWindowCol::kSortAscend) {
      BitBlt(hdc, w + 8 + sz.cx, GetListHeaderTop() + 4, 8, 8,
        g_dc_sort_ascend, 0, 0, SRCCOPY);
    } else if (m_col_array[j]->sort_type == ListWindowCol::kSortDescend) {
      BitBlt(hdc, w + 8 + sz.cx, GetListHeaderTop() + 4, 8, 8,
        g_dc_sort_descend, 0, 0, SRCCOPY);
    }
    w += m_col_array[j]->width;
  }
  DeleteObject(hbFixedBG);

  // 区切り線
  HPEN hpBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER);
  SelectObject(hdc, hpBorder);
  for (u32 i = 0; i < m_col_array.size(); i++) {
    MoveToEx(hdc, GetColLeft(i), clientrect.top + GetListHeaderTop(), NULL);
    LineTo(hdc, GetColLeft(i), clientrect.bottom);
  }
  MoveToEx(hdc, clientrect.left, GetListTop(), NULL);
  LineTo(hdc, clientrect.right, GetListTop());
  MoveToEx(hdc, clientrect.left, GetListHeaderTop(), NULL);
  LineTo(hdc, clientrect.right, GetListHeaderTop());
  DeleteObject(hpBorder);
}

void ListWindowBase::DrawAddress(HDC p_hdc, RECT* p_rect, u32 p_va) {
  HBRUSH hbPC = CreateSolidBrush(RGB(255, 255, 255));
  HBRUSH hbBreak = CreateSolidBrush(COLOR_BREAK);

  int idx;
  if (VA2PA(g_cpu->m_pc) == VA2PA(p_va)) {
    SetTextColor(p_hdc, RGB(0, 0, 0));
    FillRect(p_hdc, p_rect, hbPC);
  } else if (g_dbg->get_breakpoint_idx(p_va, &idx, true) >= 0) {
    SetTextColor(p_hdc, RGB(255, 255, 255));
    if (g_dbg->get_breakpoint(idx)->enable()) {
      FillRect(p_hdc, p_rect, hbBreak);
    } else {
      FrameRect(p_hdc, p_rect, hbBreak);
    }
  } else {
    AnalyzeInfoMapItr itr =
      g_dbg->analyze_map_.find(p_va);
    if (g_dbg->isInModuleCodeSection(p_va) &&
        itr != g_dbg->analyze_map_.end() &&
        itr->second.enable()) {
      SetTextColor(p_hdc, RGB(255, 255, 255));
    } else {
      SetTextColor(p_hdc, COLOR_DISABLE);
    }
  }

  // 最適化対象コードなら
  idx = (p_va - g_code_addr) >> 2;
  if (idx >= 0 &&
      idx < static_cast<int>(g_cpu->m_recompile_hint_ary.size()) &&
      g_cpu->m_recompile_hint_ary[idx] > 0) {
    SetTextColor(p_hdc, COLOR_HINT_SPOT);
  }

  char str[1024];
  sprintf(str, "%08x", p_va);
  TextOut(p_hdc, p_rect->left + 3, p_rect->top, str, strlen(str));

  DeleteObject(hbBreak);
  DeleteObject(hbPC);

  SetTextColor(p_hdc, RGB(255, 255, 255));
}

void ListWindowBase::CursorUp(int p_val) {
  m_sel -= p_val;
  // 制限
  if (m_sel < 0) m_sel = 0;
  // 自動スクロール
  if (m_sel < m_head) {
    m_head = m_sel;
    // 制限
    if (m_head < 0) m_head = 0;
  }
  SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
  Draw();
}

void ListWindowBase::CursorDown(int p_val) {
  m_sel += p_val;
  // 制限
  if (m_sel > GetAllLine() - 1) m_sel = GetAllLine() - 1;
  // 自動スクロール
  if (m_sel > m_head + GetPageLine() - 1) {
    m_head = m_sel - GetPageLine() + 1;
  }
  SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
  Draw();
}

LRESULT CALLBACK ListWindowBase::DlgProc(HWND p_hwnd, UINT p_msg, WPARAM p_wp, LPARAM p_lp) {
  ListWindowBase* list_window;
  if (p_msg == WM_INITDIALOG) {
    assert(p_lp);
    list_window = reinterpret_cast<ListWindowBase*>(p_lp);
  } else {
    ds_dialog* dlg = (ds_dialog*)GetWindowLong(p_hwnd, DWL_USER);
    list_window = dlg ? (ListWindowBase*)dlg->getUserData() : NULL;
  }

  if (!list_window) return FALSE;

  switch (p_msg) {
    case WM_INITDIALOG:
      list_window->OnInitDialog(p_hwnd);
      break;
    case WM_ACTIVATE:
      // モーダルダイアログがあったら制御を切り替えない
      if ((p_wp & 0xffff) != WA_INACTIVE) {
        HWND hwnd = g_dbg->findModalDialog();
        if (hwnd) {
          SetActiveWindow(hwnd);
          break;
        }
      }
      list_window->OnActivate(p_wp & 0xffff, (p_wp >> 16) & 0xffff, p_lp);
      break;
    case WM_COMMAND:
      list_window->OnCommand(LOWORD(p_wp), HIWORD(p_wp));
      break;
    case WM_SHOWWINDOW:
      list_window->OnShowWindow(p_wp, p_lp);
      break;
    case WM_SIZE:
      list_window->OnSize(p_wp, p_lp);
      break;
    case WM_VSCROLL:
      list_window->OnVScroll(p_wp, p_lp);
      break;
    case WM_PAINT:
      list_window->OnPaint(p_wp, p_lp);
      break;
    case WM_MOUSEWHEEL:
      list_window->OnMouseWheel(p_wp, p_lp);
      break;
    case WM_MOUSEMOVE:
      list_window->OnMouseMove(p_lp & 0xffff, (p_lp >> 16) & 0xffff);
      break;
    case WM_LBUTTONDOWN:
      list_window->OnLButtonDown(p_lp & 0xffff, (p_lp >> 16) & 0xffff);
      break;
    case WM_LBUTTONDBLCLK:
      list_window->OnLButtonDblClk(p_lp & 0xffff, (p_lp >> 16) & 0xffff);
      break;
    case WM_LBUTTONUP:
      list_window->OnLButtonUp(p_lp & 0xffff, (p_lp >> 16) & 0xffff);
      break;
    case WM_RBUTTONDOWN:
      list_window->OnRButtonDown(p_lp & 0xffff, (p_lp >> 16) & 0xffff);
      break;
    case WM_KEYDOWN:
      list_window->OnKeyDown(
        p_wp,
        (GetKeyState(VK_CONTROL) & 0x8000) != 0,
        (GetKeyState(VK_SHIFT) & 0x8000) != 0);
      break;
    case WM_DRAWITEM:
      list_window->OnDrawItem(
        p_wp,
        reinterpret_cast<DRAWITEMSTRUCT*>(p_lp));
      break;
    case WM_MENUSELECT:
      list_window->OnMenuSelect(
        p_wp & 0xffff,
        (p_wp >> 16) & 0xffff,
        (HMENU)p_lp);
      break;
  }
  return FALSE;
}
