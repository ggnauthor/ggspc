#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_mmi2_fix_01);

SETUP(r5900_mmi2_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_mmi2_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// pmaddw ノーマル
BEGIN_TESTF(r5900_mmi2_pmaddw_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[3] = 0x00000000; g_cpu->m_a0.d[2] = 0x10000000;  g_cpu->m_a0.d[1] = 0x00000000; g_cpu->m_a0.d[0] = 0x00012345;
    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x0000000f;  g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x0006789a;
    g_cpu->m_hi.d[3] = 0x00000000; g_cpu->m_hi.d[2] = 0x00000001;  g_cpu->m_hi.d[1] = 0x00000000; g_cpu->m_hi.d[0] = 0x00001111;
    g_cpu->m_lo.d[3] = 0x00000000; g_cpu->m_lo.d[2] = 0x20000000;  g_cpu->m_lo.d[1] = 0x00000000; g_cpu->m_lo.d[0] = 0x00002222;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x00000000 && g_cpu->m_hi.d[2] == 0x00000002 && g_cpu->m_hi.d[1] == 0x00000000 && g_cpu->m_hi.d[0] == 0x00001118);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x00000000 && g_cpu->m_lo.d[2] == 0x10000000 && g_cpu->m_lo.d[1] == 0x00000000 && g_cpu->m_lo.d[0] == 0x5cd5b1a4);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x00000002 && g_cpu->m_t0.d[2] == 0x10000000 && g_cpu->m_t0.d[1] == 0x00001118 && g_cpu->m_t0.d[0] == 0x5cd5b1a4);

    g_cpu->init();
    g_cpu->m_a0.d[3] = 0x00000000; g_cpu->m_a0.d[2] = 0x00000000;  g_cpu->m_a0.d[1] = 0x00000000; g_cpu->m_a0.d[0] = 0x00000000;
    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00000001;  g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_hi.d[3] = 0x00000000; g_cpu->m_hi.d[2] = 0x80000000;  g_cpu->m_hi.d[1] = 0x00000000; g_cpu->m_hi.d[0] = 0xffffffff;
    g_cpu->m_lo.d[3] = 0x00000000; g_cpu->m_lo.d[2] = 0xffffffff;  g_cpu->m_lo.d[1] = 0x00000000; g_cpu->m_lo.d[0] = 0x80000000;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0xffffffff && g_cpu->m_hi.d[2] == 0x80000000 && g_cpu->m_hi.d[1] == 0xffffffff && g_cpu->m_hi.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0xffffffff && g_cpu->m_lo.d[2] == 0xffffffff && g_cpu->m_lo.d[1] == 0xffffffff && g_cpu->m_lo.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x80000000 && g_cpu->m_t0.d[2] == 0xffffffff && g_cpu->m_t0.d[1] == 0xffffffff && g_cpu->m_t0.d[0] == 0x80000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmsubw ノーマル
BEGIN_TESTF(r5900_mmi2_pmsubw_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[3] = 0x00000000; g_cpu->m_a0.d[2] = 0x10000000;  g_cpu->m_a0.d[1] = 0x00000000; g_cpu->m_a0.d[0] = 0x00012345;
    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x0000000f;  g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x0006789a;
    g_cpu->m_hi.d[3] = 0x00000000; g_cpu->m_hi.d[2] = 0x00000002;  g_cpu->m_hi.d[1] = 0x00000000; g_cpu->m_hi.d[0] = 0x00001111;
    g_cpu->m_lo.d[3] = 0x00000000; g_cpu->m_lo.d[2] = 0x20000000;  g_cpu->m_lo.d[1] = 0x00000000; g_cpu->m_lo.d[0] = 0x00002222;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x00000000 && g_cpu->m_hi.d[2] == 0x00000001 && g_cpu->m_hi.d[1] == 0x00000000 && g_cpu->m_hi.d[0] == 0x00001109);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x00000000 && g_cpu->m_lo.d[2] == 0x30000000 && g_cpu->m_lo.d[1] == 0xffffffff && g_cpu->m_lo.d[0] == 0xa32a92a0);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x00000001 && g_cpu->m_t0.d[2] == 0x30000000 && g_cpu->m_t0.d[1] == 0x00001109 && g_cpu->m_t0.d[0] == 0xa32a92a0);

    g_cpu->init();
    g_cpu->m_a0.d[3] = 0x00000000; g_cpu->m_a0.d[2] = 0x00000000;  g_cpu->m_a0.d[1] = 0x00000000; g_cpu->m_a0.d[0] = 0x00000000;
    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00000001;  g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_hi.d[3] = 0x00000000; g_cpu->m_hi.d[2] = 0x80000000;  g_cpu->m_hi.d[1] = 0x00000000; g_cpu->m_hi.d[0] = 0xffffffff;
    g_cpu->m_lo.d[3] = 0x00000000; g_cpu->m_lo.d[2] = 0xffffffff;  g_cpu->m_lo.d[1] = 0x00000000; g_cpu->m_lo.d[0] = 0x80000000;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0xffffffff && g_cpu->m_hi.d[2] == 0x80000000 && g_cpu->m_hi.d[1] == 0xffffffff && g_cpu->m_hi.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0xffffffff && g_cpu->m_lo.d[2] == 0xffffffff && g_cpu->m_lo.d[1] == 0xffffffff && g_cpu->m_lo.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x80000000 && g_cpu->m_t0.d[2] == 0xffffffff && g_cpu->m_t0.d[1] == 0xffffffff && g_cpu->m_t0.d[0] == 0x80000000);

  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmaddh ノーマル
BEGIN_TESTF(r5900_mmi2_pmaddh_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[3] = 0x00027fff; g_cpu->m_a0.d[2] = 0xffff8000;  g_cpu->m_a0.d[1] = 0x00027fff; g_cpu->m_a0.d[0] = 0xffff8000;
    g_cpu->m_s0.d[3] = 0x00037fff; g_cpu->m_s0.d[2] = 0xffff8000;  g_cpu->m_s0.d[1] = 0xffff8000; g_cpu->m_s0.d[0] = 0x00020005;
    g_cpu->m_hi.d[3] = 0x88888888; g_cpu->m_hi.d[2] = 0x77777777;  g_cpu->m_hi.d[1] = 0x44444444; g_cpu->m_hi.d[0] = 0x33333333;
    g_cpu->m_lo.d[3] = 0x66666666; g_cpu->m_lo.d[2] = 0x55555555;  g_cpu->m_lo.d[1] = 0x22222222; g_cpu->m_lo.d[0] = 0x11111111;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x8888888e && g_cpu->m_hi.d[2] == 0xb7767778 && g_cpu->m_hi.d[1] == 0x44444442 && g_cpu->m_hi.d[0] == 0xf333b333);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x66666667 && g_cpu->m_lo.d[2] == 0x95555555 && g_cpu->m_lo.d[1] == 0x22222220 && g_cpu->m_lo.d[0] == 0x110e9111);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0xb7767778 && g_cpu->m_t0.d[2] == 0x95555555 && g_cpu->m_t0.d[1] == 0xf333b333 && g_cpu->m_t0.d[0] == 0x110e9111);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmsubh ノーマル
BEGIN_TESTF(r5900_mmi2_pmsubh_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[3] = 0x00027fff; g_cpu->m_a0.d[2] = 0xffff8000;  g_cpu->m_a0.d[1] = 0x00027fff; g_cpu->m_a0.d[0] = 0xffff8000;
    g_cpu->m_s0.d[3] = 0x00037fff; g_cpu->m_s0.d[2] = 0xffff8000;  g_cpu->m_s0.d[1] = 0xffff8000; g_cpu->m_s0.d[0] = 0x00020005;
    g_cpu->m_hi.d[3] = 0x88888888; g_cpu->m_hi.d[2] = 0x77777777;  g_cpu->m_hi.d[1] = 0x44444444; g_cpu->m_hi.d[0] = 0x33333333;
    g_cpu->m_lo.d[3] = 0x66666666; g_cpu->m_lo.d[2] = 0x55555555;  g_cpu->m_lo.d[1] = 0x22222222; g_cpu->m_lo.d[0] = 0x11111111;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x88888882 && g_cpu->m_hi.d[2] == 0x37787776 && g_cpu->m_hi.d[1] == 0x44444446 && g_cpu->m_hi.d[0] == 0x7332b333);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x66666665 && g_cpu->m_lo.d[2] == 0x15555555 && g_cpu->m_lo.d[1] == 0x22222224 && g_cpu->m_lo.d[0] == 0x11139111);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x37787776 && g_cpu->m_t0.d[2] == 0x15555555 && g_cpu->m_t0.d[1] == 0x7332b333 && g_cpu->m_t0.d[0] == 0x11139111);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmultw ノーマル
BEGIN_TESTF(r5900_mmi2_pmultw_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[2] = 0x00010000; g_cpu->m_t0.d[2] = 0x00010000;  g_cpu->m_a0.d[0] = 0x00000020; g_cpu->m_t0.d[0] = 0x00000045;
    g_cpu->m_a1.d[2] = 0xffffffff; g_cpu->m_t1.d[2] = 0x00000100;  g_cpu->m_a1.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000002;
    g_cpu->m_a2.d[2] = 0xffffffff; g_cpu->m_t2.d[2] = 0xffffffff;  g_cpu->m_a2.d[0] = 0x7fffffff; g_cpu->m_t2.d[0] = 0x7fffffff;
    g_cpu->m_a3.d[2] = 0x00000000; g_cpu->m_t3.d[2] = 0x00000000;  g_cpu->m_a3.d[0] = 0x80000000; g_cpu->m_t3.d[0] = 0x7fffffff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x00000001 && g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00000000 && g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x000008a0);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_ze.d[3] == 0x00000000 && g_cpu->m_ze.d[2] == 0x00000000 && g_cpu->m_ze.d[1] == 0x00000000 && g_cpu->m_ze.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0xffffffff && g_cpu->m_a1.d[2] == 0xffffffff && g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xffffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0xffffff00 && g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xfffffffe);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_ze.d[3] == 0x00000000 && g_cpu->m_ze.d[2] == 0x00000000 && g_cpu->m_ze.d[1] == 0x00000000 && g_cpu->m_ze.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a2.d[3] == 0x00000000 && g_cpu->m_a2.d[2] == 0x00000000 && g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x3fffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x00000000 && g_cpu->m_s2.d[2] == 0x00000001 && g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000001);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[3] == g_cpu->m_a2.d[2] && g_cpu->m_v0.d[2] == g_cpu->m_s2.d[2] && g_cpu->m_v0.d[1] == g_cpu->m_a2.d[0] && g_cpu->m_v0.d[0] == g_cpu->m_s2.d[0]);

    WIN_ASSERT_TRUE(g_cpu->m_a3.d[3] == 0x00000000 && g_cpu->m_a3.d[2] == 0x00000000 && g_cpu->m_a3.d[1] == 0xffffffff && g_cpu->m_a3.d[0] == 0xc0000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[3] == 0x00000000 && g_cpu->m_s3.d[2] == 0x00000000 && g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x80000000);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[3] == g_cpu->m_a3.d[2] && g_cpu->m_v1.d[2] == g_cpu->m_s3.d[2] && g_cpu->m_v1.d[1] == g_cpu->m_a3.d[0] && g_cpu->m_v1.d[0] == g_cpu->m_s3.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmulth ノーマル
BEGIN_TESTF(r5900_mmi2_pmulth_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[3] = 0x00000020; g_cpu->m_a0.d[2] = 0x10010002;  g_cpu->m_a0.d[1] = 0xffff8000; g_cpu->m_a0.d[0] = 0x7fff8000;
    g_cpu->m_t0.d[3] = 0x00000033; g_cpu->m_t0.d[2] = 0x0100ffff;  g_cpu->m_t0.d[1] = 0xffff8000; g_cpu->m_t0.d[0] = 0x7fff7fff;

    g_cpu->m_a1.d[3] = 0x00000020; g_cpu->m_a1.d[2] = 0x10010002;  g_cpu->m_a1.d[1] = 0xffff8000; g_cpu->m_a1.d[0] = 0x7fff8000;
    g_cpu->m_t1.d[3] = 0x00000033; g_cpu->m_t1.d[2] = 0x0100ffff;  g_cpu->m_t1.d[1] = 0xffff8000; g_cpu->m_t1.d[0] = 0x7fff7fff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[0] == 0xc0008000);  // lo.d[0] = a0.w[0] * t0.w[0]
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x3fff0001);  // lo.d[1] = a0.w[1] * t0.w[1]
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 0x40000000);  // hi.d[0] = a0.w[2] * t0.w[2]
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000001);  // hi.d[1] = a0.w[3] * t0.w[3]
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[2] == 0xfffffffe);  // lo.d[2] = a0.w[4] * t0.w[4]
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00100100);  // lo.d[3] = a0.w[5] * t0.w[5]
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[2] == 0x00000660);  // hi.d[2] = a0.w[6] * t0.w[6]
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000);  // hi.d[3] = a0.w[7] * t0.w[7]
    WIN_ASSERT_TRUE(g_cpu->m_ze.d[3] == 0x00000000 && g_cpu->m_ze.d[2] == 0x00000000 && g_cpu->m_ze.d[1] == 0x00000000 && g_cpu->m_ze.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_s1.d[0] == 0xc0008000);  // lo.d[0] = a1.w[0] * t1.w[0]
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x3fff0001);  // lo.d[1] = a1.w[1] * t1.w[1]
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 0x40000000);  // hi.d[0] = a1.w[2] * t1.w[2]
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000001);  // hi.d[1] = a1.w[3] * t1.w[3]
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[2] == 0xfffffffe);  // lo.d[2] = a1.w[4] * t1.w[4]
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x00100100);  // lo.d[3] = a1.w[5] * t1.w[5]
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[2] == 0x00000660);  // hi.d[2] = a1.w[6] * t1.w[6]
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x00000000);  // hi.d[3] = a1.w[7] * t1.w[7]
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[3] == g_cpu->m_a1.d[2] && g_cpu->m_v0.d[2] == g_cpu->m_s1.d[2] && g_cpu->m_v0.d[1] == g_cpu->m_a1.d[0] && g_cpu->m_v0.d[0] == g_cpu->m_s1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmfhi pmflo ノーマル
BEGIN_TESTF(r5900_mmi2_pmfhi_pmflo_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = 0x01234567; g_cpu->m_hi.d[2] = 0x89abcdef; g_cpu->m_hi.d[1] = 0xcccccccc; g_cpu->m_hi.d[0] = 0x88888888;
    g_cpu->m_lo.d[3] = 0x11111111; g_cpu->m_lo.d[2] = 0x22222222; g_cpu->m_lo.d[1] = 0x12345678; g_cpu->m_lo.d[0] = 0x55556666;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x01234567 && g_cpu->m_a0.d[2] == 0x89abcdef && g_cpu->m_a0.d[1] == 0xcccccccc && g_cpu->m_a0.d[0] == 0x88888888);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x11111111 && g_cpu->m_a1.d[2] == 0x22222222 && g_cpu->m_a1.d[1] == 0x12345678 && g_cpu->m_a1.d[0] == 0x55556666);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pcpyld ノーマル
BEGIN_TESTF(r5900_mmi2_pcpyld_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[3] = 0x11111111; g_cpu->m_s0.d[2] = 0x22222222; g_cpu->m_s0.d[1] = 0x33333333; g_cpu->m_s0.d[0] = 0x44444444;
    g_cpu->m_t0.d[3] = 0x55555555; g_cpu->m_t0.d[2] = 0x66666666; g_cpu->m_t0.d[1] = 0x77777777; g_cpu->m_t0.d[0] = 0x88888888;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x33333333 && g_cpu->m_a0.d[2] == 0x44444444 && g_cpu->m_a0.d[1] == 0x77777777 && g_cpu->m_a0.d[0] == 0x88888888);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pinth ノーマル
BEGIN_TESTF(r5900_mmi2_pinth_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[3] = 0x00001111; g_cpu->m_s0.d[2] = 0x22223333;  g_cpu->m_s0.d[1] = 0x44445555; g_cpu->m_s0.d[0] = 0x66667777;
    g_cpu->m_t0.d[3] = 0x88889999; g_cpu->m_t0.d[2] = 0xaaaabbbb;  g_cpu->m_t0.d[1] = 0xccccdddd; g_cpu->m_t0.d[0] = 0xeeeeffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x0000cccc && g_cpu->m_a0.d[2] == 0x1111dddd && g_cpu->m_a0.d[1] == 0x2222eeee && g_cpu->m_a0.d[0] == 0x3333ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pexew pexeh ノーマル
BEGIN_TESTF(r5900_mmi2_pexew_pexeh_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[3] = 0x11112222; g_cpu->m_t0.d[2] = 0x33334444; g_cpu->m_t0.d[1] = 0x55556666; g_cpu->m_t0.d[0] = 0x77778888;
    g_cpu->m_t1.d[3] = 0x11112222; g_cpu->m_t1.d[2] = 0x33334444; g_cpu->m_t1.d[1] = 0x55556666; g_cpu->m_t1.d[0] = 0x77778888;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x11112222 && g_cpu->m_a0.d[2] == 0x77778888 && g_cpu->m_a0.d[1] == 0x55556666 && g_cpu->m_a0.d[0] == 0x33334444);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x11114444 && g_cpu->m_a1.d[2] == 0x33332222 && g_cpu->m_a1.d[1] == 0x55558888 && g_cpu->m_a1.d[0] == 0x77776666);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// prevh prot3w ノーマル
BEGIN_TESTF(r5900_mmi2_prevh_prot3w_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[3] = 0x11112222; g_cpu->m_t0.d[2] = 0x33334444; g_cpu->m_t0.d[1] = 0x55556666; g_cpu->m_t0.d[0] = 0x77778888;
    g_cpu->m_t1.d[3] = 0x11112222; g_cpu->m_t1.d[2] = 0x33334444; g_cpu->m_t1.d[1] = 0x55556666; g_cpu->m_t1.d[0] = 0x77778888;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x44443333 && g_cpu->m_a0.d[2] == 0x22221111 && g_cpu->m_a0.d[1] == 0x88887777 && g_cpu->m_a0.d[0] == 0x66665555);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x11112222 && g_cpu->m_a1.d[2] == 0x77778888 && g_cpu->m_a1.d[1] == 0x33334444 && g_cpu->m_a1.d[0] == 0x55556666);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pand pxor ノーマル
BEGIN_TESTF(r5900_mmi2_pand_pxor_001, r5900_mmi2_fix_01){
  try
  {
    OPEN_ELF("mmi2");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00110000; g_cpu->m_s0.d[2] = 0x334466ff; g_cpu->m_s0.d[1] = 0x01030507; g_cpu->m_s0.d[0] = 0x12345678;
    g_cpu->m_t0.d[3] = 0x000088ff; g_cpu->m_t0.d[2] = 0x22cc6688; g_cpu->m_t0.d[1] = 0x20406080; g_cpu->m_t0.d[0] = 0xedcba987;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x22446688 && g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);  // and
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x001188ff && g_cpu->m_a1.d[2] == 0x11880077 && g_cpu->m_a1.d[1] == 0x21436587 && g_cpu->m_a1.d[0] == 0xffffffff);  // xor
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
