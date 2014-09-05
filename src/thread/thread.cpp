#include "main.h"
#include "memory/memory.h"
#include "cpu/r5900.h"
#include "thread/thread.h"

#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

PS2ThreadMgr::PS2ThreadMgr() :
    cur_thread_id_(-1) {
}

PS2ThreadMgr::~PS2ThreadMgr() {
  clearThread();
}

bool PS2ThreadMgr::init() {
  clearThread();
  cur_thread_id_ = 0;
  
  thread_ary_.push_back(new PS2Thread(0, 0, 0, 0, 0, 0)); // メインスレッド
  thread_ary_[cur_thread_id_]->save_context(g_cpu);
  return true;
}

u32 PS2ThreadMgr::create(u32 p_pc, u32 p_sp, u32 p_gp) {
  u32 thread_id = thread_ary_.size();
  thread_ary_.push_back(new PS2Thread(thread_id, 0, 0, p_pc, p_sp, p_gp));
  return thread_id;
}

void PS2ThreadMgr::remove(s32 p_thid) {
  if (p_thid < static_cast<int>(thread_ary_.size()) &&  thread_ary_[p_thid]) {
    delete thread_ary_[p_thid];
    thread_ary_[p_thid] = NULL;
  }
}

void PS2ThreadMgr::clean() {
  clearThread();
}

void PS2ThreadMgr::clearThread() {
  for (vector<PS2Thread*>::iterator itr = thread_ary_.begin();
       itr != thread_ary_.end();
       ++itr) {
    delete *itr;
  }
  thread_ary_.clear();
}

void PS2ThreadMgr::updateX86Retaddrs(u8* p_old_ptr, u32 p_size, u8* p_new_ptr) {
  for (u32 i = 0; i < thread_ary_.size(); i++) {
    for (vector<u32*>::iterator itr = thread_ary_[i]->x86_retaddr_.begin();
         itr != thread_ary_[i]->x86_retaddr_.end();
         ++itr) {
      u32* addr = *itr;
      if (addr &&
          *addr >= (u32)p_old_ptr &&
          *addr <  (u32)p_old_ptr + p_size) {
        *addr += p_new_ptr - p_old_ptr;
      }
    }
  }
}

u32 PS2ThreadMgr::switchTo(s32 p_tid) {
  u32 new_tid;
  if (p_tid != -1) {
    // 指定されたスレッドへ
    new_tid = p_tid;
  } else {
    // 次のスレッドへ
    new_tid = (cur_thread_id_ + 1) % thread_ary_.size();
  }
  if (new_tid != cur_thread_id_) {
    //DBGOUT_CPU("Thread Switch id=%d > %d\n", g_cur_thread, new_tid);

    thread_ary_[cur_thread_id_]->save_context(g_cpu);
    cur_thread_id_ = new_tid;
    thread_ary_[cur_thread_id_]->load_context(g_cpu);

    u8* code = g_cpu->m_recompiled_code_table[VA2IDX(g_cpu->m_pc)];
    thread_ary_[cur_thread_id_]->set_recompiled_code(code);

#if defined _DEBUGGER
    if (g_dbg && g_dbg->step_breakpoint()->is_break()) {
      // ステップインかステップオーバーなら
      // 飛び先の先頭にステップブレークを張り替える。
      g_dbg->step_breakpoint()->breakOff();
      g_dbg->step_breakpoint()->set_va(g_cpu->m_pc);
      g_dbg->step_breakpoint()->breakOn();
    }
#endif
    SwitchToFiber(thread_ary_[cur_thread_id_]->fiber());
  }
  return new_tid;
}

PS2Thread::PS2Thread(u32 p_id, s32 p_prio,
    u32 p_status, u32 p_pc, u32 p_sp, u32 p_gp) {
  id_ = p_id;
  prio_ = p_prio;
  status_ = p_status;
  context_.init();
  
  recompiled_code_ = NULL;
  
  context_.m_pc      = p_pc;
  context_.m_sp.d[0] = p_sp;
  context_.m_gp.d[0] = p_gp;

  fiber_ = CreateFiber(0, ee_execute_proc, &recompiled_code_);
}

PS2Thread::~PS2Thread() {
  id_ = 0xffffffff;
}

void CALLBACK PS2Thread::ee_execute_proc(void* p_parameter) {
  while (1) {
    // コードがコンパイルされていないか、ブレークポイントにヒットした場合に
    // ret命令でここへ戻ってくるので実行ファイバへ遷移する
    SwitchToFiber(g_app.execute_fiber());

    _asm {
      pushad;
      mov eax, p_parameter;
      call dword ptr [eax];
      popad;
    }
  }
}
