#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/source_info.h"
#include "debugger/disasm.h"
#include "debugger/analyze_info.h"
#include "debugger/patch_entry.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/window/breakpoint_window.h"
#include "debugger/gui/window/reference_window.h"
#include "debugger/gui/window/register_window.h"
#include "debugger/gui/window/memory_window.h"
#include "debugger/gui/dialog/line_edit_dialog.h"
#include "debugger/gui/dialog/addr_jump_dialog.h"
#include "debugger/gui/dialog/find_instruction_dialog.h"
#include "debugger/gui/dialog/breakpoint_info_dialog.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

const s8 CodeWindow::kText_ColStr_Addr[]           = "addr";
const s8 CodeWindow::kText_ColStr_Opcode[]         = "opcode";
const s8 CodeWindow::kText_ColStr_MacroRecompile[] = "macro";
const s8 CodeWindow::kText_ColStr_Assembly[]       = "assembly";
const s8 CodeWindow::kText_ColStr_Comment[]        = "comment";
const s8 CodeWindow::kText_ColStr_Line[]           = "line";
const s8 CodeWindow::kText_ColStr_Source[]         = "source";

CodeWindow::CodeWindow() :
    ListWindowBase(), // TODO:???
    module_idx_(-1),
    src_idx_(-1),
    accel_(NULL) {
}

CodeWindow::~CodeWindow() {
  browse_log_.clear();
}

void CodeWindow::Init() {
  ListWindowBase::Init(
    IDD_CODE,
    (DLGPROC)ListWindowBase::DlgProc,
    kViewX, kViewY, kViewW, kViewH);
  
  HMENU menu = LoadMenu(m_dlg->getInstance(),
                        MAKEINTRESOURCE(IDR_MENU_CODEWINDOW));
  SetMenu(m_dlg->getHwnd(), menu);
  
  accel_ = LoadAccelerators(m_dlg->getInstance(),
                            MAKEINTRESOURCE(IDR_ACC_CODEWINDOW));
  assert(accel_);

  // コントロールがダブルクリックを拾わないようにする
#define EXCLUDE_DOUBLE_CLICK(_ctrl_id, _cls_style) { \
    HWND ctrlHwnd; \
    ctrlHwnd = GetDlgItem(m_dlg->getHwnd(), (_ctrl_id)); \
    SetClassLong(ctrlHwnd, GCL_STYLE, \
      GetClassLong(ctrlHwnd, GCL_STYLE) & ~(_cls_style)); \
  }
  EXCLUDE_DOUBLE_CLICK(ID_DEBUG_RUN, CS_DBLCLKS);
  EXCLUDE_DOUBLE_CLICK(ID_DEBUG_PAUSE, CS_DBLCLKS);
  EXCLUDE_DOUBLE_CLICK(ID_DEBUG_STEPIN, CS_DBLCLKS);
  EXCLUDE_DOUBLE_CLICK(ID_DEBUG_STEPOVER, CS_DBLCLKS);
  EXCLUDE_DOUBLE_CLICK(ID_DEBUG_STEPOUT, CS_DBLCLKS);
  EXCLUDE_DOUBLE_CLICK(ID_DEBUG_RESET, CS_DBLCLKS);
#undef EXCLUDE_DOUBLE_CLICK
}

bool CodeWindow::PreTranslateMessage(MSG* p_msg) {
  if (p_msg->message == WM_KEYDOWN &&
      p_msg->wParam == VK_DOWN &&
      p_msg->hwnd != m_dlg->getHwnd()) {
    return true;
  }
  return false;
}

void CodeWindow::Draw() {
  HDC hdc = m_dlg->getBackDC();

  SetTextColor(hdc, RGB(255, 255, 255));
  SetBkColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  g_dbg->setFontToFixed(hdc);

  DrawListFrame();

  if (GetCurModule()) {
    if (src_idx_ == 0)  {
      Draw_Assembly(hdc);
    } else {
      Draw_Source(hdc);
    }
  }

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);
  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

int CodeWindow::GetAllLine() {
  if (!g_dbg) return 0;

  if (src_idx_ > 0) {
    return g_dbg->src_file_info_ary_[src_idx_]->line_data_size();
  } else {
    if (GetCurModule()) {
      return GetCurModule()->size() / 4;
    }
  }
  return 0;
}

void CodeWindow::Draw_Assembly(HDC p_hdc) {
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);
  HBRUSH hbPC = CreateSolidBrush(COLOR_DEFAULT);
  HBRUSH hbBreak = CreateSolidBrush(COLOR_BREAK);
  //HBRUSH hbPatch = CreateSolidBrush(COLOR_PATCH);
  HBRUSH hbDisableBG = (HBRUSH)CreateSolidBrush(COLOR_DISABLE_BG);

  DWORD st = timeGetTime();

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  bool displayOpcodeInColumn2 = false;
  if (m_col_array.size() >= 1) {
    displayOpcodeInColumn2 =
      strcmp(m_col_array[1]->label, kText_ColStr_Opcode) == 0;
  }

  for (int i = 0; i < GetPageLine() + 1; i++) {
    int left = 0;
    u32 va = GetCurModule()->addr() + (i + m_head) * 4;
    AnalyzeInfoMapItr itr = g_dbg->analyze_map_.find(va);
    bool is_analyzed =
      itr != g_dbg->analyze_map_.end() && itr->second.enable();

    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom = (i + 1) * kFontSize + GetListTop();
      selrect.left   = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else if (itr != g_dbg->analyze_map_.end() &&
                 itr->second.enable()) {
        FillRect(p_hdc, &selrect, hbBG);
      } else {
        FillRect(p_hdc, &selrect, hbDisableBG);
      }
      
      switch (j) {
        case 0:
          DrawAddress(p_hdc, &selrect, va);
          break;
        case 1:
          if (displayOpcodeInColumn2) {
            drawAssemblyOpcodeCol(p_hdc, i, left, is_analyzed);
          } else {
            drawAssemblyMacroRecIdCol(p_hdc, i, left);
          }
          break;
        case 2:
          drawAssemblyCodeCol(p_hdc, i, left, is_analyzed);
          break;
        case 3:
          drawAssemblyCommentCol(p_hdc, i, left);
          break;
      }
      left += m_col_array[j]->width;
    }
  }
  DBGOUT_APP("DrawAssembly = %d(ms)\n", timeGetTime() - st);

  DeleteObject(hbDisableBG);
  //DeleteObject(hbPatch);
  DeleteObject(hbBG);
  DeleteObject(hbSelect);
  DeleteObject(hbBreak);
  DeleteObject(hbPC);
}

void CodeWindow::Draw_Source(HDC p_hdc) {
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  bool c_comment = false;
  bool cpp_comment = false;
  bool in_string = false;

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

      if (m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (i + m_head >= g_dbg->src_file_info_ary_[src_idx_]->line_data_size()) {
        continue;
      }
      
      switch (j) {
        case 0:
          drawSourceLineCol(p_hdc, i, left);
          break;
        case 1:
          drawSourceCodeCol(p_hdc, i, left,
                            &c_comment, &cpp_comment, &in_string);
          break;
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hbBG);
  DeleteObject(hbSelect);
}

void CodeWindow::OnCommand(int p_ctrlID, int p_notify) {
  switch (p_ctrlID) {
  case ID_DEBUG_RUN:
  case ID_DEBUG_RUN_WITHOUT_BREAK:
  case ID_DEBUG_STEPIN:
  case ID_DEBUG_STEPOVER:
    handleDebugExecute(p_ctrlID);
    break;
  case ID_DEBUG_STEPOUT:
    // TODO:
    break;
  case ID_DEBUG_PAUSE:
    handleDebugPause();
    break;
  case ID_DEBUG_RESET:
    PostQuitMessage(0);
    break;
  case IDC_CMB_MEMORY:
    if (p_notify == CBN_SELCHANGE) handleChangeMemory();
    break;
  case IDC_CMB_SOURCE:
    if (p_notify == CBN_SELCHANGE) handleChangeSource();
    break;
  case ID_DEBUG_BREAK:
    handleDebugToggleBreak();
    break;
  case ID_DEBUG_BREAKDLG:
    handleDebugBreakDialog();
    break;
  case ID_DEBUG_TOGGLE_PATCH:
    handleDebugTogglePatch();
    break;
  case ID_DEBUG_SETPC:
    handleDebugSetPC();
    break;
  case ID_DEBUG_EDITDLG:
    handleDebugEdit();
    break;
  case ID_DISPLAY_GO:
    handleDisplayGoDialog();
    break;
  case ID_DISPLAY_GO_PC:
    handleDisplayGoPC();
    break;
  case ID_DISPLAY_GO_REF:
    handleDisplayGoRef();
    break;
  case ID_DISPLAY_GO_PREV:
    handleDisplayGoPrev();
    break;
  case ID_DISPLAY_SOURCE:
    handleDisplaySource();
    break;
  case ID_DISPLAY_ASSEMBLY:
    handleDisplayAssembly();
    break;
  case ID_DISPLAY_FIND_REF:
    handleDisplayFindRef();
    break;
  case ID_DISPLAY_FIND_INST:
    handleDisplayFindInst();
    break;
  case ID_PROFILER_ENABLE:
    handleProfilerEnable(p_notify);
    break;
  case ID_PROFILER_SET_START:
    handleProfilerSetStart();
    break;
  case ID_PROFILER_SET_END:
    handleProfilerSetEnd();
    break;
  }
}

void CodeWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  PostMessage(m_dlg->getHwnd(),
              WM_COMMAND,
              (CBN_SELCHANGE << 16) | IDC_CMB_SOURCE, 0L);
}

void CodeWindow::OnLButtonDown(int p_x, int p_y) {
  // ソート
  if (p_y >= GetListHeaderTop() && p_y < GetListTop()) {
    int left = 0;
    for (u32 i = 0; i < m_col_array.size(); i++) {
      if (p_x >= left && p_x < left + m_col_array[i]->width) {
        if (strcmp(m_col_array[i]->label, kText_ColStr_Opcode) == 0) {
          strcpy(m_col_array[i]->label, kText_ColStr_MacroRecompile);
        } else if (strcmp(m_col_array[i]->label, kText_ColStr_MacroRecompile) == 0) {
          strcpy(m_col_array[i]->label, kText_ColStr_Opcode);
        }
      }
      left += m_col_array[i]->width;
    }
    Draw();
    return;
  }
  ListWindowBase::OnLButtonDown(p_x, p_y);
}

void CodeWindow::OnRButtonDown(int p_x, int p_y) {
  if (p_y < GetListTop()) return;

  int line = (p_y - GetListTop()) / kFontSize;

  if (line >= 0 && line <= GetPageLine()) {
    if (line < GetAllLine()) {
      m_sel = m_head + line;
      Draw();

      POINT gpt = { p_x, p_y };
      ClientToScreen(m_dlg->getHwnd(), &gpt);

      HMENU menu = CreatePopupMenu();
      if (src_idx_ == 0) { // アセンブラ表示時
        AppendMenu(menu,
          MF_STRING | MF_ENABLED,
          ID_DEBUG_EDITDLG, (LPCTSTR)kMenu_LineEdit);
        AppendMenu(menu,
          MF_STRING | MF_SEPARATOR, 0, (LPCTSTR)"");
        AppendMenu(menu,
          MF_STRING | MF_ENABLED,
          ID_DISPLAY_FIND_REF, (LPCTSTR)kMenu_FindRef);
        AppendMenu(menu,
          MF_STRING | MF_SEPARATOR, 0, (LPCTSTR)"");
        AppendMenu(menu,
          MF_STRING | (HasSource() ? MF_ENABLED : MF_GRAYED),
          ID_DISPLAY_SOURCE, (LPCTSTR)kMenu_DispSrc);
      } else {
        AppendMenu(menu,
          MF_STRING | (HasAssembler(m_sel) ? MF_ENABLED : MF_GRAYED),
          ID_DISPLAY_ASSEMBLY, (LPCTSTR)kMenu_DispAsm);
      }
      TrackPopupMenu(menu,
        TPM_LEFTALIGN | TPM_LEFTBUTTON,
        gpt.x, gpt.y, 0, m_dlg->getHwnd(), NULL);
    }
  }
}

void CodeWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {
  if (GetCurModule() == NULL) return;

  // フォーカスウインドウに対するショートカットはOnKeyDownに記述し、
  // アプリ全体に対するものはアクセラレータに記述すること
  switch (p_vk) {
    case VK_F2:
      PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                  p_ctrl ? ID_DEBUG_BREAKDLG : ID_DEBUG_BREAK, 0);
      break;
    case VK_F4:
      PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                  p_ctrl ? ID_DEBUG_SETPC : ID_DISPLAY_GO_PC, 0);
      break;
    case VK_RETURN:
      PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                  ID_DEBUG_EDITDLG, 0);
      break;
    case VK_SPACE:
      PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                  ID_DEBUG_TOGGLE_PATCH, 0);
      break;
    case 'R':
      if (p_ctrl) {
        PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                    ID_DISPLAY_FIND_REF, 0);
      }
      break;
    case 'F':
      if (p_ctrl) {
        PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                    ID_DISPLAY_FIND_INST, 0);
      }
      break;
    case VK_UP:
      CursorUp(1);
      break;
    case VK_DOWN:
      CursorDown(1);
      break;
    case VK_PRIOR: // PageUp
      CursorUp(GetPageLine());
      break;
    case VK_NEXT: // PageDown
      CursorDown(GetPageLine());
      break;
    case VK_RIGHT:
      PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                  ID_DISPLAY_GO_REF, 0);
      break;
    case VK_LEFT:
      PostMessage(m_dlg->getHwnd(), WM_COMMAND,
                  ID_DISPLAY_GO_PREV, 0);
      break;
  }
}

void CodeWindow::OnDrawItem(int p_ctrlID, DRAWITEMSTRUCT* p_di) {
  HDC hdc = p_di->hDC;

  // アイコンの描画
  HINSTANCE inst = (HINSTANCE)GetWindowLong(m_dlg->getHwnd(), GWL_HINSTANCE);
  HBITMAP hBitmap;
  switch (p_ctrlID) {
    case ID_DEBUG_RUN:
      hBitmap = LoadBitmap(inst, MAKEINTRESOURCE(IDB_BITMAP1));
      break;
    case ID_DEBUG_PAUSE:
      hBitmap = LoadBitmap(inst, MAKEINTRESOURCE(IDB_BITMAP2));
      break;
    case ID_DEBUG_RESET:
      hBitmap = LoadBitmap(inst, MAKEINTRESOURCE(IDB_BITMAP6));
      break;
    case ID_DEBUG_STEPIN:
      hBitmap = LoadBitmap(inst, MAKEINTRESOURCE(IDB_BITMAP3));
      break;
    case ID_DEBUG_STEPOVER:
      hBitmap = LoadBitmap(inst, MAKEINTRESOURCE(IDB_BITMAP4));
      break;
    case ID_DEBUG_STEPOUT:
      hBitmap = LoadBitmap(inst, MAKEINTRESOURCE(IDB_BITMAP5));
      break;
  }
  HDC hmdc = CreateCompatibleDC(hdc);
  SelectObject(hmdc, hBitmap);
  BitBlt(hdc,
    (p_di->itemState & ODS_SELECTED) - 1,
    (p_di->itemState & ODS_SELECTED) - 1,
    32, 32, hmdc, 0, 0, SRCCOPY);
  DeleteDC(hmdc);
  DeleteObject(hBitmap);

  // フレームの描画
  HPEN hPen1 = CreatePen(PS_SOLID, 1, RGB(255,255,255));
  HPEN hPen2 = CreatePen(PS_SOLID, 1, RGB(64,64,64));

  RECT clientrect;
  GetClientRect(GetDlgItem(m_dlg->getHwnd(), p_ctrlID), &clientrect);

  SelectObject(hdc, (p_di->itemState & ODS_SELECTED) ? hPen2 : hPen1);
  MoveToEx(hdc, 0, 0, NULL);
  LineTo(hdc, 0, clientrect.bottom);
  MoveToEx(hdc, 0, 0, NULL);
  LineTo(hdc, clientrect.right, 0);

  SelectObject(hdc, (p_di->itemState & ODS_SELECTED) ? hPen1 : hPen2);
  MoveToEx(hdc, clientrect.right - 1, 0, NULL);
  LineTo(hdc, clientrect.right - 1, clientrect.bottom);
  MoveToEx(hdc, 0, clientrect.bottom - 1, NULL);
  LineTo(hdc, clientrect.right, clientrect.bottom - 1);

  DeleteObject(hPen1);
  DeleteObject(hPen2);
}

void CodeWindow::OnMenuSelect(int p_menu_id,
                              int p_menu_flag,
                              HMENU p_menu_handle) {
  bool src_cmb_state = src_idx_ == 0 ? MF_ENABLED : MF_GRAYED;
  switch (p_menu_id) {
    case 0:  // デバッグ
      EnableMenuItem(p_menu_handle, ID_DEBUG_RUN, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_PAUSE, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_RESET, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_STEPIN, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_STEPOVER, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_STEPOUT, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_BREAK, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_BREAKDLG, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_SETPC, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DEBUG_EDITDLG, src_cmb_state);
      break;
    case 1:  // 表示
      EnableMenuItem(p_menu_handle, ID_DISPLAY_GO, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DISPLAY_GO_PC, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DISPLAY_GO_REF, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DISPLAY_GO_PREV, src_cmb_state);
      EnableMenuItem(p_menu_handle,
                     ID_DISPLAY_SOURCE,
                     HasSource() ? MF_ENABLED : MF_GRAYED);
      EnableMenuItem(p_menu_handle,
                     ID_DISPLAY_ASSEMBLY,
                     HasAssembler(m_sel) ? MF_ENABLED : MF_GRAYED);
      EnableMenuItem(p_menu_handle, ID_DISPLAY_FIND_INST, src_cmb_state);
      EnableMenuItem(p_menu_handle, ID_DISPLAY_FIND_REF, src_cmb_state);
      break;
  }
}

void CodeWindow::ModuleSetup() {
  module_idx_ = 0;
  src_idx_ = 0;

  HWND wnd;
  // メモリラベルの設定
  wnd = GetDlgItem(m_dlg->getHwnd(), IDC_ST_MEMORY);
  ShowWindow(wnd, SW_SHOW);
  MoveWindow(wnd, 150, 3, 30, 12, true);
  // メモリコンボの設定
  wnd = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_MEMORY);
  ShowWindow(wnd, SW_SHOW);
  MoveWindow(wnd, 180, 0, 130, 200, true);
  for (int i = SendMessage(wnd, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
    SendMessage(wnd, CB_DELETESTRING, 0, 0);
  }
  for (int i = 0; i < g_dbg->module_count(); i++) {
    SendMessage(wnd, CB_ADDSTRING, 0L, (LPARAM)(g_dbg->module(i)->name()));
  }
  SendMessage(wnd, CB_SETCURSEL, 0L, 0L);

  // ソースラベルの設定
  wnd = GetDlgItem(m_dlg->getHwnd(), IDC_ST_SOURCE);
  ShowWindow(wnd, SW_SHOW);
  MoveWindow(wnd, 317, 3, 40, 12, true);
  // ソースコンボの設定
  wnd = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_SOURCE);
  ShowWindow(wnd, SW_SHOW);
  MoveWindow(wnd, 360, 0, 120, 200, true);
  for (int i = SendMessage(wnd, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) {
    SendMessage(wnd, CB_DELETESTRING, 0, 0);
  }
  for (u32 i = 0; i < g_dbg->src_file_info_ary_.size(); i++) {
    SendMessage(wnd, CB_ADDSTRING, 0L,
      (LPARAM)(g_dbg->src_file_info_ary_[i]->src_file() ?
                 g_dbg->src_file_info_ary_[i]->src_file()->name :
                 "Assembly"));
  }
  SendMessage(wnd, CB_SETCURSEL, 0L, 0L);

  // スクロールの設定
  InitScroll();
  SetScrollMax(GetAllLine());
  ScrollTo(VA2Line(g_cpu->m_pc));

  browse_log_.clear();
}

ModuleInfo* CodeWindow::GetCurModule() {
  if (module_idx_ < 0 || module_idx_ >= g_dbg->module_count()) {
    return NULL;
  }
  return g_dbg->module(module_idx_);
}

void CodeWindow::AutoJumpModule(u32 p_va) {
  if (p_va > GetCurModule()->addr() &&
      p_va < GetCurModule()->addr() + GetCurModule()->size()) {
    return;
  }

  int idx = -1;
  for (int i = 0; i < g_dbg->module_count(); i++) {
    if (p_va >= g_dbg->module(i)->addr() &&
        p_va <  g_dbg->module(i)->addr() + g_dbg->module_count()) {
      // アドレス範囲内にありサイズの最も小さいものを選択
      if (idx == -1 ||
          g_dbg->module(i)->size() < g_dbg->module(idx)->size()) {
        idx = i;
      }
    }
  }
  if (idx != -1) {
    module_idx_ = idx;
    PostMessage(
      GetDlgItem(m_dlg->getHwnd(), IDC_CMB_MEMORY),
      CB_SETCURSEL,
      module_idx_,
      0L);
  }
}

int CodeWindow::VA2Line(u32 p_va) {
  return (VA2PA(p_va) - VA2PA(g_dbg->module(module_idx_)->addr())) >> 2;
}

// p_vaで与えられた行を選択、スクロールインして再描画する
void CodeWindow::SelectAddress(u32 p_va, bool p_redraw) {
  m_sel = VA2Line(p_va);
  ScrollTo(m_sel);
  if (p_redraw) Draw();
}

bool CodeWindow::TranslateAccelerator(MSG* p_msg) {
  return ::TranslateAccelerator(m_dlg->getHwnd(), accel_, p_msg) != 0;
}

bool CodeWindow::HasSource() {
  return src_idx_ == 0 && g_dbg->src_file_ary_.size() > 0;
}

bool CodeWindow::HasAssembler(int p_line) {
  // 対応する箇所がなければグレイ表示
  bool find = false;
  for (u32 i = 0; i < g_dbg->src_loc_ary_line_.size(); i++) {
    if (g_dbg->src_loc_ary_line_[i]->src_info_id != src_idx_ - 1) continue;
    if (p_line + 1 <  g_dbg->src_loc_ary_line_[i]->line) break;
    if (p_line + 1 == g_dbg->src_loc_ary_line_[i]->line) find = true;
  }
  return src_idx_ > 0 && find;
}

bool CodeWindow::OpenLineDialog() {
  bool result = LineEditDialog::open(m_dlg->getHwnd());
  if (result) {
    g_dbg->drawAllWindow();
  }
  return result;
}

bool CodeWindow::OpenFindInstDialog() {
  bool result = FindInstructionDialog::open(m_dlg->getHwnd());
  if (result) {
    // 自動で検索結果を表示
    SendMessage(
      GetDlgItem(
        g_dbg->reference_window()->dlg()->getHwnd(), IDC_CMB_CATEGORY),
      CB_SETCURSEL,
      ReferenceWindow::kCat_FindResult,
      0L);
    SendMessage(
      g_dbg->reference_window()->dlg()->getHwnd(),
      WM_COMMAND,
      (CBN_SELCHANGE << 16) | IDC_CMB_CATEGORY,
      0L);
    g_dbg->reference_window()->update_list();
    g_dbg->drawAllWindow();
  }
  return result;
}

bool CodeWindow::OpenAddrJumpDialog() {
  bool result = AddrJumpDialog::open(m_dlg->getHwnd());
  if (result) {
    AutoJumpModule(GetCurModule()->addr() + m_sel * 4);
    ScrollTo(m_sel);
    g_dbg->drawAllWindow();
  }
  return result;
}

bool CodeWindow::OpenBreakInfoDialog(int p_idx) {
  RECT rect;
  GetWindowRect(m_dlg->getHwnd(), &rect);
  BreakPointInfoDlgSetup(
    rect.left, rect.top - 100,
    g_dbg->get_breakpoint(p_idx));
  
  bool result = MODALDIALOG(
                  IDD_BREAKPOINT_INFO_DIALOG,
                  m_dlg->getHwnd(),
                  BreakPointInfoDlgProc) == IDOK;
  SetFocus(m_dlg->getHwnd());
  SetActiveWindow(m_dlg->getHwnd());

  if (result) {
    g_dbg->drawAllWindow();
  }
  return result;
}

void CodeWindow::handleDebugExecute(int p_ctrlID) {
  // プログラム読み込み済みで実行中で無ければ実行スレッドに遷移する
  if (GetCurModule()->size() > 0 && g_dbg->command_id() == -1) {
    g_dbg->set_command_id(p_ctrlID);
    SwitchToFiber(g_app.execute_fiber());
  }
}

void CodeWindow::handleDebugPause() {
  int saved_head = m_head;

  // 現在のPCの位置で強制停止させる
  TempCodeBreakPoint  tempBP;
  tempBP.set_va(g_cpu->m_pc);
  tempBP.breakOn();
  SwitchToFiber(g_app.execute_fiber());
  tempBP.breakOff();
  
  // 勝手にPCに移動しないほうがいい
  ScrollTo(saved_head + 4);
  Draw();
}

void CodeWindow::handleDebugToggleBreak() {
  u32 va = GetCurModule()->addr() + m_sel * 4;
  int idx = 0;
  if (g_dbg->get_breakpoint_idx(va, &idx, true) >= 0) {
    // 既にあればそれを削除
    g_dbg->delete_breakpoint(idx);
  } else {
    if (!g_dbg->get_current_ddb()) {
      MessageBox(dlg()->getHwnd(), kMsg_NoCurDDB, NULL, NULL);
      return;
    }
    // 無ければブレークポイントを作成
    g_dbg->add_breakpoint(new CodeBreakPoint(g_dbg->get_current_ddb(), va));
  }
  g_dbg->drawAllWindow();
}

void CodeWindow::handleDebugBreakDialog() {
  u32 va = GetCurModule()->addr() + m_sel * 4;
  int idx = 0;

  bool create = false;
  if (g_dbg->get_breakpoint_idx(va, &idx, true) == -1) {
    // 無ければブレークポイントを作成
    create = true;
    g_dbg->add_breakpoint(new CodeBreakPoint(g_dbg->get_current_ddb(), va));
  }
  if (OpenBreakInfoDialog(idx) == false) {
    // 新規作成してキャンセルしたら削除する
    if (create) {
      g_dbg->delete_breakpoint(idx);
    }
  }
}

void CodeWindow::handleDebugTogglePatch() {
  PatchEntry* patch =
    g_dbg->findPatch(GetCurModule()->addr() + m_sel * 4, 4, false);
  if (patch) {
    int cmd_id = g_dbg->command_id();
    // リコンパイルコードが破棄されるので一旦止める
    if (cmd_id != -1) {
      SendMessage(m_dlg->getHwnd(), WM_COMMAND, ID_DEBUG_PAUSE, 0);
    }
    if (patch->state()) {
      patch->SetDisable();
    } else {
      patch->SetEnable();
    }
    g_dbg->doStaticCodeAnalyze();
    g_dbg->drawAllWindow();

    // 再開
    if (cmd_id != -1) {
      SendMessage(m_dlg->getHwnd(), WM_COMMAND, cmd_id, 0);
    }
  }
}

void CodeWindow::handleDebugSetPC() {
  if (src_idx_ == 0) { // アセンブラ表示時
    g_cpu->m_pc = GetCurModule()->addr() + m_sel * 4;
    Draw();
    g_dbg->register_window()->gpr_update();
  }
}

void CodeWindow::handleDebugEdit() {
  if (src_idx_ == 0) { // アセンブラ表示時
    OpenLineDialog();
  }
}

void CodeWindow::handleDisplayGoDialog() {
  if (src_idx_ == 0) { // アセンブラ表示時
    OpenAddrJumpDialog();
  }
}

void CodeWindow::handleDisplayGoPC() {
  if (src_idx_ == 0) { // アセンブラ表示時
    m_sel = VA2Line(g_cpu->m_pc);
    ScrollTo(m_sel);
    Draw();
  }
}

void CodeWindow::handleDisplayGoRef() {
  if (src_idx_ == 0)  { // アセンブラ表示時
    // 分岐命令なら分岐先を参照する
    u32  va = m_sel * 4 + GetCurModule()->addr();
    u32  opcode = *((u32*)VA2PA(va));
    u32  jaddr;
    bool jump = false;

    switch (opcode & 0xfc000000) { // misc
      case 0x08000000:  // j
      case 0x0c000000:  // jal
        jump = true;
        jaddr = va + 4 & 0xf0000000 | (opcode & 0x03ffffff) << 2;
        break;
      case 0x10000000:  // beq
      case 0x14000000:  // bne
      case 0x18000000:  // blez
      case 0x1c000000:  // bgtz
      case 0x50000000:  // beql
      case 0x54000000:  // bnel
      case 0x58000000:  // blezl
      case 0x5c000000:  // bgtzl
        jump = true;
        jaddr = va + 4 + (s16)(opcode & 0xffff) * 4;
        break;
    }
    switch (opcode & 0xfc1f0000) { // regimm
      case 0x04000000:  // bltz
      case 0x04100000:  // bltzal
      case 0x04120000:  // bltzall
      case 0x04020000:  // bltzl
      case 0x04010000:  // bgez
      case 0x04110000:  // bgezal
      case 0x04130000:  // bgezall
      case 0x04030000:  // bgezl
        jump = true;
        jaddr = va + 4 + (s16)(opcode & 0xffff) * 4;
        break;
    }
    if (jump) {
      m_sel = VA2Line(jaddr);
      ScrollTo(m_sel);
      browse_log_.push_back(va);
      Draw();
    } else {
      // ジャンプ命令以外は静的解析結果にジャンプする
      AnalyzeInfoMapItr itr = g_dbg->analyze_map_.find(va);
      if (itr != g_dbg->analyze_map_.end() &&
          itr->second.valid_estimate()) {
        if (g_dbg->isInModuleCodeSection(itr->second.estimate_value())) {
          //m_sel = VA2Line(ana_inf->estimate_value());
          //ScrollTo(m_sel);
          //browse_log_.push_back(va);
          //Draw();
          AutoJumpModule(itr->second.estimate_value());
          SelectAddress(itr->second.estimate_value());
          browse_log_.push_back(va);
        } else {
          g_dbg->memory_window()->SelectAddress(itr->second.estimate_value());
        }
      }
    }
  }
}

void CodeWindow::handleDisplayGoPrev() {
  if (src_idx_ == 0) { // アセンブラ表示時
    if (browse_log_.size() >= 1) {
      u32 jaddr = browse_log_.back();
      m_sel = VA2Line(jaddr);
      ScrollTo(m_sel);
      Draw();
      browse_log_.pop_back();
    }
  }
}

void CodeWindow::handleDisplaySource() {
  u32 addr = GetCurModule()->addr() + m_sel * 4;
  for (u32 i = 0; i < g_dbg->src_loc_ary_addr_.size() - 1; i++) {
    if (addr >= g_dbg->src_loc_ary_addr_[i + 1]->addr) continue;

    if (g_dbg->src_loc_ary_addr_[i]->line == 0x7fffffff) {
      MessageBox(m_dlg->getHwnd(), "ソースはありません", NULL, NULL);
    } else {
      if (g_dbg->src_loc_ary_addr_[i]->src_info_id >= 0) {
        // スクロール位置を保存
        g_dbg->src_file_info_ary_[src_idx_]->set_scroll(m_head);

        src_idx_ = g_dbg->src_loc_ary_addr_[i]->src_info_id + 1;
        // スクロール位置復元
        m_head = g_dbg->src_file_info_ary_[src_idx_]->scroll();
        g_dbg->src_file_info_ary_[src_idx_]->set_scroll(g_dbg->src_loc_ary_addr_[i]->line - 1);

        m_sel = g_dbg->src_file_info_ary_[src_idx_]->scroll();
        SetScrollMax(GetAllLine());
        ScrollTo(g_dbg->src_file_info_ary_[src_idx_]->scroll());
        Draw();
        
        // コンボを変更
        HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_SOURCE);
        SendMessage(cmb, CB_SETCURSEL, src_idx_, 0);
        PostMessage(m_dlg->getHwnd(), WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_SOURCE, 0L);
      } else {
        char str[1024];
        sprintf(str,
          kMsg_SourceFileNotFound,
          g_dbg->src_file_ary_[g_dbg->src_loc_ary_addr_[i]->src_id]->path);
        MessageBox(m_dlg->getHwnd(), str, NULL, NULL);
      }
    }
    break;
  }
}

void CodeWindow::handleDisplayAssembly() {
  for (u32 i = 0; i < g_dbg->src_loc_ary_line_.size(); i++) {
    if (g_dbg->src_loc_ary_line_[i]->src_info_id != src_idx_ - 1) continue;
    if (g_dbg->src_loc_ary_line_[i]->line != m_sel + 1) continue;
    if (g_dbg->src_loc_ary_line_[i]->line >  m_sel + 1) break;

    // スクロール位置を保存
    g_dbg->src_file_info_ary_[src_idx_]->set_scroll(m_head);

    src_idx_ = 0;
    // スクロール位置復元
    m_head = g_dbg->src_file_info_ary_[src_idx_]->scroll();
    g_dbg->src_file_info_ary_[src_idx_]->set_scroll((g_dbg->src_loc_ary_line_[i]->addr - GetCurModule()->addr()) / 4);
    
    m_sel = g_dbg->src_file_info_ary_[src_idx_]->scroll();
    SetScrollMax(GetAllLine());
    ScrollTo(g_dbg->src_file_info_ary_[src_idx_]->scroll());
    Draw();
    
    // コンボを変更
    HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_SOURCE);
    SendMessage(cmb, CB_SETCURSEL, src_idx_, 0);
    PostMessage(m_dlg->getHwnd(), WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_SOURCE, 0L);
    break;
  }
}

void CodeWindow::handleDisplayFindRef() {
  g_dbg->reference_window()->SearchReference(GetCurModule()->addr() + m_sel * 4);
}

void CodeWindow::handleDisplayFindInst() {
  if (src_idx_ == 0) { // アセンブラ表示時
    OpenFindInstDialog();
  }
}

void CodeWindow::handleProfilerEnable(int p_ctrlID) {
  g_dbg->set_enable_profiler(!g_dbg->enable_profiler());
  CheckMenuItem(GetMenu(m_dlg->getHwnd()), p_ctrlID, g_dbg->enable_profiler() ? MF_CHECKED : MF_UNCHECKED);
}

void CodeWindow::handleProfilerSetStart() {
  g_dbg->set_profiler_start(GetCurModule()->addr() + m_sel * 4);
}

void CodeWindow::handleProfilerSetEnd() {
  g_dbg->set_profiler_end(GetCurModule()->addr() + m_sel * 4);
}

void CodeWindow::handleChangeSource() {
  // スクロール位置を保存
  g_dbg->src_file_info_ary_[src_idx_]->set_scroll(m_head);
  
  HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_SOURCE);
  src_idx_ = SendMessage(cmb, CB_GETCURSEL, 0, 0);
  // スクロール位置復元
  m_head = g_dbg->src_file_info_ary_[src_idx_]->scroll();

  // カラムの切り替え
  clearColumns();
  if (src_idx_ == 0) {
    AddCol(kText_ColStr_Addr, 52, true, NULL);
    AddCol(kText_ColStr_Opcode, 58, true, NULL);
    AddCol(kText_ColStr_Assembly, 200, false, NULL);
    AddCol(kText_ColStr_Comment, 200, false, NULL);
  } else {
    AddCol(kText_ColStr_Line, 40, false, NULL);
    AddCol(kText_ColStr_Source, 500, false, NULL);
  }

  SetScrollMax(GetAllLine());
  ScrollTo(g_dbg->src_file_info_ary_[src_idx_]->scroll());
  Draw();

  // フォーカスをダイアログにセット
  SetFocus(GetDlgItem(m_dlg->getHwnd(), ID_DEBUG_RUN));
}

void CodeWindow::handleChangeMemory() {
  s64 oldaddr = GetCurModule()->addr();

  HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_MEMORY);
  module_idx_ = SendMessage(cmb, CB_GETCURSEL, 0, 0);

  SetScrollMax(GetAllLine());

  // スクロール値を修正（アドレスを可能な範囲で保存する）
  m_head += (int)(oldaddr - (s64)GetCurModule()->addr()) / 4;
  if (m_head < 0) m_head = 0;
  if (m_head + GetPageLine() >= GetAllLine()) {
    m_head = GetAllLine() - GetPageLine();
  }
  ScrollTo(m_head);
  
  // 選択行の修正
  m_sel += (int)(oldaddr - (s64)GetCurModule()->addr()) / 4;

  Draw();

  // フォーカスをダイアログにセット
  SetFocus(GetDlgItem(m_dlg->getHwnd(), ID_DEBUG_RUN));
}

void CodeWindow::drawAssemblyOpcodeCol(HDC p_hdc,
                                       int p_line,
                                       int p_left,
                                       bool p_is_analyzed) {
  u32  va = GetCurModule()->addr() + (p_line + m_head) * 4;
  u32* pa = reinterpret_cast<u32*>(VA2PA(va));

  if (pa) {
    // パッチがあればopcodeの色を変える（複数ある場合、有効なパッチを優先）
    PatchEntry* patch = g_dbg->findPatch(va, 4, true);
    if (patch) {
      SetTextColor(p_hdc, COLOR_PATCH);
    } else {
      patch = g_dbg->findPatch(va, 4, false);
      if (patch) {
        SetTextColor(p_hdc, COLOR_PATCH_DISABLE);
      } else {
        SetTextColor(p_hdc, p_is_analyzed ? COLOR_DEFAULT : COLOR_DISABLE);
      }
    }
    s8 str[1024];
    sprintf(str, "%08x%c",
            *((u32*)pa),
            g_dbg->getMacroRecompileId(va) != -1 ? '*' : ' ');
    TextOut(p_hdc,
            p_left + 4,
            kFontSize * p_line + GetListTop(),
            str,
            strlen(str));
  }
}

void CodeWindow::drawAssemblyMacroRecIdCol(HDC p_hdc,
                                           int p_line,
                                           int p_left) {
  u32  va = GetCurModule()->addr() + (p_line + m_head) * 4;

  u32 macro_rec_id = g_dbg->getMacroRecompileId(va);
  if (macro_rec_id != -1) {
    s8 str[1024];
    sprintf(str, "%d", macro_rec_id);
    TextOut(p_hdc, p_left + 4, kFontSize * p_line + GetListTop(), str, strlen(str));
  }
}

void CodeWindow::drawAssemblyCodeCol(HDC p_hdc,
                                     int p_line,
                                     int p_left,
                                     bool p_is_analyzed) {
  u32  va = GetCurModule()->addr() + (p_line + m_head) * 4;
  u32* pa = reinterpret_cast<u32*>(VA2PA(va));

  if (pa) {
    if (p_is_analyzed) {
      s8 str[1024];
      disasm(va, *pa, str);
      ds_util::TextOutColor(p_hdc,
                            p_left + 12,
                            kFontSize * p_line + GetListTop(),
                            str,
                            COLOR_DEFAULT);
    } else {
      s8 str[1024];
      disasm_plain(va, *pa, str);
      SetTextColor(p_hdc, COLOR_DISABLE);
      TextOut(p_hdc,
              p_left + 12,
              kFontSize * p_line + GetListTop(),
              str,
              strlen(str));
    }
  }
}

void CodeWindow::drawAssemblyCommentCol(HDC p_hdc,
                                        int p_line,
                                        int p_left) {
  u32 va = GetCurModule()->addr() + (p_line + m_head) * 4;

  Label* label = g_dbg->find_codelabel(va);
  Comment* comment = g_dbg->find_comment(va);
  if (comment || label) {
    // ユーザコメントやラベルがあればそれを優先して表示
    int label_width = 0;
    if (label) {
      SetTextColor(p_hdc, label->is_cur() ? COLOR_LABEL : COLOR_LABEL2);
      TextOut(p_hdc,
              p_left + 4,
              kFontSize * p_line + GetListTop(),
              label->text(),
              strlen(label->text()));
      label_width = (strlen(label->text()) + 1) * 6;
    }
    if (comment) {
      SetTextColor(p_hdc, comment->is_cur() ? COLOR_DEFAULT : COLOR_DEFAULT2);
      TextOut(p_hdc,
              p_left + 4 + label_width,
              kFontSize * p_line + GetListTop(),
              comment->text(),
              strlen(comment->text()));
    }
  } else {
    // 実行ファイル内のデバッグ情報を代わりに表示
    s8* name = g_dbg->findSymbolTableName(va);
    if (name) {
      SetTextColor(p_hdc, COLOR_SYMBOL);
      TextOut(p_hdc,
              p_left + 4,
              kFontSize * p_line + GetListTop(),
              (s8*)name,
              strlen((s8*)name));
    }
  }
}

void CodeWindow::drawSourceLineCol(HDC p_hdc, int p_line, int p_left) {
  s8 str[256];
  sprintf(str, "@d%5d", p_line + m_head + 1);
  ds_util::TextOutColor(p_hdc,
                        p_left + 4,
                        kFontSize * p_line + GetListTop(),
                        str,
                        COLOR_DEFAULT);
}

void CodeWindow::drawSourceCodeCol(HDC p_hdc,
                                   int p_line,
                                   int p_left,
                                   bool* p_c_comment,
                                   bool* p_cpp_comment,
                                   bool* p_in_string) {
  // 対応する参照コードがあるか？
  for (u32 i = 0; i < g_dbg->src_loc_ary_line_.size(); i++) {
    if (g_dbg->src_loc_ary_line_[i]->src_info_id != src_idx_ - 1) continue;
    if (g_dbg->src_loc_ary_line_[i]->line != p_line + m_head + 1) continue;
    if (g_dbg->src_loc_ary_line_[i]->line >  p_line + m_head + 1) break;

    ds_util::TextOutColor(p_hdc,
                          p_left + 4,
                          kFontSize * p_line + GetListTop(),
                          "*",
                          COLOR_SRCMAP_SIGN);
    break;
  }

  // 可能な限りソースを修飾
  // \tをスペースＮ個に置き換える
  // コメント、文字列の色を変える
  int  tabspace = 0;
  char str[1024];
  str[1023] = '\0';
  int m = 0, k = 0;

  if (*p_c_comment || *p_cpp_comment) {
    str[k++] = '@';
    str[k++] = 'f';
  }
  while (k < 1023) {
    if (tabspace > 0) {
      str[k++] = ' ';
      tabspace--;
    } else {
      s8* ch = &g_dbg->src_file_info_ary_[src_idx_]->line_data(p_line + m_head)[m++];
      if (*ch == '\t') {
        tabspace = 4 - (k % 4);
      } else if (*ch == '/' && *(ch + 1) == '/') {
        // C++コメント
        *p_cpp_comment = true;
        str[k++] = '@';
        str[k++] = 'f';
        str[k++] = *ch;
      } else if (*ch == '/' && *(ch + 1) == '*') {
        if (!*p_cpp_comment && !*p_in_string) {
          // Cコメント開始
          *p_c_comment = true;
          str[k++] = '@';
          str[k++] = 'f';
          str[k++] = *ch;
        }
      } else if (m - 1 > 0 && *(ch - 1) == '*' && *ch == '/') {
        if (!*p_cpp_comment && !*p_in_string) {
          // Cコメント終了
          *p_c_comment = false;
          str[k++] = *ch;
          str[k++] = '@';
          str[k++] = 'w';
        }
      } else if (*ch == '\"') {
        if (!*p_cpp_comment && !*p_c_comment) {
          // 文字列
          if (*p_in_string) {
            *p_in_string = false;
            str[k++] = *ch;
            str[k++] = '@';
            str[k++] = 'w';
          } else {
            *p_in_string = true;
            str[k++] = '@';
            str[k++] = 'o';
            str[k++] = *ch;
          }
        }
      } else if (*ch == '\\' && *(ch + 1) == '\n') {
        // 行の継続
        str[k++] = '\0';
        break;
      } else if (*ch == '@') {
        // ソース内の@マーク
        str[k++] = '@';
        str[k++] = '@';
      } else {
        str[k++] = *ch;
        if (*ch == '\0') {
          // 行の終わり
          *p_cpp_comment = false;
          break;
        }
      }
    }
  }
  ds_util::TextOutColor(p_hdc,
                        p_left + 14,
                        kFontSize * p_line + GetListTop(),
                        str,
                        COLOR_DEFAULT);
}