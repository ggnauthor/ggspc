#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_mmi0_fix_01);

SETUP(r5900_mmi0_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_mmi0_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// paddw ノーマル
BEGIN_TESTF(r5900_mmi0_paddw_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00050000 && g_cpu->m_s0.d[2] == 0x80017fff && g_cpu->m_s0.d[1] == 0x80067ff4 && g_cpu->m_s0.d[0] == 0x0000ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// paddh ノーマル
BEGIN_TESTF(r5900_mmi0_paddh_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00040000 && g_cpu->m_s0.d[2] == 0x80007fff && g_cpu->m_s0.d[1] == 0x80057ff4 && g_cpu->m_s0.d[0] == 0x0000ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// paddb ノーマル
BEGIN_TESTF(r5900_mmi0_paddb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x0004ff00 && g_cpu->m_s0.d[2] == 0x7f007fff && g_cpu->m_s0.d[1] == 0x80057ef4 && g_cpu->m_s0.d[0] == 0xff00ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubw ノーマル
BEGIN_TESTF(r5900_mmi0_psubw_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xfffefffe && g_cpu->m_s0.d[2] == 0x7ffd8001 && g_cpu->m_s0.d[1] == 0x80057ff6 && g_cpu->m_s0.d[0] == 0xfffd0001);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubh ノーマル
BEGIN_TESTF(r5900_mmi0_psubh_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xfffefffe && g_cpu->m_s0.d[2] == 0x7ffe8001 && g_cpu->m_s0.d[1] == 0x80057ff6 && g_cpu->m_s0.d[0] == 0xfffe0001);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubb ノーマル
BEGIN_TESTF(r5900_mmi0_psubb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00fefffe && g_cpu->m_s0.d[2] == 0x7ffe8101 && g_cpu->m_s0.d[1] == 0x800580f6 && g_cpu->m_s0.d[0] == 0xfffe0101);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// paddsw ノーマル
BEGIN_TESTF(r5900_mmi0_paddsw_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000020; g_cpu->m_s0.d[2] = 0x80000033; g_cpu->m_s0.d[1] = 0x40000012; g_cpu->m_s0.d[0] = 0x30000045;
    g_cpu->m_t0.d[3] = 0x80000020; g_cpu->m_t0.d[2] = 0x00000033; g_cpu->m_t0.d[1] = 0x40000111; g_cpu->m_t0.d[0] = 0x40000045;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x7fffffff; g_cpu->m_s1.d[1] = 0x00000012; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x00000001; g_cpu->m_t1.d[2] = 0x00000001; g_cpu->m_t1.d[1] = 0xfffffff0; g_cpu->m_t1.d[0] = 0xffffffff;
    
    g_cpu->m_s2.d[3] = 0x80000012; g_cpu->m_s2.d[2] = 0xffffff55; g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_t2.d[3] = 0xffffff55; g_cpu->m_t2.d[2] = 0x80000012; g_cpu->m_t2.d[1] = 0xffffffff; g_cpu->m_t2.d[0] = 0x80000000;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x80000040 && g_cpu->m_s0.d[2] == 0x80000066 && g_cpu->m_s0.d[1] == 0x7fffffff && g_cpu->m_s0.d[0] == 0x7000008a);

    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x00000000 && g_cpu->m_s1.d[2] == 0x7fffffff && g_cpu->m_s1.d[1] == 0x00000002 && g_cpu->m_s1.d[0] == 0xfffffffe);

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x80000000 && g_cpu->m_s2.d[2] == 0x80000000 && g_cpu->m_s2.d[1] == 0x80000000 && g_cpu->m_s2.d[0] == 0x80000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// paddsh ノーマル
BEGIN_TESTF(r5900_mmi0_paddsh_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00040000 && g_cpu->m_s0.d[2] == 0x7fff8000 && g_cpu->m_s0.d[1] == 0x80057ff4 && g_cpu->m_s0.d[0] == 0x0000ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// paddsb ノーマル
BEGIN_TESTF(r5900_mmi0_paddsb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0xff05fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0101ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x0004ff00 && g_cpu->m_s0.d[2] == 0x7f0080ff && g_cpu->m_s0.d[1] == 0x80057ef4 && g_cpu->m_s0.d[0] == 0xff00ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubsw ノーマル
BEGIN_TESTF(r5900_mmi0_psubsw_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000020; g_cpu->m_s0.d[2] = 0x00001033; g_cpu->m_s0.d[1] = 0x40000012; g_cpu->m_s0.d[0] = 0x00000030;
    g_cpu->m_t0.d[3] = 0x00000020; g_cpu->m_t0.d[2] = 0x00000033; g_cpu->m_t0.d[1] = 0x01000000; g_cpu->m_t0.d[0] = 0x00000031;
    
    g_cpu->m_s1.d[3] = 0x7fffffff; g_cpu->m_s1.d[2] = 0x80000000; g_cpu->m_s1.d[1] = 0xfffffffe; g_cpu->m_s1.d[0] = 0x00000001;
    g_cpu->m_t1.d[3] = 0xffffffff; g_cpu->m_t1.d[2] = 0x00000001; g_cpu->m_t1.d[1] = 0x7fffffff; g_cpu->m_t1.d[0] = 0x80000000;
    
    g_cpu->m_s2.d[3] = 0x7fffffff; g_cpu->m_s2.d[2] = 0x80000000; g_cpu->m_s2.d[1] = 0x7fffffff; g_cpu->m_s2.d[0] = 0x80000000;
    g_cpu->m_t2.d[3] = 0x80000000; g_cpu->m_t2.d[2] = 0x7fffffff; g_cpu->m_t2.d[1] = 0x7fffffff; g_cpu->m_t2.d[0] = 0x80000000;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00001000 && g_cpu->m_s0.d[1] == 0x3f000012 && g_cpu->m_s0.d[0] == 0xffffffff);

    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x7fffffff && g_cpu->m_s1.d[2] == 0x80000000 && g_cpu->m_s1.d[1] == 0x80000000 && g_cpu->m_s1.d[0] == 0x7fffffff);

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x7fffffff && g_cpu->m_s2.d[2] == 0x80000000 && g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubsh ノーマル
BEGIN_TESTF(r5900_mmi0_psubsh_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0fff7fff; g_cpu->m_s0.d[0] = 0xf0008000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x8000ffff; g_cpu->m_t0.d[0] = 0x7fff0001;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xfffefffe && g_cpu->m_s0.d[2] == 0x7ffe8001 && g_cpu->m_s0.d[1] == 0x7fff7fff && g_cpu->m_s0.d[0] == 0x80008000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubsb ノーマル
BEGIN_TESTF(r5900_mmi0_psubsb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0xff05fff5; g_cpu->m_s0.d[0] = 0xff7f8000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0101ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x00ff01ff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00fefffe && g_cpu->m_s0.d[2] == 0x7efe8101 && g_cpu->m_s0.d[1] == 0x7f0580f6 && g_cpu->m_s0.d[0] == 0xff7f8001);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pcgtw pcgth pcgtb ノーマル
BEGIN_TESTF(r5900_mmi0_pcgtw_pcgth_pcgtb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000030; g_cpu->m_s0.d[2] = 0x00000122; g_cpu->m_s0.d[1] = 0x00000222; g_cpu->m_s0.d[0] = 0x7fffffff;
    g_cpu->m_t0.d[3] = 0x00000015; g_cpu->m_t0.d[2] = 0x00000333; g_cpu->m_t0.d[1] = 0x00000222; g_cpu->m_t0.d[0] = 0x7ffffffe;
    
    g_cpu->m_s1.d[3] = 0xfffffffe; g_cpu->m_s1.d[2] = 0x7fffffff; g_cpu->m_s1.d[1] = 0x7fffffff; g_cpu->m_s1.d[0] = 0x80000000;
    g_cpu->m_t1.d[3] = 0xfffffffc; g_cpu->m_t1.d[2] = 0x80000000; g_cpu->m_t1.d[1] = 0x7fffffff; g_cpu->m_t1.d[0] = 0x7fffffff;
    
    g_cpu->m_s2.d[3] = 0x00000004; g_cpu->m_s2.d[2] = 0x0003ffff; g_cpu->m_s2.d[1] = 0x00337fff; g_cpu->m_s2.d[0] = 0x80000000;
    g_cpu->m_t2.d[3] = 0x00000002; g_cpu->m_t2.d[2] = 0x00050000; g_cpu->m_t2.d[1] = 0x00338000; g_cpu->m_t2.d[0] = 0x7fffffff;

    g_cpu->m_s3.d[3] = 0x00040300; g_cpu->m_s3.d[2] = 0xfefffecc; g_cpu->m_s3.d[1] = 0x7f7f7f7f; g_cpu->m_s3.d[0] = 0x80808080;
    g_cpu->m_t3.d[3] = 0x000205ff; g_cpu->m_t3.d[2] = 0x02feffdd; g_cpu->m_t3.d[1] = 0x007e80ff; g_cpu->m_t3.d[0] = 0x00817fff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xffffffff && g_cpu->m_s0.d[2] == 0x00000000 && g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0xffffffff && g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x0000ffff && g_cpu->m_s2.d[2] == 0x00000000 && g_cpu->m_s2.d[1] == 0x0000ffff && g_cpu->m_s2.d[0] == 0x0000ffff);

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[3] == 0x00ff00ff && g_cpu->m_s3.d[2] == 0x00ff0000 && g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pextlw pextlh pextlb ノーマル
BEGIN_TESTF(r5900_mmi0_pextlw_pextlh_pextlb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0xffffffff; g_cpu->m_s0.d[2] = 0xffffffff; g_cpu->m_s0.d[1] = 0x100f0e0d; g_cpu->m_s0.d[0] = 0x0c0b0a09;
    g_cpu->m_t0.d[3] = 0xffffffff; g_cpu->m_t0.d[2] = 0xffffffff; g_cpu->m_t0.d[1] = 0x08070605; g_cpu->m_t0.d[0] = 0x04030201;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x100f0e0d && g_cpu->m_a0.d[2] == 0x08070605 && g_cpu->m_a0.d[1] == 0x0c0b0a09 && g_cpu->m_a0.d[0] == 0x04030201);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x100f0807 && g_cpu->m_a1.d[2] == 0x0e0d0605 && g_cpu->m_a1.d[1] == 0x0c0b0403 && g_cpu->m_a1.d[0] == 0x0a090201);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[3] == 0x10080f07 && g_cpu->m_a2.d[2] == 0x0e060d05 && g_cpu->m_a2.d[1] == 0x0c040b03 && g_cpu->m_a2.d[0] == 0x0a020901);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xffffffff && g_cpu->m_s0.d[2] == 0xffffffff && g_cpu->m_s0.d[1] == 0x100f0e0d && g_cpu->m_s0.d[0] == 0x0c0b0a09);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0xffffffff && g_cpu->m_t0.d[2] == 0xffffffff && g_cpu->m_t0.d[1] == 0x08070605 && g_cpu->m_t0.d[0] == 0x04030201);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmaxw ノーマル
BEGIN_TESTF(r5900_mmi0_pmaxw_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00000001; g_cpu->m_s0.d[1] = 0x00001000; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_t0.d[3] = 0x00000000; g_cpu->m_t0.d[2] = 0x00000010; g_cpu->m_t0.d[1] = 0x00000100; g_cpu->m_t0.d[0] = 0xfffffffe;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x7fffffff; g_cpu->m_s1.d[1] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x7fffffff; g_cpu->m_t1.d[2] = 0xffffffff; g_cpu->m_t1.d[1] = 0x80000000; g_cpu->m_t1.d[0] = 0x80000000;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x00000010 && g_cpu->m_a0.d[1] == 0x00001000 && g_cpu->m_a0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x7fffffff && g_cpu->m_a1.d[2] == 0x7fffffff && g_cpu->m_a1.d[1] == 0x7fffffff && g_cpu->m_a1.d[0] == 0xffffffff);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00000010 && g_cpu->m_s0.d[1] == 0x00001000 && g_cpu->m_s0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x7fffffff && g_cpu->m_s1.d[2] == 0x7fffffff && g_cpu->m_s1.d[1] == 0x7fffffff && g_cpu->m_s1.d[0] == 0xffffffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmaxh ノーマル
BEGIN_TESTF(r5900_mmi0_pmaxh_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00200001; g_cpu->m_s0.d[1] = 0x00001000; g_cpu->m_s0.d[0] = 0xfffeffff;
    g_cpu->m_t0.d[3] = 0x00000000; g_cpu->m_t0.d[2] = 0x00100010; g_cpu->m_t0.d[1] = 0xffff0100; g_cpu->m_t0.d[0] = 0xfffffffe;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x7fffffff; g_cpu->m_s1.d[1] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x7fffffff; g_cpu->m_t1.d[2] = 0xffffffff; g_cpu->m_t1.d[1] = 0x80000000; g_cpu->m_t1.d[0] = 0x80000000;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x00200010 && g_cpu->m_a0.d[1] == 0x00001000 && g_cpu->m_a0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x7fffffff && g_cpu->m_a1.d[2] == 0x7fffffff && g_cpu->m_a1.d[1] == 0x7fff0000 && g_cpu->m_a1.d[0] == 0xffff0000);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00200010 && g_cpu->m_s0.d[1] == 0x00001000 && g_cpu->m_s0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x7fffffff && g_cpu->m_s1.d[2] == 0x7fffffff && g_cpu->m_s1.d[1] == 0x7fff0000 && g_cpu->m_s1.d[0] == 0xffff0000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ppacw ppach ppacb ノーマル
BEGIN_TESTF(r5900_mmi0_ppacw_ppach_ppacb_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00112233; g_cpu->m_s0.d[2] = 0x44556677; g_cpu->m_s0.d[1] = 0x8899aabb; g_cpu->m_s0.d[0] = 0xccddeeff;
    g_cpu->m_t0.d[3] = 0x80818283; g_cpu->m_t0.d[2] = 0x84858687; g_cpu->m_t0.d[1] = 0x88898a8b; g_cpu->m_t0.d[0] = 0x8c8d8e8f;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x44556677 && g_cpu->m_a0.d[2] == 0xccddeeff && g_cpu->m_a0.d[1] == 0x84858687 && g_cpu->m_a0.d[0] == 0x8c8d8e8f);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x22336677 && g_cpu->m_a1.d[2] == 0xaabbeeff && g_cpu->m_a1.d[1] == 0x82838687 && g_cpu->m_a1.d[0] == 0x8a8b8e8f);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[3] == 0x11335577 && g_cpu->m_a2.d[2] == 0x99bbddff && g_cpu->m_a2.d[1] == 0x81838587 && g_cpu->m_a2.d[0] == 0x898b8d8f);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00112233 && g_cpu->m_s0.d[2] == 0x44556677 && g_cpu->m_s0.d[1] == 0x8899aabb && g_cpu->m_s0.d[0] == 0xccddeeff);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x80818283 && g_cpu->m_t0.d[2] == 0x84858687 && g_cpu->m_t0.d[1] == 0x88898a8b && g_cpu->m_t0.d[0] == 0x8c8d8e8f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ppac5 pext5 ノーマル
BEGIN_TESTF(r5900_mmi0_ppac5_pext5_001, r5900_mmi0_fix_01){
  try
  {
    OPEN_ELF("mmi0");
    INIT_GPR(0xcccccccc);

    // 8:8:8:8 > 1:5:5:5
    g_cpu->m_t0.d[3] = 0x00ff0000; g_cpu->m_t0.d[2] = 0x8000ff00; g_cpu->m_t0.d[1] = 0x000000ff; g_cpu->m_t0.d[0] = 0x7f7f7f7f;
    // 1:5:5:5 > 8:8:8:8
    g_cpu->m_t1.d[3] = 0xffff7c00; g_cpu->m_t1.d[2] = 0x000083e0; g_cpu->m_t1.d[1] = 0x0000001f; g_cpu->m_t1.d[0] = 0x00003def;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00007c00 && g_cpu->m_s0.d[2] == 0x000083e0 && g_cpu->m_s0.d[1] == 0x0000001f && g_cpu->m_s0.d[0] == 0x00003def);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x00f80000 && g_cpu->m_s1.d[2] == 0x8000f800 && g_cpu->m_s1.d[1] == 0x000000f8 && g_cpu->m_s1.d[0] == 0x00787878);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
