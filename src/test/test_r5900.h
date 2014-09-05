#if defined _UNIT_TEST

#ifndef _test_test_r5900
#define  _test_test_r5900

/*---------*/
/* include */
/*---------*/
#include "main.h"
#include "elf.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "thread/thread.h"

/*---------*/
/* defines */
/*---------*/
#define INIT_GPR(_value)           \
  for (int i = 1; i < 32; i++) {   \
    g_cpu->m_gpr[i].d[0] = _value; \
    g_cpu->m_gpr[i].d[1] = _value; \
    g_cpu->m_gpr[i].d[2] = _value; \
    g_cpu->m_gpr[i].d[3] = _value; \
  }

#define INIT_FPR(_value)                    \
  for (int i = 1; i < 32; i++) {            \
    *((u32*)&g_cpu->m_fpu.fpr[i]) = _value; \
    g_cpu->m_fpu.fcr31 = 0xffffffff;        \
  }

#define OPEN_ELF(_category) {                                \
  g_cpu->m_pc =                                              \
    Elf::load("src/test/"##_category##"/"__FUNCTION__".elf", \
              &g_code_addr, &g_code_size,                    \
              (u8*)VA2PA(Memory::kMainMemoryBase),           \
              Memory::kMainMemorySize);                      \
  g_cpu->create_recompile_cache();                           \
  thread_mgr_ = new PS2ThreadMgr();                          \
  thread_mgr_->init();                                       \
}

#define IS_OUT_OF_TEST_ELF_CODE_SECTION(_addr) ((_addr) <  g_code_addr || (_addr) >= g_code_addr + g_code_size)

extern PS2ThreadMgr* thread_mgr_;

#endif // _test_test_r5900

#endif // #if defined _UNIT_TEST
