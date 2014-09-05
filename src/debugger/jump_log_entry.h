#ifndef _jump_log_entry
#define _jump_log_entry

#include "main.h"
#include "stlp_wrapper.h"

struct JumpLogRepeat {
  int  count;  // ŒJ‚è•Ô‚µ”
  int  len;  // ŒJ‚è•Ô‚µ’·
};

class JumpLogEntry {
// constructor/destructor
public:
  enum {
    kGroupMaxLen = 64  // ƒOƒ‹[ƒv‰»‰Â”\‚ÈŒJ‚è•Ô‚µ’·
  };

  JumpLogEntry(u32 p_va) {
    m_va = p_va;
  }
  ~JumpLogEntry() {
    for (vector<JumpLogRepeat*>::iterator itr = m_repeat.begin();
        itr != m_repeat.end();
        ++itr) {
      delete *itr;
    }
    m_repeat.clear();
  }

// accessor
public:
  inline u32 va() { return m_va; }

// member variable
private:
public:
  u32              m_va;
  vector<JumpLogRepeat*>  m_repeat;
};

#endif // _jump_log_entry
