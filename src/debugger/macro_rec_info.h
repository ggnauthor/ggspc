#ifndef _debugger_macro_rec_info
#define _debugger_macro_rec_info

#include "main.h"

class MacroRecInfo {
public:
  MacroRecInfo(u32 p_va, u32 p_pat_id) {
    va_ = p_va;
    pattern_id_ = p_pat_id;
  }

  ~MacroRecInfo() {}

  u32 va() { return va_; }
  u32 pattern_id() { return pattern_id_; }

private:
  u32 va_;
  u32 pattern_id_;
};

#endif // _debugger_macro_rec_info
