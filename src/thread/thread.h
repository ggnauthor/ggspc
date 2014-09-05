#ifndef _thread_thread
#define _thread_thread

#include "main.h"
#include "stlp_wrapper.h"
#include "thread/thread.h"

struct PS2ThreadParam {
  u32 status;
  u8* entry_addr;
  u8* stack_addr;
  u32 stack_size;
  u8* global_ptr_reg;
  s32 init_prio;
  s32 cur_prio;
  u32 attr;
  u32 option;
  s32 wait_type;
  s32 wait_id;
  s32 wakeup_count;
};

class PS2Thread {
  friend class PS2ThreadMgr;
public:
  PS2Thread(u32 p_id, s32 p_prio, u32 p_status, u32 p_pc, u32 p_sp, u32 p_gp);
  ~PS2Thread();

  void save_context(const R5900REGS* p_context) {
    memcpy(&context_, p_context, sizeof(context_));
  }

  void load_context(R5900REGS* p_context) {
    memcpy(p_context, &context_, sizeof(context_));
  }

  void* fiber() { return fiber_; }

  void set_recompiled_code(u8* p_ptr) { recompiled_code_ = p_ptr; }
  u32* x86_retaddr() {
    if (x86_retaddr_.size() <= 0) {
      return NULL;
    } else {
      return x86_retaddr_[x86_retaddr_.size() - 1];
    }
  }
  void push_x86_retaddr(u32* p_retaddr) { x86_retaddr_.push_back(p_retaddr); }
  void pop_x86_retaddr() { x86_retaddr_.pop_back(); }

private:
  static void CALLBACK ee_execute_proc(void* p_parameter);

  u32          id_;
  s32          prio_;
  u32          status_;
  R5900REGS    context_;
  void*        fiber_;
  u8*          recompiled_code_;
  vector<u32*> x86_retaddr_;
};

class PS2ThreadMgr {
public:
  PS2ThreadMgr();
  ~PS2ThreadMgr();

  bool init();
  void clean();
  void clearThread();
  void updateX86Retaddrs(u8* p_old_ptr, u32 p_size, u8* p_new_ptr);
  u32  switchTo(s32 p_tid = -1);

  u32  create(u32 p_pc, u32 p_sp, u32 p_gp);
  void remove(s32 p_thid);

  PS2Thread* cur_thread() { return thread_ary_[cur_thread_id_]; }

  u32 cur_thread_id() { return cur_thread_id_; }
  u32 thread_count() { return thread_ary_.size(); }

private:
  u32                cur_thread_id_;
  vector<PS2Thread*> thread_ary_;
};

#endif // _thread_thread
