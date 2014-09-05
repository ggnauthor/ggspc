#if defined _UNIT_TEST

/*---------*/
/* include */
/*---------*/
#include "test/test_r5900.h"

#include "debug_malloc.h"

#define QP_BEGIN()              \
  if (g_qp_support) {            \
    QueryPerformanceCounter(&g_qp_work);\
  }

#define QP_END()              \
  if (g_qp_support) {            \
    LARGE_INTEGER end_time;        \
    QueryPerformanceCounter(&end_time);  \
    /* _resultにus単位で返す */      \
    g_qp_result = (int)((end_time.QuadPart - g_qp_work.QuadPart) * 1000000 / g_qp_freq.QuadPart);\
  }

/*--------*/
/* global */
/*--------*/
LARGE_INTEGER g_qp_freq;
LARGE_INTEGER g_qp_work;
BOOL g_qp_support = false;
int g_qp_result = -1;

/*----------*/
/* function */
/*----------*/
FIXTURE(r5900_macro_fix_01);

SETUP(r5900_macro_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();

  g_qp_support = QueryPerformanceFrequency(&g_qp_freq);
}

TEARDOWN(r5900_macro_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// macro01
BEGIN_TESTF(r5900_macro01_001, r5900_macro_fix_01) {
  try {
    OPEN_ELF("macro");
    u32 entry_point = g_cpu->m_pc;
    INIT_GPR(0xcccccccc);

    int time[2];
    for (int i = 0; i < 2; i++) {
      g_cpu->clear_recompile_cache();
      // あらかじめリコンパイルしておく
      g_cpu->m_pc = entry_point;
      g_cpu->execute(i == 1);

      g_cpu->m_pc = entry_point;
      g_cpu->m_fpu.fpr[0] = 0.0f;
      g_cpu->m_fpu.fpr[1] = 0.0f;
      g_cpu->m_fpu.fpr[2] = 0.0f;

      QP_BEGIN();
      g_cpu->execute();
      QP_END();
      time[i] = g_qp_result;
    }

    //WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x12345678);
    WIN_TRACE("%d > %d(us) %3.1f%%\n", time[0], time[1], (float)time[1] * 100.0f / time[0]);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// macro02
BEGIN_TESTF(r5900_macro02_001, r5900_macro_fix_01) {
  try {
    OPEN_ELF("macro");
    u32 entry_point = g_cpu->m_pc;
    INIT_GPR(0xcccccccc);

    int time[2];
    for (int i = 0; i < 2; i++) {
      g_cpu->clear_recompile_cache();
      // あらかじめリコンパイルしておく
      g_cpu->m_pc = entry_point;
      g_cpu->m_a0.d[0] = 0x1000;
      g_cpu->m_a1.d[0] = 0x1234;

      g_cpu->execute(i == 1);

      g_cpu->m_pc = entry_point;
      g_cpu->m_a0.d[0] = 0x1000;
      g_cpu->m_a1.d[0] = 0x1234;

      QP_BEGIN();
      g_cpu->execute();
      QP_END();
      time[i] = g_qp_result;

      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 -  4)) == 0x0000);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 +  0)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 +  4)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 +  8)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 + 12)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 + 16)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 + 20)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 + 24)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 + 28)) == 0x1234);
      WIN_ASSERT_TRUE(*((u32*)VA2PA(0x1000 + 32)) == 0x0000);
    }

    WIN_TRACE("%d > %d(us) %3.1f%%\n", time[0], time[1], (float)time[1] * 100.0f / time[0]);
  } catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
