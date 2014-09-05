#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_mmi1_fix_01);

SETUP(r5900_mmi1_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_mmi1_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// padduw ノーマル
BEGIN_TESTF(r5900_mmi1_padduw_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000020; g_cpu->m_s0.d[2] = 0x80000033; g_cpu->m_s0.d[1] = 0x40000012; g_cpu->m_s0.d[0] = 0x30000045;
    g_cpu->m_t0.d[3] = 0x80000020; g_cpu->m_t0.d[2] = 0x00000033; g_cpu->m_t0.d[1] = 0x40000111; g_cpu->m_t0.d[0] = 0x40000045;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x80000033; g_cpu->m_s1.d[1] = 0x00000012; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x00000001; g_cpu->m_t1.d[2] = 0x00000033; g_cpu->m_t1.d[1] = 0xfffffff0; g_cpu->m_t1.d[0] = 0xffffffff;
    
    g_cpu->m_s2.d[3] = 0x00000020; g_cpu->m_s2.d[2] = 0xffffffff; g_cpu->m_s2.d[1] = 0x00000012; g_cpu->m_s2.d[0] = 0x00000045;
    g_cpu->m_t2.d[3] = 0x00000020; g_cpu->m_t2.d[2] = 0x00000045; g_cpu->m_t2.d[1] = 0xffffff00; g_cpu->m_t2.d[0] = 0x00000045;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x80000040 && g_cpu->m_s0.d[2] == 0x80000066 && g_cpu->m_s0.d[1] == 0x80000123 && g_cpu->m_s0.d[0] == 0x7000008a);

    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0x80000066 && g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xffffffff);

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x00000040 && g_cpu->m_s2.d[2] == 0xffffffff && g_cpu->m_s2.d[1] == 0xffffff12 && g_cpu->m_s2.d[0] == 0x0000008a);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// padduh ノーマル
BEGIN_TESTF(r5900_mmi1_padduh_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0001ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x0004ffff && g_cpu->m_s0.d[2] == 0x8000ffff && g_cpu->m_s0.d[1] == 0x8005ffff && g_cpu->m_s0.d[0] == 0xffffffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// paddub ノーマル
BEGIN_TESTF(r5900_mmi1_paddub_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0xff05fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0101ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x0004ffff && g_cpu->m_s0.d[2] == 0x80ffffff && g_cpu->m_s0.d[1] == 0xff05ffff && g_cpu->m_s0.d[0] == 0xffffffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubuw ノーマル
BEGIN_TESTF(r5900_mmi1_psubuw_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000020; g_cpu->m_s0.d[2] = 0x80000033; g_cpu->m_s0.d[1] = 0x30000012; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_t0.d[3] = 0x00000020; g_cpu->m_t0.d[2] = 0x70000033; g_cpu->m_t0.d[1] = 0x40000111; g_cpu->m_t0.d[0] = 0xfffffffe;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x80000033; g_cpu->m_s1.d[1] = 0x00000012; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x00000001; g_cpu->m_t1.d[2] = 0x00000033; g_cpu->m_t1.d[1] = 0xfffffff0; g_cpu->m_t1.d[0] = 0x00000001;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x10000000 && g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xfffffffe && g_cpu->m_s1.d[2] == 0x80000000 && g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0xfffffffe);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubuh ノーマル
BEGIN_TESTF(r5900_mmi1_psubuh_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7f058000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0006ffff; g_cpu->m_t0.d[1] = 0x80007fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x0000fffe && g_cpu->m_s0.d[2] == 0x7eff0000 && g_cpu->m_s0.d[1] == 0x00007ff6 && g_cpu->m_s0.d[0] == 0xfffe0000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// psubub ノーマル
BEGIN_TESTF(r5900_mmi1_psubub_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x0001ffff; g_cpu->m_s0.d[2] = 0x7fff8000; g_cpu->m_s0.d[1] = 0x0005fff5; g_cpu->m_s0.d[0] = 0xffff0000;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0x0101ffff; g_cpu->m_t0.d[1] = 0xffff7fff; g_cpu->m_t0.d[0] = 0x0001ffff;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x0000fffe && g_cpu->m_s0.d[2] == 0x7efe0000 && g_cpu->m_s0.d[1] == 0x00008000 && g_cpu->m_s0.d[0] == 0xfffe0000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// padsbh ノーマル
BEGIN_TESTF(r5900_mmi1_padsbh_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00017fff; g_cpu->m_s0.d[2] = 0x80000001; g_cpu->m_s0.d[1] = 0x00017fff; g_cpu->m_s0.d[0] = 0x80000001;
    g_cpu->m_t0.d[3] = 0x00030001; g_cpu->m_t0.d[2] = 0xffffffff; g_cpu->m_t0.d[1] = 0x00030001; g_cpu->m_t0.d[0] = 0xffffffff;
    
    g_cpu->m_s1.d[3] = 0xffff0000; g_cpu->m_s1.d[2] = 0x4000c000; g_cpu->m_s1.d[1] = 0xffff0000; g_cpu->m_s1.d[0] = 0x4000c000;
    g_cpu->m_t1.d[3] = 0x0001ffff; g_cpu->m_t1.d[2] = 0x4000c000; g_cpu->m_t1.d[1] = 0x0001ffff; g_cpu->m_t1.d[0] = 0x4000c000;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00048000 && g_cpu->m_a0.d[2] == 0x7fff0000 && g_cpu->m_a0.d[1] == 0xfffe7ffe && g_cpu->m_a0.d[0] == 0x80010002);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x0000ffff && g_cpu->m_s1.d[2] == 0x80008000 && g_cpu->m_s1.d[1] == 0xfffe0001 && g_cpu->m_s1.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pabsw pabsh ノーマル
BEGIN_TESTF(r5900_mmi1_pabsw_pabsh_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x7fffffff; g_cpu->m_s0.d[1] = 0xffffffff; g_cpu->m_s0.d[0] = 0x80000000;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x7fffffff && g_cpu->m_s0.d[1] == 0x00000001 && g_cpu->m_s0.d[0] == 0x7fffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x00000000 && g_cpu->m_s1.d[2] == 0x7fff0001 && g_cpu->m_s1.d[1] == 0x00010001 && g_cpu->m_s1.d[0] == 0x7fff0000);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == g_cpu->m_a0.d[3] && g_cpu->m_s0.d[2] == g_cpu->m_a0.d[2] && g_cpu->m_s0.d[1] == g_cpu->m_a0.d[1] && g_cpu->m_s0.d[0] == g_cpu->m_a0.d[0]);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == g_cpu->m_a1.d[3] && g_cpu->m_s1.d[2] == g_cpu->m_a1.d[2] && g_cpu->m_s1.d[1] == g_cpu->m_a1.d[1] && g_cpu->m_s1.d[0] == g_cpu->m_a1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pceqw pceqh pceqb ノーマル
BEGIN_TESTF(r5900_mmi1_pceqw_pceqh_pceqb_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x01200024; g_cpu->m_s0.d[2] = 0xcf45ff2f; g_cpu->m_s0.d[1] = 0x70000222; g_cpu->m_s0.d[0] = 0x8fffffff;
    g_cpu->m_t0.d[3] = 0x01200024; g_cpu->m_t0.d[2] = 0xcf45ff2f; g_cpu->m_t0.d[1] = 0x70550222; g_cpu->m_t0.d[0] = 0x8ffffffe;
    
    g_cpu->m_s1.d[3] = 0x00000001; g_cpu->m_s1.d[2] = 0x7ff0ffff; g_cpu->m_s1.d[1] = 0x1234eeee; g_cpu->m_s1.d[0] = 0x12341234;
    g_cpu->m_t1.d[3] = 0x00000001; g_cpu->m_t1.d[2] = 0x7fffffff; g_cpu->m_t1.d[1] = 0x1235eeed; g_cpu->m_t1.d[0] = 0xee225456;
    
    g_cpu->m_s2.d[3] = 0x00000001; g_cpu->m_s2.d[2] = 0x800103ff; g_cpu->m_s2.d[1] = 0x1234eeee; g_cpu->m_s2.d[0] = 0x55555555;
    g_cpu->m_t2.d[3] = 0x00000001; g_cpu->m_t2.d[2] = 0x850104ff; g_cpu->m_t2.d[1] = 0x1235eeed; g_cpu->m_t2.d[0] = 0x55545556;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xffffffff && g_cpu->m_s0.d[2] == 0xffffffff && g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0x0000ffff && g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0xffffffff && g_cpu->m_s2.d[2] == 0x00ff00ff && g_cpu->m_s2.d[1] == 0xff00ff00 && g_cpu->m_s2.d[0] == 0xff00ff00);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pextuw pextuh pextub ノーマル
BEGIN_TESTF(r5900_mmi1_pextuw_pextuh_pextub_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x100f0e0d; g_cpu->m_s0.d[2] = 0x0c0b0a09; g_cpu->m_s0.d[1] = 0xffffffff; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_t0.d[3] = 0x08070605; g_cpu->m_t0.d[2] = 0x04030201; g_cpu->m_t0.d[1] = 0xffffffff; g_cpu->m_t0.d[0] = 0xffffffff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x100f0e0d && g_cpu->m_a0.d[2] == 0x08070605 && g_cpu->m_a0.d[1] == 0x0c0b0a09 && g_cpu->m_a0.d[0] == 0x04030201);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x100f0807 && g_cpu->m_a1.d[2] == 0x0e0d0605 && g_cpu->m_a1.d[1] == 0x0c0b0403 && g_cpu->m_a1.d[0] == 0x0a090201);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[3] == 0x10080f07 && g_cpu->m_a2.d[2] == 0x0e060d05 && g_cpu->m_a2.d[1] == 0x0c040b03 && g_cpu->m_a2.d[0] == 0x0a020901);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x100f0e0d && g_cpu->m_s0.d[2] == 0x0c0b0a09 && g_cpu->m_s0.d[1] == 0xffffffff && g_cpu->m_s0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x08070605 && g_cpu->m_t0.d[2] == 0x04030201 && g_cpu->m_t0.d[1] == 0xffffffff && g_cpu->m_t0.d[0] == 0xffffffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pminw ノーマル
BEGIN_TESTF(r5900_mmi1_pminw_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00000001; g_cpu->m_s0.d[1] = 0x00001000; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_t0.d[3] = 0x00000000; g_cpu->m_t0.d[2] = 0x00000010; g_cpu->m_t0.d[1] = 0x00000100; g_cpu->m_t0.d[0] = 0xfffffffe;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x7fffffff; g_cpu->m_s1.d[1] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x7fffffff; g_cpu->m_t1.d[2] = 0xffffffff; g_cpu->m_t1.d[1] = 0x80000000; g_cpu->m_t1.d[0] = 0x80000000;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x00000001 && g_cpu->m_a0.d[1] == 0x00000100 && g_cpu->m_a0.d[0] == 0xfffffffe);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0xffffffff && g_cpu->m_a1.d[2] == 0xffffffff && g_cpu->m_a1.d[1] == 0x80000000 && g_cpu->m_a1.d[0] == 0x80000000);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00000001 && g_cpu->m_s0.d[1] == 0x00000100 && g_cpu->m_s0.d[0] == 0xfffffffe);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0xffffffff && g_cpu->m_s1.d[1] == 0x80000000 && g_cpu->m_s1.d[0] == 0x80000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pminh ノーマル
BEGIN_TESTF(r5900_mmi1_pminh_001, r5900_mmi1_fix_01){
  try
  {
    OPEN_ELF("mmi1");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00200001; g_cpu->m_s0.d[1] = 0x00001000; g_cpu->m_s0.d[0] = 0xfffeffff;
    g_cpu->m_t0.d[3] = 0x00000000; g_cpu->m_t0.d[2] = 0x00100010; g_cpu->m_t0.d[1] = 0xffff0100; g_cpu->m_t0.d[0] = 0xfffffffe;
    
    g_cpu->m_s1.d[3] = 0xffffffff; g_cpu->m_s1.d[2] = 0x7fffffff; g_cpu->m_s1.d[1] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_t1.d[3] = 0x7fffffff; g_cpu->m_t1.d[2] = 0xffffffff; g_cpu->m_t1.d[1] = 0x80000000; g_cpu->m_t1.d[0] = 0x80000000;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x00100001 && g_cpu->m_a0.d[1] == 0xffff0100 && g_cpu->m_a0.d[0] == 0xfffefffe);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0xffffffff && g_cpu->m_a1.d[2] == 0xffffffff && g_cpu->m_a1.d[1] == 0x8000ffff && g_cpu->m_a1.d[0] == 0x8000ffff);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00100001 && g_cpu->m_s0.d[1] == 0xffff0100 && g_cpu->m_s0.d[0] == 0xfffefffe);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0xffffffff && g_cpu->m_s1.d[1] == 0x8000ffff && g_cpu->m_s1.d[0] == 0x8000ffff);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
