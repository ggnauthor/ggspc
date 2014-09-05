#ifndef _debugger_sourceinfo
#define _debugger_sourceinfo

#include "main.h"
#include "stlp_wrapper.h"

struct Symbol {
  Symbol(u32 p_va, s8* p_name) {
    strncpy(name, p_name, 256);
    va = p_va;
  }
  ~Symbol() {}

  s8  name[256];
  u32 va;
};

struct SourceLocationEntry {
  SourceLocationEntry(u32 p_addr, int p_src_id, int p_line) {
    addr = p_addr;
    src_id = p_src_id;
    src_info_id = -1;
    line = p_line;
  }
  ~SourceLocationEntry() {}

  u32 addr;
  s32 src_id;
  s32 src_info_id;
  s32 line;
};

struct SourceFileEntry {
  SourceFileEntry(char* p_path, int p_path_length) {
    strncpy(path, p_path, 1024);
    name = path + p_path_length;
  }
  ~SourceFileEntry() {}

  s8  path[1024];
  s8* name;
};

class SourceFileInfo {
// constructor/destructor
public:
  SourceFileInfo(char* p_data, int p_fsize, SourceFileEntry* p_sfe);

  ~SourceFileInfo() {
    if (m_data) delete m_data;
  }

// accessor
public:
  s8*            line_data(int p_idx) { return m_line_data_ary[p_idx]; }
  int            line_data_size() { return m_line_data_ary.size(); }
  const SourceFileEntry*  src_file() { return m_src_file; }
  int            scroll() { return m_scroll; }
  void            set_scroll(int p_val) { m_scroll = p_val; }

// member variable
private:
  s8*              m_data;
  vector<s8*>      m_line_data_ary; // 行分割したソースm_dataバッファ内をポイント
  int              m_scroll;
  SourceFileEntry* m_src_file;
};

#endif // _debugger_sourceinfo
