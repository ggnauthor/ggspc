/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/memory_window.h"
#include "debugger/gui/window/breakpoint_window.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/window/reference_window.h"
#include "debugger/gui/dialog/memory_edit_dialog.h"
#include "debugger/gui/dialog/breakpoint_info_dialog.h"
#include "debugger/gui/dialog/find_binary_dialog.h"

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

/*----------*/
/* function */
/*----------*/
MemoryWindow::MemoryWindow() :
    ListWindowBase(), // TODO:???
    m_offset(0),
    m_jump_ref_addr(true),
    m_edit_low4bits(false) {
}

MemoryWindow::~MemoryWindow() {
}

void MemoryWindow::Init() {
  ListWindowBase::Init(IDD_MEMORY, (DLGPROC)ListWindowBase::DlgProc, kViewX, kViewY, kViewW, kViewH);

  HMENU menu = LoadMenu(m_dlg->getInstance(), MAKEINTRESOURCE(IDR_MENU_MEMORYWINDOW));
  SetMenu(m_dlg->getHwnd(), menu);
  
  //今のところ必要ない
  //accel_ = LoadAccelerators(m_dlg->getInstance(), MAKEINTRESOURCE(IDR_ACC_MEMORYWINDOW));
  //assert(accel_);

  SetScrollMax(GetAllLine());

  clearColumns();
  AddCol("addr", 52, true, NULL);
  AddCol("data", 255, true, NULL);
  AddCol("ascii", 120, false, NULL);
}

bool MemoryWindow::PreTranslateMessage(MSG* p_msg) {
  return false;
}

void MemoryWindow::Draw()
{
  HDC hdc = m_dlg->getBackDC();

  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);
  HBRUSH hbBreak = CreateSolidBrush(COLOR_BREAK);
  HBRUSH hbLabel = CreateSolidBrush(COLOR_MEMORY_LABEL);

  SetBkColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  g_dbg->setFontToFixed(hdc);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);
  FillRect(hdc, &clientrect, hbBG);

  DrawListFrame();

  SetTextColor(hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    if (i + m_head >= GetAllLine()) break;

    sprintf(str, "%08x", (i + m_head) *  16 + m_offset);
    TextOut(hdc, 4, kFontSize * i + GetListTop(), str, strlen(str));

    for (int j = 0; j < 16; j++) {
      u32 addr = (i + m_head) * 16 + m_offset + j;

      int offset = g_dbg->getMemLabelIdx(addr, NULL);
      if (offset != -1) {
        RECT selrect = { -2, GetListTop(), 14, kFontSize + GetListTop() };
        OffsetRect(&selrect, 54 + j * 16, i * kFontSize);
        FillRect(hdc, &selrect, hbLabel);
      }

      if (m_sel == addr) {
        RECT selrect = { -2, GetListTop(), 14, kFontSize + GetListTop() };
        OffsetRect(&selrect, 54 + j * 16, i * kFontSize);
        FillRect(hdc, &selrect, hbSelect);
      }

      int breakPos = g_dbg->get_breakpoint_idx(addr, NULL, true);
      if (breakPos >= 0) {
        RECT selrect = { -2, kFontSize + GetListTop() - 2, 14, kFontSize + GetListTop() };
        OffsetRect(&selrect, 54 + j * 16, i * kFontSize);
        FillRect(hdc, &selrect, hbBreak);

        if (breakPos == 0) {
          RECT selrect = { -2, kFontSize + GetListTop() - 8, 0, kFontSize + GetListTop() };
          OffsetRect(&selrect, 54 + j * 16, i * kFontSize);
          FillRect(hdc, &selrect, hbBreak);
        }
      }

      u8* pa = (u8*)VA2PA(addr);
      if (pa) {

        PatchEntry* patch = g_dbg->findPatch(addr, 1, true);
        if (patch) {
          SetTextColor(hdc, COLOR_PATCH);
        } else {
          patch = g_dbg->findPatch(addr, 1, false);
          if (patch) {
            SetTextColor(hdc, COLOR_PATCH_DISABLE);
          } else {
            SetTextColor(hdc, RGB(255, 255, 255));
          }
        }

        sprintf(str, "%02x", *pa);
        TextOut(hdc, 54 + 16 * j, kFontSize * i + GetListTop(), str, strlen(str));
      }
    }
    char* va = (char*)VA2PA((i + m_head) * 16 + m_offset);
    if (va) {
      for (int j = 0; j < 16; j++) {
        char str[3];
        if (IS_MB_CHAR(va[j])) {
          str[0] = va[j];
          str[1] = va[j + 1];
          str[2] = '\0';
          TextOut(hdc, 310 + 12 * j, kFontSize * i + GetListTop(), str, 2);
          j++;
        } else if (IS_ASCII(va[j])) {
          str[0] = va[j];
          str[1] = '\0';
          TextOut(hdc, 310 + 6 * j, kFontSize * i + GetListTop(), str, 1);
        } else {
          str[0] = '.';
          str[1] = '\0';
          TextOut(hdc, 310 + 6 * j, kFontSize * i + GetListTop(), str, 1);
        }
      }
    }
  }

  DeleteObject(hbBreak);
  DeleteObject(hbSelect);
  DeleteObject(hbBG);

  // 区切り線
  HPEN hpBorder = CreatePen(PS_SOLID, 1, COLOR_BORDER);
  SelectObject(hdc, hpBorder);
  for (int i = 0; i < 3; i++) {
    MoveToEx(hdc, m_col_array[0]->width + (i + 1) * 64, clientrect.top + GetListTop(), NULL);
    LineTo(hdc,   m_col_array[0]->width + (i + 1) * 64, clientrect.bottom);
  }
  DeleteObject(hpBorder);

  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

int MemoryWindow::GetAllLine()
{
  return 0x10000000;  // 0x100000000 / 16
}

// スクロール範囲を設定する。１ページに収まる場合、スクロールは非表示にする。
void MemoryWindow::SetScrollMax(int p_max_lines)
{
  m_line_count = p_max_lines;
  if (p_max_lines <= GetPageLine())
  {
    SetScrollRange(m_dlg->getHwnd(), SB_VERT, 0, 0, TRUE);
    EnableScrollBar(m_dlg->getHwnd(), SB_VERT, ESB_DISABLE_BOTH);
  } else {
    SetScrollRange(m_dlg->getHwnd(), SB_VERT, (m_offset + 15) / 16 * -1, p_max_lines - GetPageLine(), TRUE);
    EnableScrollBar(m_dlg->getHwnd(), SB_VERT, ESB_ENABLE_BOTH);
  }
}

//-------------------------------------------------------------------------------------------------
// event handler
void MemoryWindow::OnInitDialog(HWND p_hwnd)
{
  // このタイミングではMemoryWindowの初期化が完了していない。m_dlgも使えないので注意。
  HWND cmb = GetDlgItem(p_hwnd, IDC_ADDR);
  MoveWindow(cmb, 0, 1, 160, 200, true);
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"00000000(Main Memory)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"10000000(EE Reg)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"11000000(VU Reg)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"12000000(GS Reg)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"1fc00000(Boot Rom)");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"70000000(Scratch Pad)");
  SendMessage(cmb, CB_SETCURSEL, 0L, 0L);

  HWND chk = GetDlgItem(p_hwnd, IDC_JUMP_REF_ADDR);
  MoveWindow(chk, 170, 3, 150, 16, true);
}

void MemoryWindow::OnCommand(int p_ctrlID, int p_notify) {
  switch (p_ctrlID) {
  case IDC_ADDR:
    switch (p_notify) {
    case CBN_SELCHANGE:
      // 指定したアドレスへジャンプ
      HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_ADDR);
      int selidx = SendMessage(cmb, CB_GETCURSEL, 0, 0);

      char str[256];
      if (selidx != -1) {
        SendMessage(cmb, CB_GETLBTEXT, selidx, (LPARAM)str);
      } else {
        GetWindowText(cmb, str, 256);
      }
      u32 addr = ds_util::atox(str, 16);

      m_head = addr / 16;
      m_offset = addr % 16;
      SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
      // アドレスの下１桁が0で無かったらスクロール範囲+1
      SetScrollMax(GetAllLine());
      Draw();
      break;
    }
    break;
  case IDC_JUMP_REF_ADDR: {
      HWND chk = GetDlgItem(m_dlg->getHwnd(), IDC_JUMP_REF_ADDR);
      m_jump_ref_addr = (SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED);
    break; }
  case ID_DEBUG_BREAK: {
      int idx = 0;
      if (g_dbg->get_breakpoint_idx(m_sel, &idx, true) >= 0) {
        // 既にある場合は削除
        g_dbg->delete_breakpoint(idx);
      } else {
        // デフォルト値でブレークポイントを作成
        g_dbg->add_breakpoint(new MemoryBreakPoint(g_dbg->get_current_ddb(), m_sel));
      }
      Draw();
      g_dbg->breakpoint_window()->Draw();
    break; }
  case ID_DEBUG_EDITDLG:
    OpenMemoryEditDialog();
    break;
  case ID_DEBUG_DUMP:
    OpenDumpDialog();
    break;
  case ID_DISPLAY_FIND_REF:
    g_dbg->reference_window()->SearchReference(m_sel);
    break;
  case ID_DISPLAY_FIND_BINARY:
    OpenFindBinaryDialog();
    break;
  }
}

void MemoryWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  if (p_wp) {
    HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_ADDR);
    g_dbg->setFontToFixed(cmb);
    SendMessage(cmb, CB_SETEDITSEL, 0L, MAKELONG(0, 0));      // コンボテキスト選択解除
    SetFocus(m_dlg->getHwnd());                              // フォーカスをダイアログにセット
    
    HWND chk = GetDlgItem(m_dlg->getHwnd(), IDC_JUMP_REF_ADDR);
    SendMessage(chk, BM_SETCHECK, m_jump_ref_addr ? BST_CHECKED : BST_UNCHECKED, 0);

    m_edit_low4bits = false;
  }
}

void MemoryWindow::OnLButtonDown(int p_x, int p_y) {
  int line = (p_y - GetListTop()) / kFontSize;
  int word = (p_x - 54) / 16;
  if (word > 15) word = 15;
  
  if (line >= 0 && line <= GetPageLine() && word >= 0) {
    m_sel = (m_head + line) * 16 + word + m_offset;
    Draw();
  }

  // フォーカスをダイアログにセット
  SetFocus(GetDlgItem(m_dlg->getHwnd(), IDC_DUMMY));

  m_edit_low4bits = false;
}

void MemoryWindow::OnLButtonDblClk(int p_x, int p_y) {
  int line = (p_y - GetListTop()) / kFontSize;
  int word = (p_x - 54) / 16;
  if (word > 15) word = 15;

  if (line >= 0 && line <= GetPageLine() && word >= 0) {
    m_sel = (m_head + line) * 16 + word + m_offset;
    PostMessage(m_dlg->getHwnd(), WM_KEYDOWN, VK_RETURN, 0L);
  }
}

void MemoryWindow::OnRButtonDown(int p_x, int p_y) {
  if (p_y < GetListTop()) return;

  int line = (p_y - GetListTop()) / kFontSize;
  int word = (p_x - 54) / 16;
  if (word > 15) word = 15;
  
  if (line >= 0 && line <= GetPageLine() && word >= 0) {
    m_sel = (m_head + line) * 16 + word + m_offset;
    Draw();

    POINT gpt = { p_x, p_y };
    ClientToScreen(m_dlg->getHwnd(), &gpt);

    HMENU menu = CreatePopupMenu();
    AppendMenu(menu, MF_STRING | MF_ENABLED, ID_DISPLAY_FIND_REF, (LPCTSTR)"参照元を検索");
    TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON, gpt.x, gpt.y, 0, m_dlg->getHwnd(), NULL);
  }

  // フォーカスをダイアログにセット
  SetFocus(GetDlgItem(m_dlg->getHwnd(), IDC_DUMMY));

  m_edit_low4bits = false;
}

void MemoryWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {
  if (GetDlgItem(m_dlg->getHwnd(), IDC_DUMMY) == GetFocus()) {
    switch (p_vk) {
    case 'R':
      if (p_ctrl) PostMessage(m_dlg->getHwnd(), WM_COMMAND, ID_DISPLAY_FIND_REF, 0);
      break;
    case '0': case '1': case '2': case '3': case '4': case '5': case '6': case '7':
    case '8': case '9': case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
    case VK_NUMPAD0: case VK_NUMPAD1: case VK_NUMPAD2: case VK_NUMPAD3: case VK_NUMPAD4:
    case VK_NUMPAD5: case VK_NUMPAD6: case VK_NUMPAD7: case VK_NUMPAD8: case VK_NUMPAD9: {
      if (p_ctrl) {
        if (p_vk == 'F') {
          PostMessage(m_dlg->getHwnd(), WM_COMMAND, ID_DISPLAY_FIND_BINARY, 0);
        }
        break;
      }
      u8* addr = (u8*)VA2PA(m_sel);
      if (addr == NULL) break;

      int  val;
      if (p_vk >= VK_NUMPAD0) val = p_vk - VK_NUMPAD0;
      else if (p_vk >= 'A') val = 10 + p_vk - 'A';
      else if (p_vk >= '0') val = p_vk - '0';

      //PatchEntry *patch = g_dbg->findPatch(m_sel, 1, false);

      if (m_edit_low4bits) {
        s8 new_value = (*addr & 0xf0) | val;

        //if (patch) {
        //  patch->SetEnable();
        //  patch->update(0, 1, &new_value);
        //} else {
        //  g_dbg->addPatch(new PatchEntry(m_sel, 1, true, &new_value));
        //}
        *addr = new_value;

        m_edit_low4bits = false;
        PostMessage(m_dlg->getHwnd(), WM_KEYDOWN, VK_RIGHT, 0);
      } else {
        s8 new_value = (*addr & 0x0f) | (val << 4 & 0xf0);

        //if (patch) {
        //  patch->SetEnable();
        //  patch->update(0, 1, &new_value);
        //} else {
        //  g_dbg->addPatch(new PatchEntry(m_sel, 1, true, &new_value));
        //}
        *addr = new_value;

        m_edit_low4bits = true;
      }
      g_dbg->doStaticCodeAnalyze();
      g_dbg->drawAllWindow();
      break; }
    case VK_RETURN:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, ID_DEBUG_EDITDLG, 0);  break;
    case VK_F2:    PostMessage(m_dlg->getHwnd(), WM_COMMAND, p_ctrl ? ID_DEBUG_BREAKDLG : ID_DEBUG_BREAK, 0); break;
    case VK_LEFT:
    case VK_UP:
    case VK_PRIOR: {  // PageUp
      int lim = m_offset > 0 ? -16 : 0;

      s64 oldGridSel = m_sel - m_offset;
      if (oldGridSel < 0 && m_head >= GetAllLine() / 2) oldGridSel += 0x100000000;

      s64 newGridSel = oldGridSel;
      switch (p_vk) {
      case VK_LEFT:
        if (newGridSel > lim) newGridSel--;
        break;
      case VK_UP:
        newGridSel -= 16;
        break;
      case VK_PRIOR:
        newGridSel -= 16 * GetPageLine();
        break;
      }

      if (newGridSel >= lim) {
        m_sel = (int)(newGridSel + m_offset);
      } else {
        m_sel = (int)((newGridSel & 0xf) + m_offset + lim);
      }

      // スクロール
      int selrow = (int)(newGridSel / 16);
      if (selrow < m_head) m_head = selrow;
      if (m_head < lim / 16) m_head = lim / 16;
      
      SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
      Draw();

      m_edit_low4bits = false;
      break; }
    case VK_RIGHT:
    case VK_DOWN:
    case VK_NEXT: {  // PageDown
      s64 oldGridSel = m_sel - m_offset;
      if (oldGridSel < 0 && m_head >= GetAllLine() / 2) oldGridSel += 0x100000000;

      s64 newGridSel = oldGridSel;
      switch (p_vk) {
        case VK_RIGHT:
        if (newGridSel < 0xffffffff) newGridSel++;
        break;
        case VK_DOWN:
        newGridSel += 16;
        break;
        case VK_NEXT:
        newGridSel += 16 * GetPageLine();
        break;
      }

      if (newGridSel < 0x100000000) {
        m_sel = (int)(newGridSel + m_offset);
      } else {
        m_sel = (int)((0xfffffff0 | (newGridSel & 0xf)) + m_offset);
      }

      // スクロール
      // newGridSelは-15までなら負数になるので+16して計算する
      int selrow = (int)((newGridSel + 16) / 16 - 1);
      if (m_head + GetPageLine() <= selrow) {
        m_head = selrow - GetPageLine() + 1;
      }
      if (m_head > 0x10000000 - GetPageLine()) {
        m_head = 0x10000000 - GetPageLine();
      }
      SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
      Draw();

      m_edit_low4bits = false;
      break; }
    }
  } else if (GetDlgItem(m_dlg->getHwnd(), IDC_ADDR) == GetParent(GetFocus())) {
    // コンボボックス内のエディットボックスは親コントロールをチェックする
    switch (p_vk) {
    case VK_RETURN:
      SendMessage(m_dlg->getHwnd(), WM_COMMAND, MAKELONG(IDC_ADDR, CBN_SELCHANGE), 0L);
      break;
    }
  }
}

void MemoryWindow::ShowCurrentAddress() {
  ScrollTo((u32)m_sel / 16);
  Draw();
}

// p_vaで与えられた行を選択、スクロールインして再描画する
void MemoryWindow::SelectAddress(u32 p_va, bool p_redraw) {
  m_sel = p_va;
  if (p_redraw) {
    ScrollTo((u32)m_sel / 16);
    ShowCurrentAddress();
  }
}

void MemoryWindow::OpenMemoryEditDialog() {
  if (MODALDIALOG(IDD_MEMEDIT_DIALOG, m_dlg->getHwnd(), MemEditDlgProc) == IDOK) {
    g_dbg->drawAllWindow();
  }
}

void MemoryWindow::OpenDumpDialog() {
  if (MODALDIALOG(IDD_MEMDUMP_DIALOG, m_dlg->getHwnd(), MemDumpDlgProc) == IDOK) {
  }
}

void MemoryWindow::OpenFindBinaryDialog() {
  if (MODALDIALOG(IDD_FINDBINARY_DIALOG, m_dlg->getHwnd(), FindBinaryDialog::dlgProc) == IDOK) {
    // 自動で検索結果を表示
    SendMessage(GetDlgItem(g_dbg->reference_window()->dlg()->getHwnd(), IDC_CMB_CATEGORY), CB_SETCURSEL, ReferenceWindow::kCat_FindResult, 0L);
    SendMessage(g_dbg->reference_window()->dlg()->getHwnd(), WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_CATEGORY, 0L);
    g_dbg->reference_window()->update_list();
    g_dbg->drawAllWindow();
  }
}
