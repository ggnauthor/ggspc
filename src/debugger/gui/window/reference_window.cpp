#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/disasm.h"
#include "debugger/reference_entry.h"
#include "debugger/analyze_info.h"
#include "debugger/gui/color_def.h"
#include "debugger/gui/window/reference_window.h"
#include "debugger/gui/window/memory_window.h"
#include "debugger/gui/window/code_window.h"

#include <utils/ds_util.h>
#include <algorithm>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

bool Compare_Addr_Ascend(const void *p_a, const void *p_b) {
  const ReferenceEntryBase* a = static_cast<const ReferenceEntryBase*>(p_a);
  const ReferenceEntryBase* b = static_cast<const ReferenceEntryBase*>(p_b);
  return a->va() < b->va();
}
bool Compare_Addr_Descend(const void *p_a, const void *p_b) {
  return !Compare_Addr_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Addr(bool p_ascend) {
  return p_ascend ? Compare_Addr_Ascend : Compare_Addr_Descend;
}

bool Compare_Text_Ascend(const void *p_a, const void *p_b) {
  const ReferenceEntryBase* a = static_cast<const ReferenceEntryBase*>(p_a);
  const ReferenceEntryBase* b = static_cast<const ReferenceEntryBase*>(p_b);
  int scmp = strcmp(a->text(), b->text());
  if (scmp < 0) return true;
  else if (scmp > 0) return false;
  return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_Text_Descend(const void *p_a, const void *p_b) {
  return !Compare_Text_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Text(bool p_ascend) {
  return p_ascend ? Compare_Text_Ascend : Compare_Text_Descend;
}

bool Compare_Label_Ascend(const void *p_a, const void *p_b) {
  const ReferenceEntryBase* a = static_cast<const ReferenceEntryBase*>(p_a);
  const ReferenceEntryBase* b = static_cast<const ReferenceEntryBase*>(p_b);
  Label* la = g_dbg->find_codelabel(a->va());
  Label* lb = g_dbg->find_codelabel(b->va());
  if (!la && !lb) goto _same_label;
  if (!la) goto _less;
  if (!lb) goto _not_less;
  int scmp = strcmp(la->text(), lb->text());
  if (scmp < 0) goto _less;
  else if (scmp == 0) goto _same_label;
_not_less:   return false;
_less:       return true;
_same_label: return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_Label_Descend(const void *p_a, const void *p_b) {
  return !Compare_Label_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Label(bool p_ascend) {
  return p_ascend ? Compare_Label_Ascend : Compare_Label_Descend;
}

bool Compare_Comment_Ascend(const void *p_a, const void *p_b) {
  const ReferenceEntryBase* a = static_cast<const ReferenceEntryBase*>(p_a);
  const ReferenceEntryBase* b = static_cast<const ReferenceEntryBase*>(p_b);
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

bool Compare_MemLabel_Ascend(const void *p_a, const void *p_b) {
  const FindItem* a = static_cast<const FindItem*>(p_a);
  const FindItem* b = static_cast<const FindItem*>(p_b);
  int ia, ib;
  int ofsa = g_dbg->getMemLabelIdx(a->va(), &ia);
  int ofsb = g_dbg->getMemLabelIdx(b->va(), &ib);
  if (ofsa == -1 && ofsb == -1) goto _same_label;
  if (ofsa == -1) goto _less;
  if (ofsb == -1) goto _not_less;
  int scmp = strcmp(g_dbg->getMemLabel(ia)->text(), g_dbg->getMemLabel(ib)->text());
  if (scmp < 0) goto _less;
  else if (scmp == 0) goto _same_label;
_not_less:   return false;
_less:       return true;
_same_label: return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_MemLabel_Descend(const void *p_a, const void *p_b) {
  return !Compare_MemLabel_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_MemLabel(bool p_ascend) {
  return p_ascend ? Compare_MemLabel_Ascend : Compare_MemLabel_Descend;
}

bool Compare_MemComment_Ascend(const void *p_a, const void *p_b) {
  const FindItem* a = static_cast<const FindItem*>(p_a);
  const FindItem* b = static_cast<const FindItem*>(p_b);
  int ia, ib;
  int ofsa = g_dbg->getMemLabelIdx(a->va(), &ia);
  int ofsb = g_dbg->getMemLabelIdx(b->va(), &ib);
  if (ofsa == -1 && ofsb == -1) goto _same_comment;
  if (ofsa == -1) goto _less;
  if (ofsb == -1) goto _not_less;
  int scmp = strcmp(g_dbg->getMemLabel(ia)->comment(), g_dbg->getMemLabel(ib)->comment());
  if (scmp < 0) goto _less;
  else if (scmp == 0) goto _same_comment;
_not_less:     return false;
_less:         return true;
_same_comment: return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_MemComment_Descend(const void *p_a, const void *p_b) {
  return !Compare_MemComment_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_MemComment(bool p_ascend) {
  return p_ascend ? Compare_MemComment_Ascend : Compare_MemComment_Descend;
}

bool Compare_FindItemLabel_Ascend(const void *p_a, const void *p_b) {
  const FindItem* a = static_cast<const FindItem*>(p_a);
  const FindItem* b = static_cast<const FindItem*>(p_b);
  Label *la, *lb;
  if (a->location() == FindItem::kLocation_Code) {
    la = g_dbg->find_codelabel(a->va());
  } else {
    int idx;
    int ofs = g_dbg->getMemLabelIdx(a->va(), &idx);
    la = (ofs >= 0) ? g_dbg->getMemLabel(idx) : NULL;
  }
  if (b->location() == FindItem::kLocation_Code) {
    lb = g_dbg->find_codelabel(b->va());
  } else {
    int idx;
    int ofs = g_dbg->getMemLabelIdx(b->va(), &idx);
    lb = (ofs >= 0) ? g_dbg->getMemLabel(idx) : NULL;
  }
  if (!la && !lb) goto _same_label;
  if (!la) goto _less;
  if (!lb) goto _not_less;
  int scmp = strcmp(la->text(), lb->text());
  if (scmp < 0) goto _less;
  else if (scmp == 0) goto _same_label;
_not_less:   return false;
_less:       return true;
_same_label: return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_FindItemLabel_Descend(const void *p_a, const void *p_b) {
  return !Compare_FindItemLabel_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_FindItemLabel(bool p_ascend) {
  return p_ascend ? Compare_FindItemLabel_Ascend : Compare_FindItemLabel_Descend;
}

bool Compare_FindItemComment_Ascend(const void *p_a, const void *p_b) {
  const FindItem* a = static_cast<const FindItem*>(p_a);
  const FindItem* b = static_cast<const FindItem*>(p_b);
  const char *sa, *sb;
  if (a->location() == FindItem::kLocation_Code) {
    Comment* comment = g_dbg->find_comment(a->va());
    sa = comment ? comment->text() : NULL;
  } else {
    int idx;
    int ofs = g_dbg->getMemLabelIdx(a->va(), &idx);
    sa = (ofs >= 0) ? g_dbg->getMemLabel(idx)->comment() : NULL;
  }
  if (b->location() == FindItem::kLocation_Code) {
    Comment* comment = g_dbg->find_comment(b->va());
    sb = comment ? comment->text() : NULL;
  } else {
    int idx;
    int ofs = g_dbg->getMemLabelIdx(b->va(), &idx);
    sb = (ofs >= 0) ? g_dbg->getMemLabel(idx)->comment() : NULL;
  }
  if (!sa && !sa) goto _same_string;
  if (!sa) goto _less;
  if (!sb) goto _not_less;
  int scmp = strcmp(sa, sb);
  if (scmp < 0) goto _less;
  else if (scmp == 0) goto _same_string;
_not_less:    return false;
_less:        return true;
_same_string: return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_FindItemComment_Descend(const void *p_a, const void *p_b) {
  return !Compare_FindItemComment_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_FindItemComment(bool p_ascend) {
  return p_ascend ? Compare_FindItemComment_Ascend : Compare_FindItemComment_Descend;
}

bool Compare_Overhead_Ascend(const void *p_a, const void *p_b) {
  const BottleNeck* a = static_cast<const BottleNeck*>(p_a);
  const BottleNeck* b = static_cast<const BottleNeck*>(p_b);
  if (a->overhead() < b->overhead()) return true;
  else if (a->overhead() > b->overhead()) return false;
  return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_Overhead_Descend(const void *p_a, const void *p_b) {
  return !Compare_Overhead_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Overhead(bool p_ascend) {
  return p_ascend ? Compare_Overhead_Ascend : Compare_Overhead_Descend;
}

bool Compare_Disasm_Ascend(const void *p_a, const void *p_b) {
  const BottleNeck* a = static_cast<const BottleNeck*>(p_a);
  const BottleNeck* b = static_cast<const BottleNeck*>(p_b);
  s8 sa[1024], sb[1024];
  disasm(a->va(), *((u32*)VA2PA(a->va())), sa);
  disasm(b->va(), *((u32*)VA2PA(b->va())), sb);
  int scmp = strcmp(sa, sb);
  if (scmp < 0) return true;
  else if (scmp > 0) return false;
  return Compare_Addr_Ascend(p_a, p_b);
}
bool Compare_Disasm_Descend(const void *p_a, const void *p_b) {
  return !Compare_Disasm_Ascend(p_a, p_b);
}
QSORT_FUNC Compare_Disasm(bool p_ascend) {
  return p_ascend ? Compare_Disasm_Ascend : Compare_Disasm_Descend;
}

};

ReferenceWindow::ReferenceWindow() : ListWindowBase() {
  m_category = kCat_Comment;
  m_show_cur_only = true;
}

ReferenceWindow::~ReferenceWindow() {
  m_sorted_ary.clear();
  m_filtered_ary.clear();
}

void ReferenceWindow::Init() {
  ListWindowBase::Init(IDD_REFERENCE, (DLGPROC)ListWindowBase::DlgProc, kViewX, kViewY, kViewW, kViewH);
  SetScrollMax(0);
}

bool ReferenceWindow::PreTranslateMessage(MSG* p_msg) {
  return false;
}

void ReferenceWindow::Draw() {
  SetScrollMax(GetAllLine());

  HDC hdc = m_dlg->getBackDC();

  SetTextColor(hdc, RGB(255, 255, 255));
  SetBkColor(hdc, RGB(0, 0, 0));
  SetBkMode(hdc, TRANSPARENT);
  g_dbg->setFontToFixed(hdc);

  switch (m_category) {
  case kCat_Comment:    Draw_Comment(hdc); break;
  case kCat_Label:    Draw_Label(hdc); break;
  case kCat_MemLabel:    Draw_MemLabel(hdc); break;
  case kCat_String:    Draw_String(hdc); break;
  case kCat_FindResult:  Draw_FindResult(hdc); break;
  case kCat_BottleNeck:  Draw_BottleNeck(hdc); break;
  }

  DrawListFrame();

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);
  m_dlg->update(0, GetListHeaderTop(), clientrect.right, clientrect.bottom);
}

int ReferenceWindow::GetAllLine() {
  if (g_dbg) {
    if (m_category == kCat_MemLabel) {
      return GetExpandedRowCount();
    } else {
      return m_filtered_ary.size();
    }
  }
  return 0;
}

void ReferenceWindow::SortListData(QSORT_FUNC p_sort_func) {
  std::sort(m_sorted_ary.begin(), m_sorted_ary.end(), p_sort_func);
  update_filter();
}

void ReferenceWindow::Draw_Comment(HDC p_hdc) {
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);
  HBRUSH hbBreak = CreateSolidBrush(COLOR_BREAK);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    s32 bottom = kFontSize * i + GetListTop();
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = GetListTop() + i * kFontSize;
      selrect.bottom = GetListTop() + (i + 1) * kFontSize;
      selrect.left   = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size() && m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size()) {
        Comment* comment = dynamic_cast<Comment*>(m_filtered_ary[m_head + i]);
        assert(comment);

        switch (j) {
          case 0:
            DrawAddress(p_hdc, &selrect, comment->va());
            break;
          case 1: {
              Label* label = g_dbg->find_codelabel(comment->va());
              if (label) TextOut(p_hdc, left + 4, bottom,
                                 label->text(), strlen(label->text()));
          } break;
          case 2:
            disasm(comment->va(), *((u32*)VA2PA(comment->va())), str);
            ds_util::TextOutColor(p_hdc, left + 4, bottom,
                                  str, RGB(255,255,255));
            break;
          case 3:
            TextOut(p_hdc, left + 4, bottom,
                    comment->text(), strlen(comment->text()));
            break;
        }
      }
      left += m_col_array[j]->width;
    }
  }
  DeleteObject(hbBreak);
  DeleteObject(hbSelect);
  DeleteObject(hbBG);
}

void ReferenceWindow::Draw_Label(HDC p_hdc)
{
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  SetTextColor(p_hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    s32 bottom = kFontSize * i + GetListTop();
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size() &&
          m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size()) {
        Label* label = dynamic_cast<Label*>(m_filtered_ary[m_head + i]);
        assert(label);

        switch (j) {
        case 0:
          DrawAddress(p_hdc, &selrect, label->va());
          break;
        case 1:
          TextOut(p_hdc, left + 4, bottom,
                  label->text(), strlen(label->text()));
          break;
        case 2:
          disasm(label->va(), *((u32*)VA2PA(label->va())), str);
          ds_util::TextOutColor(p_hdc, left + 4, bottom,
                                str, RGB(255,255,255));
          break;
        case 3:
          {
            Comment* comment = g_dbg->find_comment(label->va());
            if (comment) TextOut(p_hdc, left + 4, bottom,
                                 comment->text(), strlen(comment->text()));
          }
          break;
        }
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hbSelect);
  DeleteObject(hbBG);
}

void ReferenceWindow::Draw_MemLabel(HDC p_hdc) {
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);
  HBRUSH hbWhite = (HBRUSH)GetStockObject(WHITE_BRUSH);
  
  HPEN hpBorder = CreatePen(PS_SOLID, 1, BLACK_BRUSH);
  SelectObject(p_hdc, hpBorder);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  SetTextColor(p_hdc, RGB(255, 255, 255));

  int expanded_row_count = GetExpandedRowCount();

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    s32 bottom = kFontSize * i + GetListTop();
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;
      
      if (i + m_head < expanded_row_count && m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (i + m_head < expanded_row_count) {
        int idx, ary_idx, elem_idx;
        int row_type = GetExpandedRowIndex(i + m_head, &idx, &ary_idx, &elem_idx);
        //if (idx == -1) break;

        MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);
        
        if (row_type == kRowType_Root) {
          switch (j)
          {
          case 0:
            sprintf(str, "%08x", memlabel->va());
            TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            break;
          case 1:
            if (memlabel->element_size() > 1 || memlabel->count() > 1) {
              DrawTreeBtn(p_hdc, left, bottom, memlabel->expand());
            }
            TextOut(p_hdc, left + 14, bottom, memlabel->text(), strlen(memlabel->text()));
            break;
          case 2:
            _itoa(memlabel->GetSize(), str, 10);
            TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            break;
          case 3:
            if (memlabel->count() == 1) {
              if (memlabel->element_size() == 1) {
                // 要素もメンバも１つのときは展開させずに同一行に表示
                const char* type_str = Debugger::TYPE_DEF[memlabel->element(0)->type].type_str;
                TextOut(p_hdc, left + 4, bottom, type_str, strlen(type_str));
              }
            } else {
              // 要素が複数のときは要素数を表示
              sprintf(str, "[%d]", memlabel->count());
              TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            }
            break;
          case 4:
            if (memlabel->count() > 1 || memlabel->element_size() > 1) {
              str[0] = '\0';
              u8* p = (u8*)VA2PA(memlabel->va());
              int data_size = memlabel->GetSize();
              for (int k = 0; k < data_size; k++)
              {
                if ((k + 1) * 2 >= 1024) break;

                char tmp[4];
                sprintf(tmp, "%02x", *p++);
                strcat(str, tmp);
              }
              TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            } else {
              // エレメントは必ず存在するとは限らない。作成直後には存在していない
              if (memlabel->element_size() != 0) {
                // 要素もメンバも１つのときは直接表示
                MemLabel::ConvertElementData(str, 1024, (u8*)VA2PA(memlabel->va()), memlabel->element(0)->type, memlabel->element(0)->size);
                TextOut(p_hdc, left + 4, bottom, str, strlen(str));
              }
            }
            break;
          case 5:
            TextOut(p_hdc, left + 4, bottom, memlabel->comment(), strlen(memlabel->comment()));
            break;
          }
        } else if (row_type == kRowType_Array) { // 配列要素
          int struct_size = memlabel->GetStructureSize();
          switch (j) {
            case 0: break;
            case 1:
              if (memlabel->element_size() > 1) {
                DrawTreeBtn(p_hdc, left + 10, bottom, memlabel->expand_ary(ary_idx));
              }
              sprintf(str, "[%d]", ary_idx);
              TextOut(p_hdc, left + 24, bottom, str, strlen(str));
              break;
            case 2:
              _itoa(struct_size, str, 10);
              TextOut(p_hdc, left + 4, bottom, str, strlen(str));
              break;
            case 3:
              if (memlabel->element_size() == 1) {
                // メンバが１つのときは展開させずに同一行に表示
                const char* type_str = Debugger::TYPE_DEF[memlabel->element(0)->type].type_str;
                TextOut(p_hdc, left + 4, bottom, type_str, strlen(type_str));
              }
              break;
            case 4:
              if (memlabel->element_size() > 1) {
                str[0] = '\0';
                u8* p = (u8*)VA2PA(memlabel->va() + struct_size * ary_idx);
                int data_size = struct_size;
                for (int k = 0; k < data_size; k++)
                {
                  char tmp[4];
                  sprintf(tmp, "%02x", *p++);
                  strcat(str, tmp);
                }
                TextOut(p_hdc, left + 4, bottom, str, strlen(str));
              } else {
                // メンバが１つのときは直接表示
                MemLabel::ConvertElementData(str, 1024, (u8*)VA2PA(memlabel->va() + struct_size * ary_idx), memlabel->element(0)->type, memlabel->element(0)->size);
                TextOut(p_hdc, left + 4, bottom, str, strlen(str));
              }
              break;
          }
        }
        else if (row_type == kRowType_Element)
        {  // 構造体要素
          int struct_size = memlabel->GetStructureSize();

          // メンバのオフセットを計算
          int offset = 0;
          for (int k = 0; k < elem_idx; k++) {
            offset += memlabel->element(k)->size;
          }

          MemLabelElement* mem_elm = memlabel->element(elem_idx);
          switch (j)
          {
          case 0: break;
          case 1:
            TextOut(p_hdc, left + 34, bottom, mem_elm->label, strlen(mem_elm->label));
            break;
          case 2:
            _itoa(mem_elm->size, str, 10);
            TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            break;
          case 3:
            TextOut(p_hdc, left + 4, bottom, Debugger::TYPE_DEF[mem_elm->type].type_str, strlen(Debugger::TYPE_DEF[mem_elm->type].type_str));
            break;
          case 4:
            {
              assert(ary_idx >= 0);
              u8* data = (u8*)VA2PA(memlabel->va() + struct_size * ary_idx + offset);

              MemLabel::ConvertElementData(str, 1024, data, mem_elm->type, mem_elm->size);
              TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            }
            break;
          }
        }
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hpBorder);
  DeleteObject(hbWhite);
  DeleteObject(hbSelect);
  DeleteObject(hbBG);
}

void ReferenceWindow::Draw_String(HDC p_hdc)
{
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  SetTextColor(p_hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    s32 bottom = kFontSize * i + GetListTop();
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size() && m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size()) {
        String* string = dynamic_cast<String*>(m_filtered_ary[m_head + i]);
        assert(string);

        switch (j) {
          case 0:
            sprintf(str, "%08x", string->va());
            TextOut(p_hdc, left + 4, bottom, str, strlen(str));
            break;
          case 1:
            TextOut(p_hdc, left + 4, bottom, string->text(), strlen(string->text()));
            break;
        }
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hbSelect);
  DeleteObject(hbBG);
}

void ReferenceWindow::Draw_FindResult(HDC p_hdc) {
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  SetTextColor(p_hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    s32 bottom = kFontSize * i + GetListTop();
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = GetListTop() + i * kFontSize;
      selrect.bottom = GetListTop() + (i + 1) * kFontSize;
      selrect.left   = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size() &&
          m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size()) {
        FindItem::ELocation loc = dynamic_cast<FindItem*>(m_filtered_ary[m_head + i])->location();
        u32  va  = m_filtered_ary[m_head + i]->va();
        switch (j) {
          case 0:
            DrawAddress(p_hdc, &selrect, va);
            break;
          case 1:
            strcpy(str, loc == FindItem::kLocation_Code ? "code" : "memory");
            TextOut(p_hdc, left + 4, bottom,
                    str, strlen(str));
            break;
          case 2:
            if (loc == FindItem::kLocation_Code) {
              disasm(va, *((u32*)VA2PA(va)), str);
            } else {
              sprintf(str, "%08x", *((u32*)VA2PA(va)));
            }
            ds_util::TextOutColor(p_hdc, left + 4, bottom,
                                  str, RGB(255,255,255));
            break;
          case 3:
            if (loc == FindItem::kLocation_Code) {
              Label* label = g_dbg->find_codelabel(va);
              if (label) {
                TextOut(p_hdc, left + 4, bottom,
                        label->text(), strlen(label->text()));
              }
            } else {
              int idx;
              int offset = g_dbg->getMemLabelIdx(va, &idx);
              if (offset != -1) {
                MemLabel* mem_label = g_dbg->getMemLabel(idx);
                if (mem_label) {
                  TextOut(p_hdc, left + 4, bottom,
                          mem_label->text(), strlen(mem_label->text()));
                }
              }
            }
            break;
          case 4:
            if (loc == FindItem::kLocation_Code) {
              Comment* comment = g_dbg->find_comment(va);
              if (comment) {
                TextOut(p_hdc, left + 4, bottom,
                        comment->text(), strlen(comment->text()));
              }
            } else {
              int idx;
              int offset = g_dbg->getMemLabelIdx(va, &idx);
              if (offset != -1) {
                MemLabel* mem_label = g_dbg->getMemLabel(idx);
                if (mem_label) {
                  TextOut(p_hdc, left + 4, bottom,
                          mem_label->comment(), strlen(mem_label->comment()));
                }
              }
            }
            break;
        }
      }
      left += m_col_array[j]->width;
    }
  }
  DeleteObject(hbSelect);
  DeleteObject(hbBG);
}

void ReferenceWindow::Draw_BottleNeck(HDC p_hdc) {
  HBRUSH hbBG = (HBRUSH)CreateSolidBrush(COLOR_BG);
  HBRUSH hbSelect = CreateSolidBrush(COLOR_SELECT);

  RECT clientrect;
  GetClientRect(m_dlg->getHwnd(), &clientrect);

  SetTextColor(p_hdc, RGB(255, 255, 255));

  char str[1024];
  for (int i = 0; i < GetPageLine() + 1; i++) {
    s32 bottom = kFontSize * i + GetListTop();
    int left = 0;
    for (u32 j = 0; j < m_col_array.size(); j++) {
      RECT selrect;
      selrect.top    = i * kFontSize + GetListTop();
      selrect.bottom  = (i + 1) * kFontSize + GetListTop();
      selrect.left  = left + 1;
      selrect.right  = left + m_col_array[j]->width;

      // 最後はウィンドウの端まで
      if (j == m_col_array.size() - 1) selrect.right = clientrect.right;

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size() && m_sel == i + m_head) {
        FillRect(p_hdc, &selrect, hbSelect);
      } else {
        FillRect(p_hdc, &selrect, hbBG);
      }

      if (static_cast<u32>(i + m_head) < m_filtered_ary.size()) {
        int overhead = dynamic_cast<BottleNeck*>(m_filtered_ary[m_head + i])->overhead();
        u32  va  = m_filtered_ary[m_head + i]->va();
        switch (j) {
        case 0:
          DrawAddress(p_hdc, &selrect, va);
          break;
        case 1:
          sprintf(str, "%d", dynamic_cast<BottleNeck*>(m_filtered_ary[m_head + i])->overhead());
          TextOut(p_hdc, left + 4, bottom, str, strlen(str));
          break;
        case 2:
          sprintf(str, "%08x", *((u32*)VA2PA(va)));
          TextOut(p_hdc, left + 4, bottom, str, strlen(str)); // 6 * 9
          break;
        case 3:
          disasm(va, *((u32*)VA2PA(va)), str);
          ds_util::TextOutColor(p_hdc, left + 4, bottom,
                                str, RGB(255,255,255));
          break;
        case 4:
          Comment* comment = g_dbg->find_comment(va);
          if (comment) TextOut(p_hdc, left + 4, bottom,
                               comment->text(), strlen(comment->text()));
          break;
        }
      }
      left += m_col_array[j]->width;
    }
  }

  DeleteObject(hbSelect);
  DeleteObject(hbBG);
}

//-------------------------------------------------------------------------------------------------
// event handler
void ReferenceWindow::OnCommand(int p_ctrlID, int p_notify) {
  switch (p_ctrlID) {
    case IDC_CMB_CATEGORY:
      switch (p_notify) {
        case CBN_SELCHANGE:
          OnCategoryComboSelChange();
          break;
      }
      break;
    case IDC_EDT_FILTER:
      if (p_notify == EN_CHANGE) {
        m_head = 0;  // 項目が見えなくなることがあるのでスクロールを先頭に
        update_list();
        Draw();
        break;
      }
      break;
    case IDC_CHK_CUR_ONLY: {
      HWND chk = GetDlgItem(m_dlg->getHwnd(), IDC_CHK_CUR_ONLY);
      m_show_cur_only = (SendMessage(chk, BM_GETCHECK, 0, 0) == BST_CHECKED);
      m_head = 0;  // 項目が見えなくなることがあるのでスクロールを先頭に
      update_list();
      Draw();
      } break;
    case kPopupMenu_SearchReference:
      SearchReference(m_filtered_ary[m_sel]->va());
      break;
    case kPopupMenu_ShowPointer:
      ShowPointer();
      break;
  }
}

void ReferenceWindow::OnCategoryComboSelChange() {
  HWND cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_CATEGORY);
  m_category = SendMessage(cmb, CB_GETCURSEL, 0L, 0L);
  
  clearColumns();
  switch (m_category) {
    case ReferenceWindow::kCat_Comment:
      AddCol("addr", 52, true, Compare_Addr);
      AddCol("label", 100, false, Compare_Label);
      AddCol("disasm", 150, false, NULL);
      AddCol("comment", 400, false, Compare_Text);
      break;
    case ReferenceWindow::kCat_Label:
      AddCol("addr", 52, true, Compare_Addr);
      AddCol("label", 100, false, Compare_Text);
      AddCol("disasm", 150, false, NULL);
      AddCol("comment", 400, false, Compare_Comment);
      break;
    case ReferenceWindow::kCat_MemLabel:
      AddCol("addr", 52, true, Compare_Addr);
      AddCol("label", 150, false, Compare_MemLabel);
      AddCol("size", 50, true, NULL);
      AddCol("type", 50, true, NULL);
      AddCol("data", 150, false, NULL);
      AddCol("comment", 200, false, Compare_MemComment);
      break;
    case ReferenceWindow::kCat_String:
      AddCol("addr", 52, true, Compare_Addr);
      AddCol("text", 600, false, Compare_Text);
      break;
    case ReferenceWindow::kCat_FindResult:
      AddCol("addr", 52, true, Compare_Addr);
      AddCol("location", 56, true, NULL);
      AddCol("disasm/data", 250, false, NULL);
      AddCol("label", 150, false, Compare_FindItemLabel);
      AddCol("comment", 150, false, Compare_FindItemComment);
      break;
    case ReferenceWindow::kCat_BottleNeck:
      AddCol("addr", 52, true, Compare_Addr);
      AddCol("overhead", 68, true, Compare_Overhead);
      AddCol("opcode", 52, true, NULL);
      AddCol("disasm", 250, false, Compare_Disasm);
      AddCol("comment", 150, false, NULL);
      break;
  }
  SetScrollMax(m_filtered_ary.size());

  m_head = 0;
  SetScrollPos(m_dlg->getHwnd(), SB_VERT, m_head, true);
  
  update_list();
  Draw();
}

void ReferenceWindow::OnShowWindow(WPARAM p_wp, LPARAM p_lp) {
  HWND cmb;
  cmb = GetDlgItem(m_dlg->getHwnd(), IDC_CMB_CATEGORY);
  for (int i = SendMessage(cmb, CB_GETCOUNT, 0, 0) - 1; i >= 0; i--) SendMessage(cmb, CB_DELETESTRING, 0, 0);
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Comment");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Code Label");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Memory Label");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"String");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Find Result");
  SendMessage(cmb, CB_ADDSTRING, 0L, (LPARAM)"Bottle Neck");
  SendMessage(cmb, CB_SETCURSEL, 0L, 0L);
  SendMessage(m_dlg->getHwnd(), WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_CATEGORY, 0L);

  HWND chk = GetDlgItem(m_dlg->getHwnd(), IDC_CHK_CUR_ONLY);
  SendMessage(chk, BM_SETCHECK, m_show_cur_only ? BST_CHECKED : BST_UNCHECKED, 0);

  SetFocus(m_dlg->getHwnd());  // フォーカスをダイアログにセット
}

void ReferenceWindow::OnLButtonDown(int p_x, int p_y) {
  if (m_category == ReferenceWindow::kCat_MemLabel) {
    int left = 0;
    for (int i = 0; i < 1; i++) {
      left += m_col_array[i]->width;
    }

    RECT clientrect;
    GetClientRect(m_dlg->getHwnd(), &clientrect);

    int expanded_row_count = GetExpandedRowCount();
    for (int i = 0; i < GetPageLine() + 1; i++) {
      if (i + m_head >= expanded_row_count) continue;
      
      int idx, ary_idx, elem_idx;
      int row_type = GetExpandedRowIndex(i + m_head, &idx, &ary_idx, &elem_idx);
      MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);

      if (row_type == kRowType_Root) {
        if (memlabel->element_size() > 1 || memlabel->count() > 1) {
          if (IsPointTreeBtn(left, kFontSize * i + GetListTop(), p_x, p_y)) {
            memlabel->set_expand(!memlabel->expand());
            break;
          }
        }
      } else if (row_type == kRowType_Array) {
        if (memlabel->element_size() > 1) {
          if (IsPointTreeBtn(left + 10, kFontSize * i + GetListTop(), p_x, p_y)) {
            memlabel->set_expand_ary(ary_idx, !memlabel->expand_ary(ary_idx));
            break;
          }
        }
      }
    }
  }
  ListWindowBase::OnLButtonDown(p_x, p_y);
}

void ReferenceWindow::OnRButtonDown(int p_x, int p_y) {
  if (p_y < GetListTop()) return;

  int line = (p_y - GetListTop()) / kFontSize;

  if (line >= 0 && line <= GetPageLine()) {
    if (line < GetAllLine()) {
      m_sel = m_head + line;
      Draw();

      POINT gpt = { p_x, p_y };
      ClientToScreen(m_dlg->getHwnd(), &gpt);

      if (m_category == ReferenceWindow::kCat_MemLabel) {
        int idx, ary_idx, elem_idx;
        GetExpandedRowIndex(m_sel, &idx, &ary_idx, &elem_idx);
        if (idx >= 0 && Debugger::TYPE_DEF[dynamic_cast<MemLabel*>(m_filtered_ary[idx])->element(0)->type].pointer) {
          HMENU menu = CreatePopupMenu();
          AppendMenu(menu, MF_STRING | MF_ENABLED, kPopupMenu_ShowPointer, (LPCTSTR)"ポインタの参照先を表示");
          TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON, gpt.x, gpt.y, 0, m_dlg->getHwnd(), NULL);
        }
      } else if (m_category == ReferenceWindow::kCat_String) {
        HMENU menu = CreatePopupMenu();
        AppendMenu(menu, MF_STRING | MF_ENABLED, kPopupMenu_SearchReference, (LPCTSTR)"参照元を検索");
        TrackPopupMenu(menu, TPM_LEFTALIGN | TPM_LEFTBUTTON, gpt.x, gpt.y, 0, m_dlg->getHwnd(), NULL);
      }
    }
  }
}

void ReferenceWindow::OnKeyDown(u32 p_vk, bool p_ctrl, bool p_shift)
{
  switch (p_vk) {
  case VK_DELETE:
    if (m_category == kCat_MemLabel) {
      if (m_sel < GetAllLine()) {
        int idx, ary_idx, elem_idx;
        GetExpandedRowIndex(m_sel, &idx, &ary_idx, &elem_idx);

        if (idx >= 0 && idx < static_cast<int>(m_filtered_ary.size())) {
          MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);

          // 実体ごと削除
          for (int i = 0; i < g_dbg->getMemLabelCount(); i++) {
            if (g_dbg->getMemLabel(i) == m_filtered_ary[idx]) {
              g_dbg->deleteMemLabel(i);

              SelectExpandedRow(idx, -1);
              Draw();
              g_dbg->memory_window()->Draw();
              break;
            }
          }
        }
      }
    } else if (m_category == kCat_String) {
      // 無効
    } else if (m_category == kCat_FindResult) {
      // 無効
    } else if (m_category == kCat_BottleNeck) {
      // 無効
    } else {
      if (m_sel < GetAllLine())
      {
        // 実体ごと削除
        switch (m_category) {
        case kCat_Comment:
          for (int i = 0; i < g_dbg->get_comment_count(); i++) {
            if (g_dbg->get_comment(i) == m_filtered_ary[m_sel]) {
              g_dbg->delete_comment(i);
              break;
            }
          }
          break;
        case kCat_Label:
          for (int i = 0; i < g_dbg->get_codelabel_count(); i++) {
            if (g_dbg->get_codelabel(i) == m_filtered_ary[m_sel]) {
              g_dbg->delete_codelabel(i);
              break;
            }
          }
          break;
        }

        if (m_sel >= GetAllLine()) m_sel = GetAllLine() - 1;
        if (m_sel < 0 ) m_sel = 0;
        g_dbg->drawAllWindow();
      }
    }
    break;
  case VK_RETURN:
    if (m_category == kCat_MemLabel) {
      // メモリビュー内を検索
      int idx, ary_idx, elem_idx;
      int row_type = GetExpandedRowIndex(m_sel, &idx, &ary_idx, &elem_idx);
      
      if (idx >= 0 && idx < static_cast<int>(m_filtered_ary.size())) {
        MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);

        if (row_type == kRowType_Root) {
          g_dbg->memory_window()->SelectAddress(memlabel->va());
        } else if (row_type == kRowType_Array) {
          int offset = memlabel->GetStructureSize() * ary_idx;
          g_dbg->memory_window()->SelectAddress(memlabel->va() + offset);
        } else if (row_type == kRowType_Element) {
          int offset = memlabel->GetStructureSize() * ary_idx;
          for (int i = 0; i < elem_idx; i++) {
            offset += memlabel->element(i)->size;
          }
          g_dbg->memory_window()->SelectAddress(memlabel->va() + offset);
        }
      }
    } else if (m_category == kCat_String) {
      if (m_sel >= 0 && m_sel < static_cast<int>(m_filtered_ary.size())) {
        // メモリビュー内を検索
        g_dbg->memory_window()->SelectAddress(m_filtered_ary[m_sel]->va());
      }
    } else if (m_category == kCat_FindResult) {
      if (m_sel >= 0 && m_sel < static_cast<int>(m_filtered_ary.size())) {
        FindItem* find_item = dynamic_cast<FindItem*>(m_filtered_ary[m_sel]);
        if (find_item->location() == FindItem::kLocation_Code) {
          // コードビュー内を検索
          g_dbg->code_window()->AutoJumpModule(find_item->va());
          g_dbg->code_window()->SelectAddress(find_item->va());
        } else {
          // メモリビュー内を検索
          g_dbg->memory_window()->SelectAddress(find_item->va());
        }
      }
    } else if (m_category == kCat_BottleNeck) {
      if (m_sel >= 0 && m_sel < static_cast<int>(m_filtered_ary.size())) {
        BottleNeck* bottle_neck = dynamic_cast<BottleNeck*>(m_filtered_ary[m_sel]);
        g_dbg->code_window()->AutoJumpModule(bottle_neck->va());
        g_dbg->code_window()->SelectAddress(bottle_neck->va());
      }
    } else {
      if (m_sel >= 0 && m_sel < static_cast<int>(m_filtered_ary.size())) {
        // コードビュー内を検索
        g_dbg->code_window()->AutoJumpModule(m_filtered_ary[m_sel]->va());
        g_dbg->code_window()->SelectAddress(m_filtered_ary[m_sel]->va());
      }
    }
    break;
  case VK_RIGHT:
    if (m_category == kCat_MemLabel) {
      int idx, ary_idx, elem_idx;
      int row_type = GetExpandedRowIndex(m_sel, &idx, &ary_idx, &elem_idx);

      if (idx >= 0 && idx < static_cast<int>(m_filtered_ary.size())) {
        MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);

        if (idx != -1) {
          if (row_type == kRowType_Root) {
            if (memlabel->count() > 1 ||
                memlabel->element_size() > 1) {
              memlabel->set_expand(true);
            }
          } else if (row_type == kRowType_Array) {
            if (memlabel->element_size() > 1) {
              memlabel->set_expand_ary(ary_idx, true);
            }
          }
          Draw();
        }
      }
    }
    break;
  case VK_LEFT:
    if (m_category == kCat_MemLabel) {
      int idx, ary_idx, elem_idx;
      int row_type = GetExpandedRowIndex(m_sel, &idx, &ary_idx, &elem_idx);

      if (idx >= 0 && idx < static_cast<int>(m_filtered_ary.size())) {
        MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);

        if (row_type == kRowType_Root) {
          memlabel->set_expand(false);
        } else {
          if (row_type == kRowType_Array) {
            if (memlabel->expand_ary(ary_idx)) {
              memlabel->set_expand_ary(ary_idx, false);
            } else {
              SelectExpandedRow(idx, -1);
            }
          } else if (row_type == kRowType_Element) {
            if (memlabel->count() > 1) {
              SelectExpandedRow(idx, ary_idx);
            } else {
              SelectExpandedRow(idx, -1);
            }
          }
        }
        Draw();
      }
    }
    break;
  case VK_UP:    CursorUp(1); break;
  case VK_PRIOR: CursorUp(GetPageLine()); break;    // PageUp
  case VK_DOWN:  CursorDown(1); break;
  case VK_NEXT:  CursorDown(GetPageLine()); break;  // PageDown
  }
}

void ReferenceWindow::OnClear() {
  m_sorted_ary.clear();
  m_filtered_ary.clear();
}

void ReferenceWindow::add_entry(Comment* p_data) {
  if (m_category == kCat_Comment) {
    m_sorted_ary.push_back(p_data);
    update_filter();
    Draw();
  }
}

void ReferenceWindow::add_entry(Label* p_data) { 
  if (m_category == kCat_Label) {
    m_sorted_ary.push_back(p_data);
    update_filter();
    Draw();
  }
}

void ReferenceWindow::add_entry(MemLabel* p_data) { 
  if (m_category == kCat_MemLabel) {
    m_sorted_ary.push_back(p_data);
    update_filter();
    Draw();
  }
}

void ReferenceWindow::add_entry(BottleNeck* p_data) { 
  if (m_category == kCat_BottleNeck) {
    m_sorted_ary.push_back(p_data);
    update_filter();
    Draw();
  }
}

void ReferenceWindow::remove_entry(Comment* p_data) {
  if (m_category == kCat_Comment) {
    for (u32 i = 0; i < m_sorted_ary.size(); i++) {
      if (m_sorted_ary[i] == p_data) {
        m_sorted_ary.erase(&m_sorted_ary[i]);
        update_filter();
        Draw();
        break;
      }
    }
  }
}

void ReferenceWindow::remove_entry(Label* p_data) {
  if (m_category == kCat_Label) {
    for (u32 i = 0; i < m_sorted_ary.size(); i++) {
      if (m_sorted_ary[i] == p_data) {
        m_sorted_ary.erase(&m_sorted_ary[i]);
        update_filter();
        Draw();
        break;
      }
    }
  }
}

void ReferenceWindow::remove_entry(MemLabel* p_data) {
  if (m_category == kCat_MemLabel) {
    for (u32 i = 0; i < m_sorted_ary.size(); i++) {
      if (m_sorted_ary[i] == p_data) {
        m_sorted_ary.erase(&m_sorted_ary[i]);
        update_filter();
        Draw();
        break;
      }
    }
  }
}

void ReferenceWindow::update_list() {
  
  m_sorted_ary.clear();

  if (g_dbg == NULL) return;

  switch (m_category) {
  case kCat_Comment:
    {
      int count = g_dbg->get_comment_count();
      for (int i = 0; i < count; i++) {
        if (!m_show_cur_only || g_dbg->get_comment(i)->is_cur()) {
          m_sorted_ary.push_back(g_dbg->get_comment(i));
        }
      }
    }
    break;
  case kCat_Label:
    {
      int count = g_dbg->get_codelabel_count();
      for (int i = 0; i < count; i++) {
        if (!m_show_cur_only || g_dbg->get_codelabel(i)->is_cur()) {
          m_sorted_ary.push_back(g_dbg->get_codelabel(i));
        }
      }
    }
    break;
  case kCat_MemLabel:
    {
      int count = g_dbg->getMemLabelCount();
      for (int i = 0; i < count; i++) {
        if (!m_show_cur_only || g_dbg->getMemLabel(i)->is_cur()) {
          m_sorted_ary.push_back(g_dbg->getMemLabel(i));
        }
      }
    }
    break;
  case kCat_String:
    for (StringMapItr itr = g_dbg->string_map_.begin();
         itr != g_dbg->string_map_.end();
         ++itr) {
      m_sorted_ary.push_back(itr->second);
    }
    break;
  case kCat_FindResult:
    for (u32 i = 0; i < g_dbg->finditem_ary_.size(); i++) {
      m_sorted_ary.push_back(g_dbg->finditem_ary_[i]);
    }
    break;
  case kCat_BottleNeck:
    for (u32 i = 0; i < g_dbg->bottle_neck_ary_.size(); i++) {
      m_sorted_ary.push_back(g_dbg->bottle_neck_ary_[i]);
    }
    break;
  }
  update_filter();
}

void ReferenceWindow::update_filter(const char* p_filter_str) {
  
  m_filtered_ary.clear();

  char  str[1024];
  if (p_filter_str) {
    strcpy(str, p_filter_str);
  } else {
    GetDlgItemText(m_dlg->getHwnd(), IDC_EDT_FILTER, str, 256);
  }

  if (g_dbg == NULL) return;

  for (u32 i = 0; i < m_sorted_ary.size(); i++) {
    if (ds_util::strmatch(m_sorted_ary[i]->text(), str)) {
      m_filtered_ary.push_back(m_sorted_ary[i]);
    }
  }
}

void ReferenceWindow::SearchReference(u32 p_va) {
  ModuleInfo* module = g_dbg->findELFModule();
  assert(module);
  
  g_dbg->clearFindItem();

  bool  over = false;
  // コードを検索
  for (u32 i = 0; i < module->size(); i += 4) {
    if (g_dbg->finditem_ary_.size() > 5000) {
      over = true;
      break;
    }

    u32 va = module->addr() + i;
    AnalyzeInfoMapItr itr = g_dbg->analyze_map_.find(va);
    if (itr != g_dbg->analyze_map_.end() &&
        itr->second.valid_estimate() &&
        itr->second.estimate_value() == p_va) {
      FindItem* finditem = new FindItem(va, FindItem::kLocation_Code);
      
      s8 str[1024];
      disasm_plain(va, *((u32*)VA2PA(va)), str);
      finditem->set_text(str);

      g_dbg->finditem_ary_.push_back(FindItemAryValue(finditem));
    }
  }
  // メモリを検索
  for (u32 i = 0; i < Memory::kMainMemorySize >> 2; i++)
  {
    if (g_dbg->finditem_ary_.size() > 5000) { over = true; break; }

    u32 va = i * 4;
    if (*((u32*)VA2PA(va)) == p_va) {
      FindItem* finditem = new FindItem(va, FindItem::kLocation_Memory);

      s8 str[1024];
      sprintf(str, "%08x", va);
      finditem->set_text(str);

      g_dbg->finditem_ary_.push_back(FindItemAryValue(finditem));
    }
  }
  if (over) {
    MessageBox(m_dlg->getHwnd(), "一定数を超えたため中断します", NULL, NULL);
  }
  
  if (g_dbg->finditem_ary_.size() == 0) {
    MessageBox(m_dlg->getHwnd(), "参照元はありません", NULL, NULL);
  } else {
    // 自動で検索結果を表示
    SendMessage(GetDlgItem(m_dlg->getHwnd(), IDC_CMB_CATEGORY), CB_SETCURSEL, kCat_FindResult, 0L);
    SendMessage(m_dlg->getHwnd(), WM_COMMAND, (CBN_SELCHANGE << 16) | IDC_CMB_CATEGORY, 0L);
  }

  update_list();
  Draw();
}

void ReferenceWindow::UpdateBottleNeckEntries() {  
  g_dbg->clearBottleNeck();

  for (u32 i = 0; i < g_cpu->m_recompile_hint_ary.size(); i++) {
    if (g_cpu->m_recompile_hint_ary[i] > 0) {
      u32 va = g_code_addr + (i << 2);
      
      BottleNeck* bottle_neck = new BottleNeck(va, &g_cpu->m_recompile_hint_ary[i]);
      s8 str[1024];
      disasm(va, *((u32*)VA2PA(va)), str);
      bottle_neck->set_text(str);

      g_dbg->bottle_neck_ary_.push_back(bottle_neck);
    }
  }

  update_list();
  Draw();
}

int ReferenceWindow::GetExpandedRowCount() {
  // ツリーボタン展開を考慮した場合の行数を求める
  if (m_category != kCat_MemLabel) return 0;

  int count = 0;
  for (u32 i = 0; i < m_filtered_ary.size(); i++) {
    count++;

    MemLabel* memlebal = dynamic_cast<MemLabel*>(m_filtered_ary[i]);

    if (memlebal->expand()) {
      if (memlebal->count() > 1) {
        for (int j = 0; j < memlebal->count(); j++) {
          count++;

          if (memlebal->expand_ary(j)) {
            count += memlebal->element_size();
          }
        }
      } else {
        // 配列ではない場合、トップのツリー展開で構造体を展開する
        count += memlebal->element_size();
      }
    }
  }
  return count;
}

int ReferenceWindow::GetExpandedRowIndex(int p_row, int* p_idx, int* p_ary_idx, int* p_elem_idx) {
  int row_type = kRowType_None;
  *p_idx    = -1;
  *p_ary_idx  = -1;
  *p_elem_idx  = -1;

  // ツリーボタン展開を考慮した場合の行に該当するインデックスを返す
  if (m_category != kCat_MemLabel) return row_type;

  int count = 0;
  for (u32 i = 0; i < m_filtered_ary.size(); i++) {
    MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[i]);

    if (p_row == count++) {
      *p_idx    = i;
      *p_ary_idx  = 0;
      *p_elem_idx  = 0;
      row_type = kRowType_Root;
    }

    if (memlabel->expand()) {
      if (memlabel->count() > 1) {
        for (int j = 0; j < memlabel->count(); j++) {
          if (p_row == count++) {
            *p_idx    = i;
            *p_ary_idx  = j;
            *p_elem_idx  = 0;
            row_type = kRowType_Array;
          }

          if (memlabel->expand_ary(j)) {
            for (int k = 0; k < memlabel->element_size(); k++) {
              if (p_row == count++) {
                *p_idx    = i;
                *p_ary_idx  = j;
                *p_elem_idx  = k;
                row_type = kRowType_Element;
              }
            }
          }
        }
      } else {
        // 配列ではない場合、トップのツリー展開で構造体を展開する
        for (int k = 0; k < memlabel->element_size(); k++) {
          if (p_row == count++) {
            *p_idx    = i;
            *p_ary_idx  = 0;
            *p_elem_idx  = k;
            row_type = kRowType_Element;
          }
        }
      }
    }
  }
  return row_type;
}

void ReferenceWindow::SelectExpandedRow(int p_idx, int p_ary_idx)
{
  // p_idxで指定されたメモリラベルのある行を選択する

  if (m_category != kCat_MemLabel) return;

  int row = 0;
  for (int i = 0; i <= p_idx; i++)
  {
    if (i == p_idx && p_ary_idx == -1) goto _end;

    MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[i]);

    if (memlabel->expand()) {
      if (memlabel->count() > 1) {
        for (int j = 0; j < memlabel->count(); j++) {
          row++;
          if (i == p_idx && j == p_ary_idx) goto _end;

          if (memlabel->expand_ary(j)) {
            row += memlabel->element_size();
          }
        }
      } else {
        // 配列ではない場合、トップのツリー展開で構造体を展開する
        row += memlabel->element_size();
      }
    }
    row++;
  }
_end:
  m_sel = row;
  if (m_sel >= GetAllLine()) m_sel = GetAllLine() - 1;
  if (m_sel < 0 ) m_sel = 0;
}

void ReferenceWindow::DrawTreeBtn(HDC p_hdc, int p_left, int p_top, bool p_expand) {
  HBRUSH hbBlack = (HBRUSH)GetStockObject(BLACK_BRUSH);
  HBRUSH hbFixedBG = (HBRUSH)CreateSolidBrush(GetSysColor(COLOR_3DFACE));

  RECT rect;
  rect.left = p_left + kTreeBtnLeft;
  rect.top = p_top + kTreeBtnTop;
  rect.right = rect.left + kTreeBtnWidth;
  rect.bottom = rect.top + kTreeBtnHeight;
  FillRect(p_hdc, &rect, hbFixedBG);
  FrameRect(p_hdc, &rect, hbBlack);

  POINT dummy;
  if (p_expand == false) {
    MoveToEx(p_hdc, rect.left + 4, rect.top + 2, &dummy);
    LineTo(p_hdc, rect.left + 4, rect.top + 7);
  }            
  MoveToEx(p_hdc, rect.left + 2, rect.top + 4, &dummy);
  LineTo(p_hdc, rect.left + 7, rect.top + 4);

  DeleteObject(hbFixedBG);
  DeleteObject(hbBlack);
}

bool ReferenceWindow::IsPointTreeBtn(int p_left, int p_top, int p_x, int p_y) {
  RECT rect;
  rect.left = p_left + kTreeBtnLeft;
  rect.top = p_top + kTreeBtnTop;
  rect.right = rect.left + kTreeBtnWidth;
  rect.bottom = rect.top + kTreeBtnHeight;
  POINT pt;
  pt.x = p_x;
  pt.y = p_y;
  return (PtInRect(&rect, pt) == TRUE);
}

void ReferenceWindow::ShowPointer() {
  int idx, ary_idx, elem_idx;
  GetExpandedRowIndex(m_sel, &idx, &ary_idx, &elem_idx);

  MemLabel* memlabel = dynamic_cast<MemLabel*>(m_filtered_ary[idx]);

  if (idx >= 0 && Debugger::TYPE_DEF[memlabel->element(0)->type].pointer) {
    u32 va = *((u32*)VA2PA(memlabel->va() + memlabel->GetStructureSize() * ary_idx + memlabel->GetElementOffset(elem_idx)));
    if (va >= g_dbg->code_window()->GetCurModule()->addr() &&
        va <  g_dbg->code_window()->GetCurModule()->addr() + g_dbg->code_window()->GetCurModule()->size()) {
      g_dbg->code_window()->AutoJumpModule(va);
      g_dbg->code_window()->SelectAddress(va);
    } else {
      g_dbg->memory_window()->SelectAddress(va);
    }
  }
}
