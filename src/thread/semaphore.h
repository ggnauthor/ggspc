#ifndef _thread_semaphore
#define _thread_semaphore

#include "main.h"
#include "stlp_wrapper.h"

struct PS2SemaParam {
  s32 cur_count;
  s32 max_count;
  s32 init_count;
  s32 wait_num;
  u32 attr;
  u32 option;
};

class PS2Semaphore {
public:
  PS2Semaphore(u32 p_id, s32 p_init, s32 p_max) {
    id_ = p_id;
    count_ = p_init;
    max_ = p_max;
  }
  
  ~PS2Semaphore() {
    id_ = 0xffffffff;
  }

  bool isSignal() { return count_ > 0; }

  void inc() {
    if (count_ < max_) {
      count_++;
    }
  }

  void dec() {
    if (count_ > 0) {
      count_--;
    }
  }
  
  int count() { return count_; }
  int maxx() { return max_; }

private:
  u32 id_;
  s32 count_;
  s32 max_;
};

class PS2SemaphoreMgr {
public:
  void init();
  void clean();
  int  create(u32 p_init_count, u32 p_max_count);
  void remove(u32 p_sema_id);
  bool signal(u32 p_sema_id);
  int  wait(u32 p_sema_id);

private:
  vector<PS2Semaphore*> sema_ary_;
};

#endif // _thread_semaphore
