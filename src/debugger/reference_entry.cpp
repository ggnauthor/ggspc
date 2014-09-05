#include "main.h"
#include "debugger/debugger.h"
#include "debugger/reference_entry.h"

#include <assert.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

ReferenceEntryBase::ReferenceEntryBase(DebugDatabase* p_owner,
                                       u32 p_va, char* p_text) {
  m_va = p_va;
  strncpy(m_text, p_text, 255);
  m_owner = p_owner;
}

bool ReferenceEntryBase::is_cur() {
  if (g_dbg->cur_ddb_idx_ < 0) return false;
  return m_owner == NULL || m_owner == g_dbg->ddb_ary_[g_dbg->cur_ddb_idx_];
}

void MemLabel::AddElement(u32 p_type, u32 p_size, s8* p_label) {
  MemLabelElement* elem = new MemLabelElement;
  elem->type = p_type;
  elem->size = p_size;
  strncpy(elem->label, p_label, 255);
  m_elem_ary.push_back(elem);
}

void MemLabel::clearElement() {
  for (vector<MemLabelElement*>::iterator itr = m_elem_ary.begin();
      itr != m_elem_ary.end();
      ++itr) {
    delete *itr;
  }
  m_elem_ary.clear();
}

int MemLabel::GetStructureSize()
{
  // 構造体のサイズを返す
  int total = 0;
  for (u32 i = 0; i < m_elem_ary.size(); i++) {
    total += m_elem_ary[i]->size;
  }
  return total;
}

bool MemLabel::GetMemoryLabelFromVA(u32 p_va, s8* p_label)
{
  // 範囲外
  if (m_va > p_va || m_va + GetSize() <= p_va) {
    return false;
  }

  // p_vaから構造体のラベルを構成する
  u32  str_size = GetStructureSize();
  u32 array_idx = (p_va - m_va) / str_size;

  u32 diff = (p_va - m_va) % str_size;

  int elem_idx = -1;
  int pos = 0;
  for (u32 i = 0; i < m_elem_ary.size(); i++) {
    if (diff < pos + m_elem_ary[i]->size) {
      elem_idx = i;
      diff -= pos;
      break;
    } else {
      pos += m_elem_ary[i]->size;
    }
  }

  sprintf(p_label, "@m ");
  char tmp[256];
  // 配列なら要素番号を付加
  if (m_count > 1) {
    sprintf(tmp, "&%s[%d]", m_text, array_idx);
  } else {
    sprintf(tmp, "%s", m_text);
  }
  strcat(p_label, tmp);

  // 複数メンバならメンバ名を付加
  if (m_elem_ary.size() > 1) {
    sprintf(tmp, ".%s", m_elem_ary[elem_idx]->label);
    strcat(p_label, tmp);
  }

  // ラベルとの差額表示
  if (diff > 0) {
    sprintf(tmp, "+%d", diff);
    strcat(p_label, tmp);
  }

  strcat(p_label, "@w");

  return true;
}

int MemLabel::GetElementOffset(int p_idx)
{
  int offset = 0;
  for (int i = 0; i < p_idx; i++) {
    offset += m_elem_ary[i]->size;
  }
  return offset;
}

void MemLabel::ConvertElementData(s8* p_buf, int p_bufsize, u8* p_data, u32 p_type, int p_size)
{
  assert(p_bufsize >= 8);

  switch (p_type)
  {
  case 0:  // hex
    p_buf[0] = '\0';
    for (int k = 0; k < p_size; k++) {
      if ((k + 1) * 2 >= p_bufsize) break;

      char tmp[4];
      sprintf(tmp, "%02x", *(p_data + k));
      strcat(p_buf, tmp);
    }
    break;
  case 1:  // int8
    sprintf(p_buf, "%d", *((s8*)p_data));
    break;
  case 2:  // uint8
    sprintf(p_buf, "%d", *((u8*)p_data));
    break;
  case 3:  // int16
    sprintf(p_buf, "%d", *((s16*)p_data));
    break;
  case 4:  // uint16
    sprintf(p_buf, "%d", *((u16*)p_data));
    break;
  case 5:  // int32
    sprintf(p_buf, "%d", *((s32*)p_data));
    break;
  case 6:  // uint32
    sprintf(p_buf, "%d", *((u32*)p_data));
    break;
  case 7:  // int64
    sprintf(p_buf, "%d", *((s64*)p_data));
    break;
  case 8:  // uint64
    sprintf(p_buf, "%d", *((u64*)p_data));
    break;
  case 9:  // float
    sprintf(p_buf, "%f", *((float*)p_data));
    break;
  case 10:// double
    sprintf(p_buf, "%f", *((double*)p_data));
    break;
  case 11:// string
    {
      int size = p_size;
      if (size > p_bufsize) size = p_bufsize;
      memcpy(p_buf, p_data, size - 1);
      p_buf[p_size - 1] = 0;
    }
    break;
  case 12:// pointer
    sprintf(p_buf, "%08x", *((u32*)p_data));
    break;
  }
}
