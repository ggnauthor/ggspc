#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_misc_fix_01);

SETUP(r5900_misc_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_misc_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// j ノーマル＆遅延スロット
BEGIN_TESTF(r5900_misc_j_001, r5900_misc_fix_01)
{

  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[0] == 1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }

}END_TESTF

// jal ノーマル＆遅延スロット＆リターンアドレス
BEGIN_TESTF(r5900_misc_jal_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_ra.d[0] = 0xffffffff;
    g_cpu->m_ra.d[1] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 0x0010);
    WIN_ASSERT_TRUE(g_cpu->m_ra.d[1] == 0x00000000 && g_cpu->m_ra.d[0] == 0x0028);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// beq ノーマル＆遅延スロット＆上下ジャンプ
BEGIN_TESTF(r5900_misc_beq_001, r5900_misc_fix_01)
{
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[0] = 0x12345678; g_cpu->m_s0.d[1] = 0x51243578;
    g_cpu->m_s1.d[0] = 0x77777777; g_cpu->m_s1.d[1] = 0x51243578;
    g_cpu->m_s2.d[0] = 0x12345678; g_cpu->m_s2.d[1] = 0x51243578;
    g_cpu->m_s3.d[0] = 0x12345678; g_cpu->m_s3.d[1] = 0x77777777;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 2);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bne ノーマル＆遅延スロット＆上下ジャンプ
BEGIN_TESTF(r5900_misc_bne_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[0] = 0x12345678; g_cpu->m_s0.d[1] = 0x51243578;
    g_cpu->m_s1.d[0] = 0x77777777; g_cpu->m_s1.d[1] = 0x51243578;
    g_cpu->m_s2.d[0] = 0x12345678; g_cpu->m_s2.d[1] = 0x51243578;
    g_cpu->m_s3.d[0] = 0x12345678; g_cpu->m_s3.d[1] = 0x77777777;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 4);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// blez 0の場合＆負数の場合＆正数の場合＆遅延スロット＆上下ジャンプ
BEGIN_TESTF(r5900_misc_blez_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 2);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bgtz 0の場合＆負数の場合＆正数の場合＆遅延スロット＆上下ジャンプ
BEGIN_TESTF(r5900_misc_bgtz_001, r5900_misc_fix_01)
{
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == -1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// blezl 0の場合＆負数の場合＆正数の場合＆遅延スロット＆上下ジャンプ
BEGIN_TESTF(r5900_misc_blezl_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bgtzl 0の場合＆負数の場合＆正数の場合＆遅延スロット＆上下ジャンプ
BEGIN_TESTF(r5900_misc_bgtzl_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x80000000; g_cpu->m_s2.d[0] = 0x00000000;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0xffffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// addi ノーマル
BEGIN_TESTF(r5900_misc_addi_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->execute();
    for (int i = 1; i < 32; i++)
    {
      // 63..32bitに符号拡張された0x00000000が格納される
      WIN_ASSERT_TRUE(g_cpu->m_gpr[i].d[1] == 0x00000000);
      WIN_ASSERT_TRUE(g_cpu->m_gpr[i].d[2] == 0xcccccccc);
      WIN_ASSERT_TRUE(g_cpu->m_gpr[i].d[3] == 0xcccccccc);
    }
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[0] == 2);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[0] == 3);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 4);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 5);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 6);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 7);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 8);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 9);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 10);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 11);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 12);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 13);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 14);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[0] == 15);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[0] == 16);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[0] == 17);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[0] == 18);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[0] == 19);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[0] == 20);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[0] == 21);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[0] == 22);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[0] == 23);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[0] == 24);
    WIN_ASSERT_TRUE(g_cpu->m_t9.d[0] == 25);
    WIN_ASSERT_TRUE(g_cpu->m_k0.d[0] == 26);
    WIN_ASSERT_TRUE(g_cpu->m_k1.d[0] == 27);
    WIN_ASSERT_TRUE(g_cpu->m_gp.d[0] == 1027);
    WIN_ASSERT_TRUE(g_cpu->m_sp.d[0] == 3027);
    WIN_ASSERT_TRUE(g_cpu->m_fp.d[0] == 13027);
    WIN_ASSERT_TRUE(g_cpu->m_ra.d[0] == 33027);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 33026);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// addi オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_misc_addi_002, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x7fffffff;
    g_cpu->m_a1.d[0] = 0xffffffff;
    g_cpu->m_a2.d[0] = 0x00000000;
    g_cpu->m_a3.d[0] = 0x80000001;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0xcccccccc && g_cpu->m_a0.d[0] == 0x7fffffff);  // +側オーバーフローにより不変
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 1);      // -1 +  2
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0xfffffffe);  //  0 + -2 (符号拡張)
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0xcccccccc && g_cpu->m_a3.d[0] == 0x80000001);  // -側オーバーフローにより不変
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// addiu ノーマル
BEGIN_TESTF(r5900_misc_addiu_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->execute();
    for (int i = 1; i < 32; i++)
    {
      // 63..32bitに符号拡張された0x00000000が格納される
      WIN_ASSERT_TRUE(g_cpu->m_gpr[i].d[1] == 0x00000000);
      WIN_ASSERT_TRUE(g_cpu->m_gpr[i].d[2] == 0xcccccccc);
      WIN_ASSERT_TRUE(g_cpu->m_gpr[i].d[3] == 0xcccccccc);
    }
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[0] == 2);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[0] == 3);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[0] == 4);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[0] == 5);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[0] == 6);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[0] == 7);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 8);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 9);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 10);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 11);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 12);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 13);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 14);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[0] == 15);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[0] == 16);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[0] == 17);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[0] == 18);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[0] == 19);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[0] == 20);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[0] == 21);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[0] == 22);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[0] == 23);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[0] == 24);
    WIN_ASSERT_TRUE(g_cpu->m_t9.d[0] == 25);
    WIN_ASSERT_TRUE(g_cpu->m_k0.d[0] == 26);
    WIN_ASSERT_TRUE(g_cpu->m_k1.d[0] == 27);
    WIN_ASSERT_TRUE(g_cpu->m_gp.d[0] == 1027);
    WIN_ASSERT_TRUE(g_cpu->m_sp.d[0] == 3027);
    WIN_ASSERT_TRUE(g_cpu->m_fp.d[0] == 13027);
    WIN_ASSERT_TRUE(g_cpu->m_ra.d[0] == 33027);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 33026);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// addiu オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_misc_addiu_002, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x7fffffff;
    g_cpu->m_a1.d[0] = 0xffffffff;
    g_cpu->m_a2.d[0] = 0x00000000;
    g_cpu->m_a3.d[0] = 0x80000001;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0xffffffff && g_cpu->m_a0.d[0] == 0x80000001);  // 0x7fffffff + 2
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 1);      // -1 +  2
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0xfffffffe);  //  0 + -2 (符号拡張)
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x7fffffff);  // 0x80000001 - 2
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// slti ノーマル＆オーバーフロー
BEGIN_TESTF(r5900_misc_slti_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0x80000000; g_cpu->m_s1.d[0] = 0x00000000;  // 上位wordで-側オーバーフローするケースは存在しない（immが16bitなので）
    g_cpu->m_s2.d[1] = 0xffffffff; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0x00000000;  // 上位wordで+側オーバーフローするケース
    g_cpu->m_s4.d[1] = 0x00000000; g_cpu->m_s4.d[0] = 0x80000000;  // 上位wordが等しく下位wordで-側オーバーフローするケース
    g_cpu->m_s5.d[1] = 0xffffffff; g_cpu->m_s5.d[0] = 0x7fffffff;  // 上位wordが等しく下位wordで+側オーバーフローするケース
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[1] == 0x00000000 && g_cpu->m_t8.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == 0x00000000 && g_cpu->m_v0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000001);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sltiu ノーマル
BEGIN_TESTF(r5900_misc_sltiu_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00000000;
    g_cpu->m_s1.d[1] = 0x80000000; g_cpu->m_s1.d[0] = 0x00000000;
    g_cpu->m_s2.d[1] = 0xffffffff; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_s3.d[1] = 0x7fffffff; g_cpu->m_s3.d[0] = 0x00000000;
    g_cpu->m_s4.d[1] = 0x00000000; g_cpu->m_s4.d[0] = 0x80000000;
    g_cpu->m_s5.d[1] = 0xffffffff; g_cpu->m_s5.d[0] = 0x7fffffff;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[1] == 0x00000000 && g_cpu->m_t8.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == 0x00000000 && g_cpu->m_v0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// andi ノーマル
BEGIN_TESTF(r5900_misc_andi_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x12345678; g_cpu->m_s0.d[0] = 0x87654321;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004321);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00004020);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00004020);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ori ノーマル
BEGIN_TESTF(r5900_misc_ori_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x12345678; g_cpu->m_s0.d[0] = 0x87654321;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x12345678 && g_cpu->m_t0.d[0] == 0x8765ffff);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x12345678 && g_cpu->m_t1.d[0] == 0x87654321);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x12345678 && g_cpu->m_t2.d[0] == 0x8765f3f1);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x12345678 && g_cpu->m_s0.d[0] == 0x8765f3f1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// xori ノーマル
BEGIN_TESTF(r5900_misc_xori_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x12345678; g_cpu->m_s0.d[0] = 0x87654321;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x12345678 && g_cpu->m_t0.d[0] == 0x8765bcde);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x12345678 && g_cpu->m_t1.d[0] == 0x87654321);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x12345678 && g_cpu->m_t2.d[0] == 0x8765b3d1);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x12345678 && g_cpu->m_s0.d[0] == 0x8765b3d1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lui ノーマル＆符号拡張
BEGIN_TESTF(r5900_misc_lui_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x7fff0000);  // 符号拡張+
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000000);  // 符号拡張+
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0xffff0000);  // 符号拡張-
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0xffffffff && g_cpu->m_a3.d[0] == 0x80000000);  // 符号拡張-
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// daddi オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_misc_daddi_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00001234;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x7fffffff; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_s3.d[1] = 0x80000000; g_cpu->m_s3.d[0] = 0x00000000;
    g_cpu->m_s4.d[1] = 0x00000001; g_cpu->m_s4.d[0] = 0x00000000;
    g_cpu->m_s5.d[1] = 0x00000000; g_cpu->m_s5.d[0] = 0xffffffff;
    g_cpu->m_t2.d[1] = 0xcccccccc; g_cpu->m_t2.d[0] = 0xcccccccc;
    g_cpu->m_t3.d[1] = 0xcccccccc; g_cpu->m_t3.d[0] = 0xcccccccc;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00005555);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xcccccccc && g_cpu->m_t2.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0xfffffff0);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000001 && g_cpu->m_t5.d[0] == 0x0000000f);

    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0x00000001 && g_cpu->m_s5.d[0] == 0x0000000f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// daddiu オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_misc_daddiu_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00001234;
    g_cpu->m_s1.d[1] = 0xffffffff; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x7fffffff; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_s3.d[1] = 0x80000000; g_cpu->m_s3.d[0] = 0x00000000;
    g_cpu->m_s4.d[1] = 0x00000001; g_cpu->m_s4.d[0] = 0x00000000;
    g_cpu->m_s5.d[1] = 0x00000000; g_cpu->m_s5.d[0] = 0xffffffff;
    g_cpu->m_t2.d[1] = 0xcccccccc; g_cpu->m_t2.d[0] = 0xcccccccc;
    g_cpu->m_t3.d[1] = 0xcccccccc; g_cpu->m_t3.d[0] = 0xcccccccc;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00005555);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x80000000 && g_cpu->m_t2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x7fffffff && g_cpu->m_t3.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0xfffffff0);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000001 && g_cpu->m_t5.d[0] == 0x0000000f);

    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0x00000001 && g_cpu->m_s5.d[0] == 0x0000000f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// beql ノーマル＆遅延スロット
BEGIN_TESTF(r5900_misc_beql_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[0] = 0x12345678; g_cpu->m_s0.d[1] = 0x51243578;
    g_cpu->m_s1.d[0] = 0x77777777; g_cpu->m_s1.d[1] = 0x51243578;
    g_cpu->m_s2.d[0] = 0x12345678; g_cpu->m_s2.d[1] = 0x51243578;
    g_cpu->m_s3.d[0] = 0x12345678; g_cpu->m_s3.d[1] = 0x77777777;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bnel ノーマル＆遅延スロット
BEGIN_TESTF(r5900_misc_bnel_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0x00000000);
    g_cpu->m_s0.d[0] = 0x12345678; g_cpu->m_s0.d[1] = 0x51243578;
    g_cpu->m_s1.d[0] = 0x77777777; g_cpu->m_s1.d[1] = 0x51243578;
    g_cpu->m_s2.d[0] = 0x12345678; g_cpu->m_s2.d[1] = 0x51243578;
    g_cpu->m_s3.d[0] = 0x12345678; g_cpu->m_s3.d[1] = 0x77777777;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[0] == 1);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[0] == 0);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[0] == 1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sb ノーマル
BEGIN_TESTF(r5900_misc_sb_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x20);
    g_cpu->m_a0.d[0] = 0x12123455;
    g_cpu->m_a1.d[0] = 0x22ccffdd;
    g_cpu->m_a2.d[0] = 0x220001ff;
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->execute();

    u8 expect[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xdd,
      0xff, 0x55, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x20) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sb アドレス変換
BEGIN_TESTF(r5900_misc_sb_002, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x20);
    g_cpu->m_a0.d[0] = 0x12123455;
    g_cpu->m_a1.d[0] = 0x22ccffdd;
    g_cpu->m_a2.d[0] = 0x220001ff;
    g_cpu->m_v0.d[0] = 0x20001010;
    g_cpu->execute();

    u8 expect[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xdd,
      0xff, 0x55, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x20) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sh ノーマル
BEGIN_TESTF(r5900_misc_sh_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x20);
    g_cpu->m_a0.d[0] = 0x12345678;
    g_cpu->m_a1.d[0] = 0x22ccffdd;
    g_cpu->m_a2.d[0] = 0x220001ff;
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->execute();

    u8 expect[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xdd, 0xff,
      0xff, 0x01, 0x78, 0x56, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x20) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sw ノーマル
BEGIN_TESTF(r5900_misc_sw_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x20);
    g_cpu->m_a0.d[0] = 0x12345678;
    g_cpu->m_a1.d[0] = 0x22ccffdd;
    g_cpu->m_a2.d[0] = 0x320001ff;
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->execute();

    u8 expect[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xdd, 0xff, 0xcc, 0x22,
      0xff, 0x01, 0x00, 0x32, 0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x20) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// swl ノーマル＆アドレス末尾0-4
BEGIN_TESTF(r5900_misc_swl_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x40);
    g_cpu->m_v0.d[0] = 0x00001000;
    g_cpu->m_a0.d[0] = 0x12345678;
    g_cpu->execute();

    u8 expect[0x40] = {
      0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x40) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// swr ノーマル＆アドレス末尾0-4
BEGIN_TESTF(r5900_misc_swr_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x40);
    g_cpu->m_v0.d[0] = 0x00001000;
    g_cpu->m_a0.d[0] = 0x12345678;
    g_cpu->execute();

    u8 expect[0x40] = {
      0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0x78, 0x56, 0x34, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0x78, 0x56, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xcc, 0x78, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x40) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sd ノーマル
BEGIN_TESTF(r5900_misc_sd_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x40);
    g_cpu->m_a0.d[1] = 0x11112222; g_cpu->m_a0.d[0] = 0x33334444;
    g_cpu->m_a1.d[1] = 0x55556666; g_cpu->m_a1.d[0] = 0x77778888;
    g_cpu->m_a2.d[1] = 0x9999aaaa; g_cpu->m_a2.d[0] = 0xbbbbeeee;
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->execute();
    
    u8 expect[0x40] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x88, 0x88, 0x77, 0x77, 0x66, 0x66, 0x55, 0x55,
      0xee, 0xee, 0xbb, 0xbb, 0xaa, 0xaa, 0x99, 0x99, 0x44, 0x44, 0x33, 0x33, 0x22, 0x22, 0x11, 0x11,
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x40) == 0);

  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sdl ノーマル＆アドレス末尾0,2,4,7
BEGIN_TESTF(r5900_misc_sdl_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x50);
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_a0.d[1] = 0x12345678; g_cpu->m_a0.d[0] = 0x9abcdef0;
    g_cpu->execute();

    u8 expect[0x50] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0x9a, 0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x50) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sdr ノーマル＆アドレス末尾0,2,4,7
BEGIN_TESTF(r5900_misc_sdr_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x50);
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_a0.d[1] = 0x12345678; g_cpu->m_a0.d[0] = 0x9abcdef0;
    g_cpu->execute();

    u8 expect[0x50] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xf0, 0xde, 0xbc, 0x9a, 0x78, 0x56, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xcc, 0xcc, 0xf0, 0xde, 0xbc, 0x9a, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xf0, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
      
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x50) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sq ノーマル
BEGIN_TESTF(r5900_misc_sq_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x60);
    g_cpu->m_a0.d[3] = 0x33333333; g_cpu->m_a0.d[2] = 0x22222222; g_cpu->m_a0.d[1] = 0x11111111; g_cpu->m_a0.d[0] = 0x00000000;
    g_cpu->m_a1.d[3] = 0x77777777; g_cpu->m_a1.d[2] = 0x66666666; g_cpu->m_a1.d[1] = 0x55555555; g_cpu->m_a1.d[0] = 0x44444444;
    g_cpu->m_a2.d[3] = 0xbbbbbbbb; g_cpu->m_a2.d[2] = 0xaaaaaaaa; g_cpu->m_a2.d[1] = 0x99999999; g_cpu->m_a2.d[0] = 0x88888888;
    g_cpu->m_a3.d[3] = 0x11112222; g_cpu->m_a3.d[2] = 0x33334444; g_cpu->m_a3.d[1] = 0x55556666; g_cpu->m_a3.d[0] = 0x77778888;
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00001020;
    g_cpu->execute();
    
    u32 expect[] = {
      0xcccccccc, 0xcccccccc, 0xcccccccc, 0xcccccccc,
      0x44444444, 0x55555555, 0x66666666, 0x77777777,
      0x88888888, 0x99999999, 0xaaaaaaaa, 0xbbbbbbbb,
      0x00000000, 0x11111111, 0x22222222, 0x33333333,
      0x77778888, 0x55556666, 0x33334444, 0x11112222,
      0xcccccccc, 0xcccccccc, 0xcccccccc, 0xcccccccc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x60) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lb ノーマル
BEGIN_TESTF(r5900_misc_lb_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001011;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xdd,
      0xff, 0x55, 0x22, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000055);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xffffffdd);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000022);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000055);

    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x00000055);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lbu ノーマル
BEGIN_TESTF(r5900_misc_lbu_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001011;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xdd,
      0xff, 0x55, 0x22, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000055);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x000000dd);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x000000ff);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000022);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x000000ff);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000055);

    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x00000055);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lh lhu ノーマル
BEGIN_TESTF(r5900_misc_lh_lhu_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001014;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x87, 0x54, 0x11, 0xff,
      0x78, 0x56, 0x34, 0x82, 0xab, 0xcd, 0xef, 0x01, 0x11, 0x22, 0x33, 0x44, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0xffffffff && g_cpu->m_a0.d[0] == 0xffff8234);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xffffff11);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x00005678);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x000001ef);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xffff8234);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0xffffcdab);

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00008234);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x0000ff11);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00005678);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x000001ef);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00008234);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x0000cdab);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == 0x00000000 && g_cpu->m_v0.d[0] == 0x00005678);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x0000cdab);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lw lwu ノーマル
BEGIN_TESTF(r5900_misc_lw_lwu_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001014;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x87, 0x54, 0x11, 0xff,
      0x78, 0x56, 0x34, 0x82, 0xab, 0xcd, 0xef, 0x01, 0x11, 0x22, 0x33, 0x44, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x01efcdab);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xff115487);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x44332211);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x01efcdab);

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x01efcdab);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0xff115487);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x44332211);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x01efcdab);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == 0xffffffff && g_cpu->m_v0.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x01efcdab);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ld ノーマル
BEGIN_TESTF(r5900_misc_ld_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001014;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x87, 0x54, 0x11, 0xff,
      0x78, 0x56, 0x34, 0x82, 0xab, 0xcd, 0xef, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x44332211 && g_cpu->m_t0.d[0] == 0x01efcdab);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x82345678 && g_cpu->m_t1.d[0] == 0xff115487);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x01efcdab && g_cpu->m_t2.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccc6655 && g_cpu->m_t3.d[0] == 0x44332211);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x01efcdab && g_cpu->m_t4.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x44332211 && g_cpu->m_t5.d[0] == 0x01efcdab);

    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x44332211 && g_cpu->m_v1.d[0] == 0x01efcdab);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lq ノーマル
BEGIN_TESTF(r5900_misc_lq_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001008;

    u8 data[0x30] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x87, 0x54, 0x11, 0xff,
      0x78, 0x56, 0x34, 0x82, 0xab, 0xcd, 0xef, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xcc, 0xcc,
      0x44, 0x33, 0x22, 0x11, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x44, 0x11,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x30);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0xcccc6655 && g_cpu->m_t0.d[2] == 0x44332211 && g_cpu->m_t0.d[1] == 0x01efcdab && g_cpu->m_t0.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[3] == 0x1144ffee && g_cpu->m_t1.d[2] == 0xddccbbaa && g_cpu->m_t1.d[1] == 0x99887766 && g_cpu->m_t1.d[0] == 0x11223344);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[3] == 0xcccc6655 && g_cpu->m_t2.d[2] == 0x44332211 && g_cpu->m_t2.d[1] == 0x01efcdab && g_cpu->m_t2.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[3] == 0xcccc6655 && g_cpu->m_t3.d[2] == 0x44332211 && g_cpu->m_t3.d[1] == 0x01efcdab && g_cpu->m_t3.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[3] == 0x1144ffee && g_cpu->m_t4.d[2] == 0xddccbbaa && g_cpu->m_t4.d[1] == 0x99887766 && g_cpu->m_t4.d[0] == 0x11223344);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[3] == 0xcccc6655 && g_cpu->m_t5.d[2] == 0x44332211 && g_cpu->m_t5.d[1] == 0x01efcdab && g_cpu->m_t5.d[0] == 0x82345678);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[3] == 0xcccc6655 && g_cpu->m_v0.d[2] == 0x44332211 && g_cpu->m_v0.d[1] == 0x01efcdab && g_cpu->m_v0.d[0] == 0x82345678);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lwl & lwr ノーマル＆アドレス末尾0,1,2,3
BEGIN_TESTF(r5900_misc_lwl_lwr_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[1] = 0x00000000; g_cpu->m_v1.d[0] = 0x20001011;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xee, 0xdd, 0xaa, 0xbb, 0x87, 0x54, 0x11, 0xff,
      0x78, 0xf6, 0x34, 0x82, 0xab, 0xcd, 0xef, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x78cccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0xf678cccc);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x34f678cc);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xffffffff && g_cpu->m_t3.d[0] == 0x8234f678);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0xffffffff && g_cpu->m_s0.d[0] == 0x8234f678);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xcccccccc && g_cpu->m_s1.d[0] == 0xcc8234f6);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xcccccccc && g_cpu->m_s2.d[0] == 0xcccc8234);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xcccccccc && g_cpu->m_s3.d[0] == 0xcccccc82);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x01efcdab);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == 0xffffffff && g_cpu->m_v0.d[0] == 0xf6781010);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x20001082);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ldl & ldr ノーマル＆アドレス末尾0,2,4,7
BEGIN_TESTF(r5900_misc_ldl_ldr_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[1] = 0x00000000; g_cpu->m_v1.d[0] = 0x20001011;
    
    u8 data[0x30] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xee, 0xdd, 0xaa, 0xbb, 0x87, 0x54, 0x11, 0xff,
      0x78, 0x56, 0x34, 0x82, 0xab, 0xcd, 0xef, 0x01, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0xcc, 0xcc,
      0x44, 0x33, 0x22, 0x11, 0x66, 0x77, 0x88, 0x99, 0xaa, 0xbb, 0xcc, 0xdd, 0xee, 0xff, 0x44, 0x11,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x30);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x78cccccc && g_cpu->m_t0.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x345678cc && g_cpu->m_t1.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x01efcdab && g_cpu->m_t2.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x115487bb && g_cpu->m_t3.d[0] == 0xaaddeecc);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x01efcdab && g_cpu->m_s0.d[0] == 0x82345678);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xcccc01ef && g_cpu->m_s1.d[0] == 0xcdab8234);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xcccccccc && g_cpu->m_s2.d[0] == 0xcccccc01);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xcccccccc && g_cpu->m_s3.d[0] == 0xccccff11);

    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == 0x78000000 && g_cpu->m_v0.d[0] == 0x00001010);
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == 0x00000000 && g_cpu->m_v1.d[0] == 0x2000ff11);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// lwc1 ノーマル
BEGIN_TESTF(r5900_misc_lwc1_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->m_v1.d[0] = 0x20001014;

    u8 data[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, 0x00, 0x00, 0x00,
      0x00, 0x00, 0x80, 0x3f, 0x11, 0x22, 0x33, 0x44, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    memcpy((void*)VA2PA(0x1000), data, 0x20);

    g_cpu->execute();

    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fpr[0]) == 0x44332211);
    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fpr[1]) == 0x00000000);
    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fpr[2]) == 0x3f800000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// swc1 ノーマル
BEGIN_TESTF(r5900_misc_swc1_001, r5900_misc_fix_01){
  try
  {
    OPEN_ELF("misc");
    INIT_GPR(0xcccccccc);
    memset((void*)VA2PA(0x1000), 0xcc, 0x20);
    *((u32*)&g_cpu->m_fpu.fpr[0]) = 0x12345678;
    *((u32*)&g_cpu->m_fpu.fpr[1]) = 0x3f800000;
    *((u32*)&g_cpu->m_fpu.fpr[2]) = 0xffffffff;
    g_cpu->m_v0.d[0] = 0x00001010;
    g_cpu->execute();

    u8 expect[0x20] = {
      0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0x00, 0x00, 0x80, 0x3f,
      0xff, 0xff, 0xff, 0xff, 0x78, 0x56, 0x34, 0x12, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc,
    };
    WIN_ASSERT_TRUE(memcmp((void*)VA2PA(0x1000), expect, 0x20) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
