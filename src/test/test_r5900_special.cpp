#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_special_fix_01);

SETUP(r5900_special_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_special_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// jr jalr ノーマル
BEGIN_TESTF(r5900_special_jr_jalr_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0x00000002);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0x00000004);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0x00000008);
    WIN_ASSERT_TRUE(g_cpu->m_at.d[1] == g_cpu->m_ra.d[1] && g_cpu->m_at.d[0] == g_cpu->m_ra.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sll srl sra ノーマル
BEGIN_TESTF(r5900_special_sll_srl_sra_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[1] = 0x00000000; g_cpu->m_a0.d[0] = 0x92348765;
    g_cpu->m_a1.d[1] = 0x00000000; g_cpu->m_a1.d[0] = 0x12348765;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0xffffffff && g_cpu->m_t0.d[0] == 0x92348765);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x24690eca);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x87650000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xffffffff && g_cpu->m_t3.d[0] == 0x80000000);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0xffffffff && g_cpu->m_s0.d[0] == 0x92348765);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x491a43b2);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00009234);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0xffffffff && g_cpu->m_s4.d[0] == 0x92348765);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0xffffffff && g_cpu->m_s5.d[0] == 0xc91a43b2);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0x00000000 && g_cpu->m_s6.d[0] == 0x12348765);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[1] == 0x00000000 && g_cpu->m_s7.d[0] == 0x091a43b2);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sllv srlv srav ノーマル
BEGIN_TESTF(r5900_special_sllv_srlv_srav_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x92348765;
    g_cpu->m_v1.d[1] = 0x00000000; g_cpu->m_v1.d[0] = 0x12348765;

    g_cpu->m_a0.d[0] = 0xff000000;
    g_cpu->m_a1.d[0] = 0xff000001;
    g_cpu->m_a2.d[0] = 0xffff0010;
    g_cpu->m_a3.d[0] = 0xffff001f;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0xffffffff && g_cpu->m_t0.d[0] == 0x92348765);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x24690eca);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x87650000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xffffffff && g_cpu->m_t3.d[0] == 0x80000000);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0xffffffff && g_cpu->m_s0.d[0] == 0x92348765);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x491a43b2);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00009234);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0xffffffff && g_cpu->m_s4.d[0] == 0x92348765);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0xffffffff && g_cpu->m_s5.d[0] == 0xc91a43b2);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0x00000000 && g_cpu->m_s6.d[0] == 0x12348765);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[1] == 0x00000000 && g_cpu->m_s7.d[0] == 0x091a43b2);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// movz movn ノーマル
BEGIN_TESTF(r5900_special_movz_movn_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x12345678; g_cpu->m_s0.d[0] = 0x9abcdef0;
    g_cpu->m_s1.d[1] = 0x56789abc; g_cpu->m_s1.d[0] = 0xdef01234;
    g_cpu->m_s2.d[1] = 0xaaaabbbb; g_cpu->m_s2.d[0] = 0xccccdddd;
    g_cpu->m_s3.d[1] = 0x11112222; g_cpu->m_s3.d[0] = 0x33334444;

    g_cpu->m_t0.d[1] = 0x00000000; g_cpu->m_t0.d[0] = 0x00000000;
    g_cpu->m_t1.d[1] = 0x00001000; g_cpu->m_t1.d[0] = 0x00000000;
    g_cpu->m_t2.d[1] = 0x00000000; g_cpu->m_t2.d[0] = 0x00001000;
    g_cpu->m_t3.d[1] = 0x00001000; g_cpu->m_t3.d[0] = 0x00001000;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x12345678 && g_cpu->m_a0.d[0] == 0x9abcdef0);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xcccccccc && g_cpu->m_a1.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xcccccccc && g_cpu->m_a2.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0xcccccccc && g_cpu->m_a3.d[0] == 0xcccccccc);

    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0xcccccccc && g_cpu->m_t4.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x56789abc && g_cpu->m_t5.d[0] == 0xdef01234);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0xaaaabbbb && g_cpu->m_t6.d[0] == 0xccccdddd);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x11112222 && g_cpu->m_t7.d[0] == 0x33334444);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mflo mfhi mtlo mthi ノーマル
BEGIN_TESTF(r5900_special_mflo_mfhi_mtlo_mthi_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    g_cpu->m_a0.d[1] = 0x12345678; g_cpu->m_a0.d[0] = 0x9abcdef0;
    g_cpu->m_a1.d[1] = 0x56789abc; g_cpu->m_a1.d[0] = 0xdef01234;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x12345678 && g_cpu->m_s0.d[0] == 0x9abcdef0);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x56789abc && g_cpu->m_s1.d[0] == 0xdef01234);

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[1] == g_cpu->m_s0.d[1] && g_cpu->m_hi.d[0] == g_cpu->m_s0.d[0]);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[1] == g_cpu->m_s1.d[1] && g_cpu->m_lo.d[0] == g_cpu->m_s1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mult ノーマル
BEGIN_TESTF(r5900_special_mult_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00000020; g_cpu->m_t0.d[0] = 0x00000045;
    g_cpu->m_a1.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000002;
    g_cpu->m_a2.d[0] = 0x7fffffff; g_cpu->m_t2.d[0] = 0x7fffffff;
    g_cpu->m_a3.d[0] = 0x80000000; g_cpu->m_t3.d[0] = 0x7fffffff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x000008a0);  // lo
    
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xffffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xfffffffe);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x3fffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000001);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == g_cpu->m_s2.d[1] && g_cpu->m_v0.d[0] == g_cpu->m_s2.d[0]);

    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0xffffffff && g_cpu->m_a3.d[0] == 0xc0000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x80000000);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == g_cpu->m_s3.d[1] && g_cpu->m_v1.d[0] == g_cpu->m_s3.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// multu ノーマル
BEGIN_TESTF(r5900_special_multu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00000020; g_cpu->m_t0.d[0] = 0x00000045;
    g_cpu->m_a1.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000002;
    g_cpu->m_a2.d[0] = 0x7fffffff; g_cpu->m_t2.d[0] = 0x7fffffff;
    g_cpu->m_a3.d[0] = 0x80000000; g_cpu->m_t3.d[0] = 0x7fffffff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x000008a0);  // lo
    
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000001);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xfffffffe);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x3fffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000001);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[1] == g_cpu->m_s2.d[1] && g_cpu->m_v0.d[0] == g_cpu->m_s2.d[0]);

    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x3fffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x80000000);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[1] == g_cpu->m_s3.d[1] && g_cpu->m_v1.d[0] == g_cpu->m_s3.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// div ノーマル
BEGIN_TESTF(r5900_special_div_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[0] = 0x00000045; g_cpu->m_s0.d[0] = 0x00000020;  // 被除数が正・除数が正　＞　商が正・余が正
    g_cpu->m_t1.d[0] = 0xfffffffd; g_cpu->m_s1.d[0] = 0x00000002;  // 被除数が負・除数が正　＞　商が負・余が負
    g_cpu->m_t2.d[0] = 0xfffffffd; g_cpu->m_s2.d[0] = 0xfffffffe;  // 被除数が負・除数が負　＞　商が正・余が負
    g_cpu->m_t3.d[0] = 0x00000003; g_cpu->m_s3.d[0] = 0xfffffffe;  // 被除数が正・除数が負　＞　商が負・余が正

    g_cpu->m_t4.d[0] = 0x00001234; g_cpu->m_s4.d[0] = 0x00000000;  // 0除算

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000005);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000002);  // lo 商
    
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xffffffff);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0xffffffff);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0xffffffff);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000001);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000001);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xffffffff && g_cpu->m_t3.d[0] == 0xffffffff);  // lo 商
    
    // 0除算はhi, loが変化しないので前回の演算結果と同じものが格納される。念のためチェック
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x00000001);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0xffffffff && g_cpu->m_t4.d[0] == 0xffffffff);  // lo 商
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// divu ノーマル
BEGIN_TESTF(r5900_special_divu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[0] = 0x00000045; g_cpu->m_s0.d[0] = 0x00000020;  // 被除数が正・除数が正
    g_cpu->m_t1.d[0] = 0xfffffffd; g_cpu->m_s1.d[0] = 0x00000002;  // 被除数が負・除数が正
    g_cpu->m_t2.d[0] = 0xfffffffd; g_cpu->m_s2.d[0] = 0xfffffffe;  // 被除数が負・除数が負
    g_cpu->m_t3.d[0] = 0x00000003; g_cpu->m_s3.d[0] = 0xfffffffe;  // 被除数が正・除数が負

    g_cpu->m_t4.d[0] = 0x00001234; g_cpu->m_s4.d[0] = 0x00000000;  // 0除算
    g_cpu->m_t5.d[0] = 0x40000000; g_cpu->m_s5.d[0] = 0x00000001;  // 1除算 商が正
    g_cpu->m_t6.d[0] = 0x80000000; g_cpu->m_s6.d[0] = 0x00000001;  // 1除算 商が負

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000005);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000002);  // lo 商
    
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x00000001);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x7ffffffe);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0xfffffffd);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000000);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000003);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000000);  // lo 商
    
    // 0除算はhi, loが変化しないので前回の演算結果と同じものが格納される。念のためチェック
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x00000003);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00000000);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0x00000000 && g_cpu->m_s5.d[0] == 0x00000000);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x40000000);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0x00000000 && g_cpu->m_s6.d[0] == 0x00000000);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0xffffffff && g_cpu->m_t6.d[0] == 0x80000000);  // lo 商
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// add ノーマル＆オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_special_add_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0x00000222;  // 結果正 (ゼロ拡張)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0x00000002;  // +側オーバーフロー
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x80000000;   // 結果負 (符号拡張)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0xfffffffe;  // -側オーバーフロー

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xcccccccc && g_cpu->m_t1.d[0] == 0xcccccccc);  // 不変
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);  // 不変

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00007ffe);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// addu ノーマル＆オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_special_addu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0x00000222;  // 結果正 (ゼロ拡張)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0x00000002;  // +側オーバーフローしない
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x80000000;   // 結果負 (符号拡張)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0xfffffffe;  // -側オーバーフローしない

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x7fffffff);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00007ffe);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sub ノーマル＆オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_special_sub_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0xfffffdde;  // 結果正 (ゼロ拡張)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xfffffffe;  // +側オーバーフロー
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x7fffffff;   // 結果負 (符号拡張)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0x00000002;  // -側オーバーフロー

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xcccccccc && g_cpu->m_t1.d[0] == 0xcccccccc);  // 不変
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);  // 不変

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00003ffd);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// subu ノーマル＆オーバーフロー＆符号拡張
BEGIN_TESTF(r5900_special_subu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0xfffffdde;  // 結果正 (ゼロ拡張)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xfffffffe;  // +側オーバーフロー
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x7fffffff;   // 結果負 (符号拡張)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0x00000002;  // -側オーバーフロー

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x7fffffff);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00003ffd);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// and or xor nor ノーマル
BEGIN_TESTF(r5900_special_and_or_xor_nor_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x0000ffff; g_cpu->m_v0.d[0] = 0xff00ff00;
    g_cpu->m_v1.d[1] = 0x00000000; g_cpu->m_v1.d[0] = 0x00000000;
    g_cpu->m_s0.d[1] = 0x12345678; g_cpu->m_s0.d[0] = 0x12345678;

    g_cpu->m_a0.d[1] = 0x0000ffff; g_cpu->m_a0.d[0] = 0xff00ff00;
    g_cpu->m_a1.d[1] = 0x0000ffff; g_cpu->m_a1.d[0] = 0xff00ff00;
    g_cpu->m_a2.d[1] = 0x0000ffff; g_cpu->m_a2.d[0] = 0xff00ff00;
    g_cpu->m_a3.d[1] = 0x0000ffff; g_cpu->m_a3.d[0] = 0xff00ff00;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00005678 && g_cpu->m_t0.d[0] == 0x12005600);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x1234ffff && g_cpu->m_t2.d[0] == 0xff34ff78);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x12345678 && g_cpu->m_t3.d[0] == 0x12345678);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x1234a987 && g_cpu->m_t4.d[0] == 0xed34a978);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x12345678 && g_cpu->m_t5.d[0] == 0x12345678);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0xedcb0000 && g_cpu->m_t6.d[0] == 0x00cb0087);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0xedcba987 && g_cpu->m_t7.d[0] == 0xedcba987);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00005678 && g_cpu->m_a0.d[0] == 0x12005600);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x1234ffff && g_cpu->m_a1.d[0] == 0xff34ff78);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x1234a987 && g_cpu->m_a2.d[0] == 0xed34a978);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0xedcb0000 && g_cpu->m_a3.d[0] == 0x00cb0087);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// slt ノーマル＆オーバーフロー
BEGIN_TESTF(r5900_special_slt_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[1] = 0x00000000; g_cpu->m_t0.d[0] = 0x00000000;
    g_cpu->m_t1.d[1] = 0x00000000; g_cpu->m_t1.d[0] = 0x00000001;
    g_cpu->m_t2.d[1] = 0xffffffff; g_cpu->m_t2.d[0] = 0xffffffff;
    g_cpu->m_t3.d[1] = 0x7fffffff; g_cpu->m_t3.d[0] = 0x00000000;  // 上位WORDオーバーフロー+
    g_cpu->m_t4.d[1] = 0x80000000; g_cpu->m_t4.d[0] = 0x00000000;  // 上位WORDオーバーフロー-
    g_cpu->m_t5.d[1] = 0x00000000; g_cpu->m_t5.d[0] = 0x7fffffff;  // 下位WORDオーバーフロー+
    g_cpu->m_t6.d[1] = 0x00000000; g_cpu->m_t6.d[0] = 0x80000000;  // 下位WORDオーバーフロー-
    g_cpu->m_t7.d[1] = 0x00000001; g_cpu->m_t7.d[0] = 0x00000000;  // 上位WORDでの大小比較

    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00000001;
    g_cpu->m_v1.d[1] = 0xffffffff; g_cpu->m_v1.d[0] = 0xffffffff;

    g_cpu->m_k0.d[1] = 0x00000000; g_cpu->m_k0.d[0] = 0x00000000;
    g_cpu->m_k1.d[1] = 0x00000002; g_cpu->m_k1.d[0] = 0x00000000;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0x00000000 && g_cpu->m_s5.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0x00000000 && g_cpu->m_s6.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[1] == 0x00000000 && g_cpu->m_s7.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sltu ノーマル＆オーバーフロー
BEGIN_TESTF(r5900_special_sltu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[1] = 0x00000000; g_cpu->m_t0.d[0] = 0x00000000;
    g_cpu->m_t1.d[1] = 0x00000000; g_cpu->m_t1.d[0] = 0x00000001;
    g_cpu->m_t2.d[1] = 0xffffffff; g_cpu->m_t2.d[0] = 0xffffffff;
    g_cpu->m_t3.d[1] = 0x7fffffff; g_cpu->m_t3.d[0] = 0x00000000;  // 上位WORDオーバーフロー+
    g_cpu->m_t4.d[1] = 0x80000000; g_cpu->m_t4.d[0] = 0x00000000;  // 上位WORDオーバーフロー-
    g_cpu->m_t5.d[1] = 0x00000000; g_cpu->m_t5.d[0] = 0x7fffffff;  // 下位WORDオーバーフロー+
    g_cpu->m_t6.d[1] = 0x00000000; g_cpu->m_t6.d[0] = 0x80000000;  // 下位WORDオーバーフロー-
    g_cpu->m_t7.d[1] = 0x00000001; g_cpu->m_t7.d[0] = 0x00000000;  // 上位WORDでの大小比較

    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00000001;
    g_cpu->m_v1.d[1] = 0xffffffff; g_cpu->m_v1.d[0] = 0xffffffff;

    g_cpu->m_k0.d[1] = 0x00000000; g_cpu->m_k0.d[0] = 0x00000000;
    g_cpu->m_k1.d[1] = 0x00000002; g_cpu->m_k1.d[0] = 0x00000000;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x00000001);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0x00000000 && g_cpu->m_s5.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0x00000000 && g_cpu->m_s6.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[1] == 0x00000000 && g_cpu->m_s7.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000001);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dadd daddu
BEGIN_TESTF(r5900_special_dadd_daddu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x00123456; g_cpu->m_s0.d[0] = 0x789abcde;
    g_cpu->m_s1.d[1] = 0xffffff11; g_cpu->m_s1.d[0] = 0xffffffff;
    g_cpu->m_s2.d[1] = 0x7fffffff; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_s3.d[1] = 0x80000000; g_cpu->m_s3.d[0] = 0x00000000;
    g_cpu->m_s4.d[1] = 0x00000001; g_cpu->m_s4.d[0] = 0x00000000;
    g_cpu->m_s5.d[1] = 0x00000000; g_cpu->m_s5.d[0] = 0xffffffff;
    
    g_cpu->m_a0.d[1] = 0x00edcba9; g_cpu->m_a0.d[0] = 0x87654321;
    g_cpu->m_a1.d[1] = 0x00000000; g_cpu->m_a1.d[0] = 0xffffffff;
    g_cpu->m_a2.d[1] = 0x00000001; g_cpu->m_a2.d[0] = 0x00000000;
    g_cpu->m_a3.d[1] = 0xfffffffe; g_cpu->m_a3.d[0] = 0x00000000;
    g_cpu->m_v0.d[1] = 0xffffffff; g_cpu->m_v0.d[0] = 0xffffffff;
    g_cpu->m_v1.d[1] = 0x00000000; g_cpu->m_v1.d[0] = 0x00000001;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00ffffff && g_cpu->m_t0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffff12 && g_cpu->m_t1.d[0] == 0xfffffffe);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xcccccccc && g_cpu->m_t2.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000001 && g_cpu->m_t5.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00ffffff && g_cpu->m_t6.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0xffffff12 && g_cpu->m_t7.d[0] == 0xfffffffe);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[1] == 0x80000000 && g_cpu->m_t8.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t9.d[1] == 0x7ffffffe && g_cpu->m_t9.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_k0.d[1] == 0x00000000 && g_cpu->m_k0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_k1.d[1] == 0x00000001 && g_cpu->m_k1.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsub dsubu
BEGIN_TESTF(r5900_special_dsub_dsubu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[1] = 0x01000000; g_cpu->m_s0.d[0] = 0xffffffff;
    g_cpu->m_s1.d[1] = 0x01000000; g_cpu->m_s1.d[0] = 0x00000000;
    g_cpu->m_s2.d[1] = 0x7fffffff; g_cpu->m_s2.d[0] = 0xffffffff;
    g_cpu->m_s3.d[1] = 0x80000000; g_cpu->m_s3.d[0] = 0x00000000;
    g_cpu->m_s4.d[1] = 0x00000001; g_cpu->m_s4.d[0] = 0x00000000;
    g_cpu->m_s5.d[1] = 0x00000000; g_cpu->m_s5.d[0] = 0xffffffff;
    
    g_cpu->m_a0.d[1] = 0x00ffffff; g_cpu->m_a0.d[0] = 0x00000000;
    g_cpu->m_a1.d[1] = 0x02000000; g_cpu->m_a1.d[0] = 0x00000000;
    g_cpu->m_a2.d[1] = 0xffffffff; g_cpu->m_a2.d[0] = 0xffffffff;
    g_cpu->m_a3.d[1] = 0x00000000; g_cpu->m_a3.d[0] = 0x00000001;
    g_cpu->m_v0.d[1] = 0x00000000; g_cpu->m_v0.d[0] = 0x00000001;
    g_cpu->m_v1.d[1] = 0xffffffff; g_cpu->m_v1.d[0] = 0xffffffff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000001 && g_cpu->m_t0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xff000000 && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xcccccccc && g_cpu->m_t2.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000001 && g_cpu->m_t5.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000001 && g_cpu->m_t6.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0xff000000 && g_cpu->m_t7.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[1] == 0x80000000 && g_cpu->m_t8.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t9.d[1] == 0x7fffffff && g_cpu->m_t9.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_k0.d[1] == 0x00000000 && g_cpu->m_k0.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_k1.d[1] == 0x00000001 && g_cpu->m_k1.d[0] == 0x00000000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsll ノーマル
BEGIN_TESTF(r5900_special_dsll_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[1] = 0x01234567; g_cpu->m_a0.d[0] = 0x89abcdef;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x01234567 && g_cpu->m_t0.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x02468acf && g_cpu->m_t1.d[0] == 0x13579bde);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xc4d5e6f7 && g_cpu->m_t2.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x456789ab && g_cpu->m_t3.d[0] == 0xcdef0000);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x02468acf && g_cpu->m_a0.d[0] == 0x13579bde);  // srcとdstが同じケース
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsll32 ノーマル
BEGIN_TESTF(r5900_special_dsll32_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[1] = 0x01234567; g_cpu->m_a0.d[0] = 0x89abcdef;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x89abcdef && g_cpu->m_t0.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x13579bde && g_cpu->m_t1.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x80000000 && g_cpu->m_t2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcdef0000 && g_cpu->m_t3.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x13579bde && g_cpu->m_a0.d[0] == 0x00000000);  // srcとdstが同じケース
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsrl ノーマル
BEGIN_TESTF(r5900_special_dsrl_001, r5900_special_fix_01){
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[1] = 0x01234567; g_cpu->m_a0.d[0] = 0x89abcdef;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x01234567 && g_cpu->m_t0.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x0091a2b3 && g_cpu->m_t1.d[0] == 0xc4d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x02468acf);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000123 && g_cpu->m_t3.d[0] == 0x456789ab);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x0091a2b3 && g_cpu->m_a0.d[0] == 0xc4d5e6f7);  // srcとdstが同じケース
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsrl32 ノーマル
BEGIN_TESTF(r5900_special_dsrl32_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[1] = 0x01234567; g_cpu->m_a0.d[0] = 0x89abcdef;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x01234567);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x0091a2b3);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000123);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x0091a2b3);  // srcとdstが同じケース
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsra dsra32 ノーマル
BEGIN_TESTF(r5900_special_dsra_dsra32_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[1] = 0x01234567; g_cpu->m_a0.d[0] = 0x89abcdef;
    g_cpu->m_a1.d[1] = 0x81234567; g_cpu->m_a1.d[0] = 0x89abcdef;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x01234567 && g_cpu->m_t0.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x0091a2b3 && g_cpu->m_t1.d[0] == 0xc4d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x02468acf);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000123 && g_cpu->m_t3.d[0] == 0x456789ab);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x81234567 && g_cpu->m_t4.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0xc091a2b3 && g_cpu->m_t5.d[0] == 0xc4d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0xffffffff && g_cpu->m_t6.d[0] == 0x02468acf);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0xffff8123 && g_cpu->m_t7.d[0] == 0x456789ab);

    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x01234567);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x0091a2b3);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000123);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0xffffffff && g_cpu->m_s4.d[0] == 0x81234567);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0xffffffff && g_cpu->m_s5.d[0] == 0xc091a2b3);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0xffffffff && g_cpu->m_s6.d[0] == 0xffffffff);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[1] == 0xffffffff && g_cpu->m_s7.d[0] == 0xffff8123);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x0091a2b3 && g_cpu->m_a0.d[0] == 0xc4d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xc091a2b3);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsllv ノーマル
BEGIN_TESTF(r5900_special_dsllv_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x01234567; g_cpu->m_v0.d[0] = 0x89abcdef;

    g_cpu->m_s0.d[0] = 0xff000000;
    g_cpu->m_s1.d[0] = 0xff000001;
    g_cpu->m_s2.d[0] = 0xffff0010;
    g_cpu->m_s3.d[0] = 0xffff001f;
    g_cpu->m_s4.d[0] = 0xff000020;
    g_cpu->m_s5.d[0] = 0xff000021;
    g_cpu->m_s6.d[0] = 0xffff003f;
    g_cpu->m_s7.d[0] = 0xffff0040;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x01234567 && g_cpu->m_t0.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x02468acf && g_cpu->m_t1.d[0] == 0x13579bde);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x456789ab && g_cpu->m_t2.d[0] == 0xcdef0000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xc4d5e6f7 && g_cpu->m_t3.d[0] == 0x80000000);

    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x89abcdef && g_cpu->m_t4.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x13579bde && g_cpu->m_t5.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x80000000 && g_cpu->m_t6.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x01234567 && g_cpu->m_t7.d[0] == 0x89abcdef);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsrlv ノーマル
BEGIN_TESTF(r5900_special_dsrlv_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x01234567; g_cpu->m_v0.d[0] = 0x89abcdef;

    g_cpu->m_s0.d[0] = 0xff000000;
    g_cpu->m_s1.d[0] = 0xff000001;
    g_cpu->m_s2.d[0] = 0xffff0010;
    g_cpu->m_s3.d[0] = 0xffff001f;
    g_cpu->m_s4.d[0] = 0xff000020;
    g_cpu->m_s5.d[0] = 0xff000021;
    g_cpu->m_s6.d[0] = 0xffff003f;
    g_cpu->m_s7.d[0] = 0xffff0040;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x01234567 && g_cpu->m_t0.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x0091a2b3 && g_cpu->m_t1.d[0] == 0xc4d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000123 && g_cpu->m_t2.d[0] == 0x456789ab);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x02468acf);

    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x01234567);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x0091a2b3);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x00000000);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x01234567 && g_cpu->m_t7.d[0] == 0x89abcdef);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsrav ノーマル
BEGIN_TESTF(r5900_special_dsrav_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_v0.d[1] = 0x01234567; g_cpu->m_v0.d[0] = 0x89abcdef;
    g_cpu->m_v1.d[1] = 0x81234567; g_cpu->m_v1.d[0] = 0x09abcdef;

    g_cpu->m_s0.d[0] = 0xff000000;
    g_cpu->m_s1.d[0] = 0xff000001;
    g_cpu->m_s2.d[0] = 0xff000022;
    g_cpu->m_s3.d[0] = 0xffff0040;

    g_cpu->m_s4.d[0] = 0xff000000;
    g_cpu->m_s5.d[0] = 0xff000001;
    g_cpu->m_s6.d[0] = 0xff000022;
    g_cpu->m_s7.d[0] = 0xffff0040;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x01234567 && g_cpu->m_t0.d[0] == 0x89abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x0091a2b3 && g_cpu->m_t1.d[0] == 0xc4d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x0048d159);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x01234567 && g_cpu->m_t3.d[0] == 0x89abcdef);

    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x81234567 && g_cpu->m_t4.d[0] == 0x09abcdef);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0xc091a2b3 && g_cpu->m_t5.d[0] == 0x84d5e6f7);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0xffffffff && g_cpu->m_t6.d[0] == 0xe048d159);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x81234567 && g_cpu->m_t7.d[0] == 0x09abcdef);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
