#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_regimm_fix_01);

SETUP(r5900_regimm_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_regimm_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// bltz
BEGIN_TESTF(r5900_regimm_bltz_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bgez
BEGIN_TESTF(r5900_regimm_bgez_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == -2);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bltzl
BEGIN_TESTF(r5900_regimm_bltzl_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bgezl
BEGIN_TESTF(r5900_regimm_bgezl_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == -1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bltzal
BEGIN_TESTF(r5900_regimm_bltzal_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == g_cpu->m_v0.d[0]);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == g_cpu->m_v1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bgezal
BEGIN_TESTF(r5900_regimm_bgezal_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0xffffffff; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_s1.d[1] = 0x00000000; g_cpu->m_s1.d[0] = 0x00000000;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == g_cpu->m_v0.d[0]);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == g_cpu->m_v1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bltzall
BEGIN_TESTF(r5900_regimm_bltzall_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == g_cpu->m_v1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bgezall
BEGIN_TESTF(r5900_regimm_bgezall_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0xffffffff; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_s1.d[1] = 0x00000000; g_cpu->m_s1.d[0] = 0x00000000;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == g_cpu->m_v1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

BEGIN_TESTF(r5900_regimm_mtsab_mtsah_001, r5900_regimm_fix_01){
  try
  {
    OPEN_ELF("regimm");
    INIT_GPR(0x00000000);
    g_cpu->m_t0.d[0] = 0xffff0010;
    g_cpu->m_t1.d[0] = 0xffffff07;
    g_cpu->m_t2.d[0] = 0x0000000e;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[0] == 0x00000008);  // 1 * 8
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[0] == 0x00000030);  // 6 * 8
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[0] == 0x00000048);  // 9 * 8
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[0] == 0x00000010);  // 1 * 16
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[0] == 0x00000060);  // 6 * 16
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[0] == 0x00000010);  // 1 * 16
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
