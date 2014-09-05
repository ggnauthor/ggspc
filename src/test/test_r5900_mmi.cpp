#if defined _UNIT_TEST

#include "test/test_r5900.h"

#if defined DEBUG_NEW
  #define new DEBUG_NEW
#endif // defined DEBUG_NEW

FIXTURE(r5900_mmi_fix_01);

SETUP(r5900_mmi_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_mmi_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// mflo1 mfhi1 mtlo1 mthi1 ノーマル
BEGIN_TESTF(r5900_mmi_mflo1_mfhi1_mtlo1_mthi1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    g_cpu->m_hi.d[1] = 0xffffffff; g_cpu->m_hi.d[0] = 0xffffffff;
    g_cpu->m_lo.d[1] = 0xffffffff; g_cpu->m_lo.d[0] = 0xffffffff;
    g_cpu->m_a0.d[1] = 0x12345678; g_cpu->m_a0.d[0] = 0x9abcdef0;
    g_cpu->m_a1.d[1] = 0x56789abc; g_cpu->m_a1.d[0] = 0xdef01234;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x12345678 && g_cpu->m_s0.d[0] == 0x9abcdef0);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x56789abc && g_cpu->m_s1.d[0] == 0xdef01234);

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == g_cpu->m_s0.d[1] && g_cpu->m_hi.d[2] == g_cpu->m_s0.d[0]);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == g_cpu->m_s1.d[1] && g_cpu->m_lo.d[2] == g_cpu->m_s1.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mult1 ノーマル
BEGIN_TESTF(r5900_mmi_mult1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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

    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == g_cpu->m_hi.d[3] && g_cpu->m_a3.d[0] == g_cpu->m_hi.d[2]);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == g_cpu->m_lo.d[3] && g_cpu->m_s3.d[0] == g_cpu->m_lo.d[2]);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// multu1 ノーマル
BEGIN_TESTF(r5900_mmi_multu1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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

    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == g_cpu->m_hi.d[3] && g_cpu->m_a3.d[0] == g_cpu->m_hi.d[2]);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == g_cpu->m_lo.d[3] && g_cpu->m_s3.d[0] == g_cpu->m_lo.d[2]);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// div1 ノーマル
BEGIN_TESTF(r5900_mmi_div1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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

    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == g_cpu->m_hi.d[3] && g_cpu->m_s4.d[0] == g_cpu->m_hi.d[2]);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == g_cpu->m_lo.d[3] && g_cpu->m_t4.d[0] == g_cpu->m_lo.d[2]);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// divu1 ノーマル
BEGIN_TESTF(r5900_mmi_divu1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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

    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == g_cpu->m_hi.d[3] && g_cpu->m_s6.d[0] == g_cpu->m_hi.d[2]);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == g_cpu->m_lo.d[3] && g_cpu->m_t6.d[0] == g_cpu->m_lo.d[2]);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// madd ノーマル
BEGIN_TESTF(r5900_mmi_madd_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 乗算結果の下位Wordが負数（オーバーフロー）
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 乗算結果が負数
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 下位Wordの加算にてキャリー発生するケース
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 上位Wordの結果が負数
    g_cpu->m_s0.d[0] = 0x00000020; g_cpu->m_s1.d[0] = 0x00000015; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0xfffffffe;
    // (hi|lo)=(s7|s6) + s4 * s5 ２オペランド
    g_cpu->m_s4.d[0] = 0x0000ffff; g_cpu->m_s5.d[0] = 0x0001ffff; g_cpu->m_s6.d[0] = 0x00100000; g_cpu->m_s7.d[0] = 0x00000010;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x00000006);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0x950008a1);  // lo
    
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000001);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0xffffffbc);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x000d0001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0xfffffffe);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x000002a1);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[1] == 0x00000000 && g_cpu->m_hi.d[0] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[1] == 0x00000000 && g_cpu->m_lo.d[0] == 0x000d0001);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// madd1 ノーマル
BEGIN_TESTF(r5900_mmi_madd1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 乗算結果の下位Wordが負数（オーバーフロー）
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 乗算結果が負数
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 下位Wordの加算にてキャリー発生するケース
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 上位Wordの結果が負数
    g_cpu->m_s0.d[0] = 0x00000020; g_cpu->m_s1.d[0] = 0x00000015; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0xfffffffe;
    // (hi|lo)=(s7|s6) + s4 * s5 ２オペランド
    g_cpu->m_s4.d[0] = 0x0000ffff; g_cpu->m_s5.d[0] = 0x0001ffff; g_cpu->m_s6.d[0] = 0x00100000; g_cpu->m_s7.d[0] = 0x00000010;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x00000006);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0x950008a1);  // lo
    
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000001);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0xffffffbc);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x000d0001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0xfffffffe);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x000002a1);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x00000000 && g_cpu->m_hi.d[2] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x00000000 && g_cpu->m_lo.d[2] == 0x000d0001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[1] == 0xcccccccc && g_cpu->m_hi.d[0] == 0xcccccccc);  // hi0
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[1] == 0xcccccccc && g_cpu->m_lo.d[0] == 0xcccccccc);  // lo0
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// maddu ノーマル
BEGIN_TESTF(r5900_mmi_maddu_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 乗算結果の下位Wordが負数（オーバーフロー）
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 乗算結果が負数
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 下位Wordの加算にてキャリー発生するケース
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 上位Wordの結果が負数
    g_cpu->m_s0.d[0] = 0x80000000; g_cpu->m_s1.d[0] = 0x7fffffff; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0x40000001;
    // (hi|lo)=(s7|s6) + s4 * s5 ２オペランド
    g_cpu->m_s4.d[0] = 0x0000ffff; g_cpu->m_s5.d[0] = 0x0001ffff; g_cpu->m_s6.d[0] = 0x00100000; g_cpu->m_s7.d[0] = 0x00000010;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x00000006);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0x950008a1);  // lo
    
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000046);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0xffffffbc);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x000d0001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x80000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0x80000001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[1] == 0x00000000 && g_cpu->m_hi.d[0] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[1] == 0x00000000 && g_cpu->m_lo.d[0] == 0x000d0001);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// maddu1 ノーマル
BEGIN_TESTF(r5900_mmi_maddu1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 乗算結果の下位Wordが負数（オーバーフロー）
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 乗算結果が負数
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 下位Wordの加算にてキャリー発生するケース
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 上位Wordの結果が負数
    g_cpu->m_s0.d[0] = 0x80000000; g_cpu->m_s1.d[0] = 0x7fffffff; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0x40000001;
    // (hi|lo)=(s7|s6) + s4 * s5 ２オペランド
    g_cpu->m_s4.d[0] = 0x0000ffff; g_cpu->m_s5.d[0] = 0x0001ffff; g_cpu->m_s6.d[0] = 0x00100000; g_cpu->m_s7.d[0] = 0x00000010;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x00000006);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0x950008a1);  // lo
    
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000046);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0xffffffbc);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x000d0001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x80000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0x80000001);  // lo

    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x00000000 && g_cpu->m_hi.d[2] == 0x00000012);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x00000000 && g_cpu->m_lo.d[2] == 0x000d0001);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
