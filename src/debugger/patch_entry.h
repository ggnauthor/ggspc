#ifndef _debugger_patch_entry
#define _debugger_patch_entry

#include "main.h"

class PatchEntry {
public:
  PatchEntry(u32 p_va, u32 p_size, bool p_state, const s8* p_new_data);
  ~PatchEntry();

  void update(int p_offset, int p_size, const s8* p_new_data);
  void SetEnable();
  void SetDisable();

  const s8* name() const { return m_name; }
  u32 va() const { return m_va; }
  int size() const { return m_size; }
  bool state() const { return m_state; }
  const s8* old_data() const { return m_old_data; }
  const s8* new_data() const { return m_new_data; }

  void set_name(const s8* p_name) { strcpy(m_name, p_name); }
  // パッチの多重適用に注意！
  // パッチの解除と再設定は呼び出し側に任せる
  void set_addr(u32 p_va) { m_va = p_va; }
  // パッチの多重適用に注意！
  // パッチの解除と再設定は呼び出し側に任せる
  void set_size(int p_size) { m_size = p_size; }

private:
  s8   m_name[256];
  u32  m_va;
  int  m_size;
  bool m_state;
  s8*  m_old_data;
  s8*  m_new_data;
};

#endif // _debugger_patch_entry
