#ifndef _debugger_module_info
#define _debugger_module_info

#include "main.h"

class ModuleInfo {
public:
  ModuleInfo(const s8* p_name, int p_addr, int p_size, int p_entrypoint, bool p_dummy) {
    strcpy(name_, p_name);
    entrypoint_ = p_entrypoint;
    addr_       = p_addr;
    size_       = p_size;
    dummy_      = p_dummy;
  }
  ~ModuleInfo() {}

  inline char* name() { return name_; }
  inline u32   addr() { return addr_; }
  inline u32   size() { return size_; }
  inline u32   entrypoint() { return entrypoint_; }
  inline bool  dummy() { return dummy_; }

private:
  char name_[1024];
  u32  entrypoint_;
  u32  addr_;
  u32  size_;
  bool dummy_;
};

#endif // _debugger_module_info
