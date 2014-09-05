#include "main.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/disasm.h"
#include "debugger/patch_entry.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/patch_window.h"
#include "debugger/gui/window/code_window.h"
#include "debugger/gui/window/memory_window.h"
#include "debugger/gui/dialog/patch_info_dialog.h"

#include <utils/ds_util.h>
#include <algorithm>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

bool Compare_Addr_Ascend(const void *p_a, const void *p_b) {
  const PatchEntry* a = static_cast<const PatchEntry*>(p_a);
  const PatchEntry* b = static_cast<const PatchEntry*>(p_b);
  return a->va() < b->va();
}
bool Compare_Addr_Descend(const void *p_a, const void *p_b) {
  return !Compare_Addr_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Addr(bool p_ascend) {
  return p_ascend ? Compare_Addr_Ascend : Compare_Addr_Descend;
}

bool Compare_Name_Ascend(const void *p_a, const void *p_b) {
  const PatchEntry* a = static_cast<const PatchEntry*>(p_a);
  const PatchEntry* b = static_cast<const PatchEntry*>(p_b);
  int scmp = strcmp(a->name(), b->name());
  if (scmp < 0) return true;
  else if (scmp > 0) return false;
  return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_Name_Descend(const void *p_a, const void *p_b) {
  return !Compare_Name_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Name(bool p_ascend) {
  return p_ascend ? Compare_Name_Ascend : Compare_Name_Descend;
}

};

PatchWindow::PatchWindow() : ListWindowBase() {
}

PatchWindow::~PatchWindow() {
  m_sorted_ary.clear();
}

void PatchWindow::Init() {
  ListWindowBase::Init(
    IDD_PATCH,
    (DLGPROC)ListWindowBase::DlgProc,
    kViewX, kViewY, kViewW, kViewH);

  HMENU menu = LoadMenu(m_dlg->getInstance(), MAKEINTRESOURCE(IDR_MENU_PATCH));
  SetMenu(m_dlg->getHwnd(), menu);

  SetScrollMax(0);

  clearColumns();
  AddCol("addr", 52, true, Compare_Addr);
  AddCol("size", 38, true, NULL);
  AddCol("state", 52, true, NULL);
  AddCol("name", 160, false, Compare_Name);
  AddCol("old", 120, false, NULL);
  AddCol("new", 120, false, NULL);

  m_head = 0;
  update_list();
}

bool PatchWindow::PreTranslateMessage(MSG* p_msg) {
  return false;
}

void PatchWindow::Draw() {
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

      if (i + m_head < GetAllLine() && m_sel == i + m_head) {
        FillRect(hdc, &selrect, hbSelect);
      } else {
        FillRect(hdc, &selrect, hbBG);
      }

      if (i + m_head < GetAllLine()) {
        PatchEntry*  patch = m_sorted_ary[i + m_head];

        switch (j) {
        case 0:
          sprintf(str, "%08x", patch->va());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 1:
          sprintf(str, "%d", patch->size());
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 2:
          strcpy(str, patch->state() ? "enable" : "disable");
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          break;
        case 3:
          TextOut(hdc, left + 4, kFontSize * i + GetListTop(), patch->name(), strlen(patch->name()));
          break;
        case 4: case 5: {
          const s8* data = NULL;
          if (j == 4) data = patch->old_data();
          if (j == 5) data = patch->new_data();

          // コード領域内ならコードウインドウに飛ぶ
          if (g_dbg->isInModuleCodeSection(patch->va())) {
            disasm(patch->va(), *((u32*)data), str);
            ds_util::TextOutColor(hdc, left + 4, kFontSize * i + GetListTop(), str, RGB(255,255,255));
          } else {
            char tmp[256];
            u32 max_bytes = patch->size();
            if (max_bytes > 128) max_bytes = 128;

            str[0] = '\0';
            for (u32 k = 0; k < max_bytes; k++) {
              sprintf(tmp, "%02x", *(data + k));
              strcat(str, tmp);
              str[(k + 1) * 2] = '\0';
            }
            TextOut(hdc, left + 4, kFontSize * i + GetListTop(), str, strlen(str));
          }
          break; }
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

int PatchWindow::GetAllLine() {
  return m_sorted_ary.size();
}

void PatchWindow::SortListData(QSORT_FUNC p_sort_func) {
  std::sort(m_sorted_ary.begin(), m_sorted_ary.end(), p_sort_func);
}

void PatchWindow::OnCommand(int p_ctrlID, int p_notify) {
  switch (p_ctrlID) {
  case ID_PATCH_EDITDLG:
    if (PatchInfoDialog::open(m_dlg->getHwnd(), m_sorted_ary[m_sel]) == IDOK) {
      g_dbg->drawAllWindow();
    }
    break;
  case ID_PATCH_DEL:
    if (m_sel < GetAllLine()) {
      for (int i = 0; i < g_dbg->getPatchCount(); i++) {
        if (g_dbg->getPatch(i) == m_sorted_ary[m_sel]) {
          g_dbg->deletePatch(i);
          break;
        }
      }
      // 静的コード解析をやり直す
      g_dbg->doStaticCodeAnalyze();
      g_dbg->drawAllWindow();
    }
    break;
  case ID_PATCH_UNION:
    if (g_dbg->unionPatch(m_sorted_ary[m_sel])) {
      m_sel--;
      g_dbg->drawAllWindow();
    } else {
      MessageBox(NULL, "直前にパッチが見つからないため統合できません。", NULL, NULL);
    }
    break;
  case ID_PATCH_TOGGLE:
    if (m_sel < GetAllLine()) {
      int cmd_id = g_dbg->command_id();
      // リコンパイルコードが破棄されるので一旦止める
      if (cmd_id != -1) {
        SendMessage(g_dbg->code_window()->dlg()->getHwnd(), WM_COMMAND, ID_DEBUG_PAUSE, 0);
      }

      if (m_sorted_ary[m_sel]->state()) {
        m_sorted_ary[m_sel]->SetDisable();
      } else {
        m_sorted_ary[m_sel]->SetEnable();
      }
      g_dbg->doStaticCodeAnalyze();
      g_dbg->drawAllWindow();

      // 再開
      if (cmd_id != -1) {
        SendMessage(g_dbg->code_window()->dlg()->getHwnd(), WM_COMMAND, cmd_id, 0);
      }
    }
    break;
  case ID_PATCH_GENERATE_BINPATCH:
    GenerateBinaryPatch();
    break;
  }
}

void PatchWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  if (p_wp) {
    HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_ADDR);
    g_dbg->setFontToFixed(cmb);
    SendMessage(cmb, CB_SETEDITSEL, 0L, MAKELONG(0, 0));    // コンボテキスト選択解除
    SetFocus(m_dlg->getHwnd());                  // フォーカスをダイアログにセット
  }
}

void PatchWindow::OnRButtonDown(int p_x, int p_y) {
  if (p_y < GetListTop()) return;

  int line = (p_y - GetListTop()) / kFontSize;

  if (line >= 0 && line <= GetPageLine()) {
    if (line < GetAllLine()) {
      m_sel = m_head + line;
      Draw();

      POINT gpt = { p_x, p_y };
      ClientToScreen(m_dlg->getHwnd(), &gpt);

      HMENU menu = CreatePopupMenu();
      AppendMenu(menu, MF_STRING | MF_ENABLED, ID_PATCH_EDITDLG, (LPCTSTR)"編集ダイアログを開く...");
      AppendMenu(menu, MF_STRING | MF_SEPARATOR, 0, (LPCTSTR)"");
      AppendMenu(menu, MF_STRING | MF_ENABLED, ID_PATCH_TOGGLE, (LPCTSTR)"Enable/Disable切り替え\t<Space>");
      AppendMenu(menu, MF_STRING | MF_ENABLED, ID_PATCH_UNION, (LPCTSTR)"直前のパッチに統合\t<+>");
      AppendMenu(menu, MF_STRING | MF_SEPARATOR, 0, (LPCTSTR)"");
      AppendMenu(menu, MF_STRING | MF_ENABLED, ID_PATCH_DEL, (LPCTSTR)"削除\t<Del>");
      TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON, gpt.x, gpt.y, 0, m_dlg->getHwnd(), NULL);
    }
  }
}

void PatchWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift) {
  switch (p_vk) {
  case VK_SPACE:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, ID_PATCH_TOGGLE, 0); break;
  case VK_DELETE:  PostMessage(m_dlg->getHwnd(), WM_COMMAND, ID_PATCH_DEL, 0); break;
  case VK_RETURN:
    if (m_sel < GetAllLine()) {
      // コード領域内ならコードウインドウに飛ぶ
      if (g_dbg->isInModuleCodeSection(m_sorted_ary[m_sel]->va())) {
        g_dbg->code_window()->AutoJumpModule(m_sorted_ary[m_sel]->va());
        g_dbg->code_window()->SelectAddress(m_sorted_ary[m_sel]->va());
      } else {
        g_dbg->memory_window()->SelectAddress(m_sorted_ary[m_sel]->va());
      }
    }
    break;
  case VK_OEM_PLUS:  // +
    if (!p_shift) break;
  case VK_ADD:    // +
    if (g_dbg->unionPatch(m_sorted_ary[m_sel])) {
      m_sel--;
      g_dbg->drawAllWindow();
    } else {
      MessageBox(NULL, "直前にパッチが見つからないため統合できません。", NULL, NULL);
    }
    break;
  case VK_UP:    CursorUp(1); break;
  case VK_PRIOR:  CursorUp(GetPageLine()); break;    // PageUp
  case VK_DOWN:  CursorDown(1); break;
  case VK_NEXT:  CursorDown(GetPageLine()); break;  // PageDown
  }
}

void PatchWindow::add_entry(PatchEntry* p_data) {
  m_sorted_ary.push_back(p_data);
  Draw();
}

void PatchWindow::remove_entry(PatchEntry* p_data) {
  for (u32 i = 0; i < m_sorted_ary.size(); i++) {
    if (m_sorted_ary[i] == p_data) {
      m_sorted_ary.erase(&m_sorted_ary[i]);
      break;
    }
  }
  Draw();
}

void PatchWindow::update_list() {
  m_sorted_ary.clear();

  if (g_dbg == NULL) return;

  int count = g_dbg->getPatchCount();
  for (int i = 0; i < count; i++) {
    m_sorted_ary.push_back(g_dbg->getPatch(i));
  }
}

void PatchWindow::GenerateBinaryPatch() {
  ds_filewindow filewindow;

  if (filewindow.savefile("pat files(*.pat)\0*.pat\0all files(*.*)\0*.*\0\0", g_elf_name, "pat")) {
    FILE* fp = fopen(filewindow.getSelectFileName(), "wb");
    if (fp) {
      for (int i = 0; i < g_dbg->getPatchCount(); i++) {
        if (g_dbg->getPatch(i)->state() == false) continue;
        u32 addr = g_dbg->getPatch(i)->va();
        fwrite(&addr, 4, 1, fp);
        u32 size = g_dbg->getPatch(i)->size();
        fwrite(&size, 4, 1, fp);
        fwrite(g_dbg->getPatch(i)->new_data(), size, 1, fp);
      }
      // サイズ0を終端とする
      u32 terminate = 0;
      fwrite(&terminate, 4, 1, fp);
      fwrite(&terminate, 4, 1, fp);

      fclose(fp);
    }
  }
}