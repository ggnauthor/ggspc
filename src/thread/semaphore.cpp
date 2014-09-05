#include "main.h"
#include "memory/memory.h"
#include "cpu/r5900.h"
#include "thread/semaphore.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

void PS2SemaphoreMgr::init() {
}

void PS2SemaphoreMgr::clean() {
  for (vector<PS2Semaphore*>::iterator itr = sema_ary_.begin();
       itr != sema_ary_.end();
       ++itr) {
    delete *itr;
  }
  sema_ary_.clear();
}

int PS2SemaphoreMgr::create(u32 p_init_count, u32 p_max_count) {
  int sema_id = sema_ary_.size();
  sema_ary_.push_back(new PS2Semaphore(sema_id, p_init_count, p_max_count));
  return sema_id;
}

void PS2SemaphoreMgr::remove(u32 p_sema_id) {
  if (p_sema_id < (u32)sema_ary_.size() &&  sema_ary_[p_sema_id]) {
    delete sema_ary_[p_sema_id];
    sema_ary_[p_sema_id] = NULL;
  }
}

bool PS2SemaphoreMgr::signal(u32 p_sema_id) {
  if (p_sema_id < (u32)sema_ary_.size() &&  sema_ary_[p_sema_id]) {
    sema_ary_[p_sema_id]->inc();
    return true;
  }
  return false;
}

int PS2SemaphoreMgr::wait(u32 p_sema_id) {
  if (p_sema_id < (u32)sema_ary_.size() && sema_ary_[p_sema_id]) {
    if (sema_ary_[p_sema_id]->isSignal()) {
      sema_ary_[p_sema_id]->dec();
      return 1;
    } else {
      return 2;
    }
  }
  return 0;
}
