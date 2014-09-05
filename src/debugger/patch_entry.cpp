#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/patch_entry.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

//-----------------------------------------------------------------------------
// global

//-----------------------------------------------------------------------------
// function
PatchEntry::PatchEntry(u32 p_va,
                       u32 p_size,
                       bool p_state,
                       const s8* p_new_data) {
  // m_old_dataにオリジナルのデータを格納するために
  // 事前にすべてのパッチを無効にしておくこと
  m_va = p_va;
  m_size = p_size;
  m_state = false;
  m_old_data = (s8*)malloc(m_size);
  m_new_data = (s8*)malloc(m_size);
  memcpy(m_old_data, reinterpret_cast<const s8*>(VA2PA(m_va)), m_size);
  memcpy(m_new_data, p_new_data, m_size);
  m_name[0] = '\0';

  if (p_state) SetEnable();
}

PatchEntry::~PatchEntry() {
  SetDisable();
  free(m_old_data);
  free(m_new_data);
}

void PatchEntry::update(int p_offset, int p_size, const s8* p_new_data) {
  // パッチの多重適用に注意！
  // パッチの解除と再設定は呼び出し側に任せる
  //assert(p_offset >= 0);
  //assert(p_offset < m_size);

  if (m_size < p_offset + p_size) {
    // サイズの拡張
    m_size = p_offset + p_size;
    m_old_data = (s8*)realloc(m_old_data, m_size);
    m_new_data = (s8*)realloc(m_new_data, m_size);
  }
  memcpy(m_old_data + p_offset, reinterpret_cast<const s8*>(VA2PA(m_va)), p_size);
  memcpy(m_new_data + p_offset, p_new_data, p_size);
}

void PatchEntry::SetEnable() {
  if (!m_state) {
    while (1) {
      PatchEntry* patch = g_dbg->findPatch(m_va, m_size, true);
      if (patch == NULL) break;

      char str[1024];
      sprintf(str, "%08x[%d]には既にパッチが適用されています。\n%sを解除して%sを適用します。", m_va, m_size, patch->name(), m_name);
      MessageBox(NULL, str, NULL, NULL);
      patch->SetDisable();
    }

    memcpy(reinterpret_cast<void*>(VA2PA(m_va)), m_new_data, m_size);
    m_state = true;

    g_cpu->clear_recompile_cache();
  }
}

void PatchEntry::SetDisable() {
  if (m_state) {
    memcpy(reinterpret_cast<void*>(VA2PA(m_va)), m_old_data, m_size);
    m_state = false;

    g_cpu->clear_recompile_cache();
  }
}
