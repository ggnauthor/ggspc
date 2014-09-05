#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_mmi3_fix_01);

SETUP(r5900_mmi3_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_mmi3_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// pmadduw ノーマル
BEGIN_TESTF(r5900_mmi3_pmadduw_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[3] = 0x00000000; g_cpu->m_a0.d[2] = 0xf0000000;  g_cpu->m_a0.d[1] = 0x00000000; g_cpu->m_a0.d[0] = 0xabcdef12;
    g_cpu->m_s0.d[3] = 0x00000000; g_cpu->m_s0.d[2] = 0x00000001;  g_cpu->m_s0.d[1] = 0x00000000; g_cpu->m_s0.d[0] = 0x00001234;
    g_cpu->m_hi.d[3] = 0x00000000; g_cpu->m_hi.d[2] = 0x00000001;  g_cpu->m_hi.d[1] = 0x00000000; g_cpu->m_hi.d[0] = 0x00001111;
    g_cpu->m_lo.d[3] = 0x00000000; g_cpu->m_lo.d[2] = 0x20000000;  g_cpu->m_lo.d[1] = 0x00000000; g_cpu->m_lo.d[0] = 0x00002222;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x00000000 && g_cpu->m_hi.d[2] == 0x00000002 && g_cpu->m_hi.d[1] == 0x00000000 && g_cpu->m_hi.d[0] == 0x00001d48);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x00000000 && g_cpu->m_lo.d[2] == 0x10000000 && g_cpu->m_lo.d[1] == 0x00000000 && g_cpu->m_lo.d[0] == 0x60a3f5ca);
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x00000002 && g_cpu->m_t0.d[2] == 0x10000000 && g_cpu->m_t0.d[1] == 0x00001d48 && g_cpu->m_t0.d[0] == 0x60a3f5ca);

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

// pmthi pmtlo ノーマル
BEGIN_TESTF(r5900_mmi3_pmthi_pmtlo_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);

    g_cpu->m_a0.d[3] = 0x01234567; g_cpu->m_a0.d[2] = 0x89abcdef; g_cpu->m_a0.d[1] = 0xcccccccc; g_cpu->m_a0.d[0] = 0x88888888;
    g_cpu->m_a1.d[3] = 0x11111111; g_cpu->m_a1.d[2] = 0x22222222; g_cpu->m_a1.d[1] = 0x12345678; g_cpu->m_a1.d[0] = 0x55556666;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_hi.d[3] == 0x01234567 && g_cpu->m_hi.d[2] == 0x89abcdef && g_cpu->m_hi.d[1] == 0xcccccccc && g_cpu->m_hi.d[0] == 0x88888888);
    WIN_ASSERT_TRUE(g_cpu->m_lo.d[3] == 0x11111111 && g_cpu->m_lo.d[2] == 0x22222222 && g_cpu->m_lo.d[1] == 0x12345678 && g_cpu->m_lo.d[0] == 0x55556666);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pmultuw ノーマル
BEGIN_TESTF(r5900_mmi3_pmultuw_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[2] = 0x00010000; g_cpu->m_t0.d[2] = 0x00010000;  g_cpu->m_a0.d[0] = 0x00000020; g_cpu->m_t0.d[0] = 0x00000045;
    g_cpu->m_a1.d[2] = 0xffffffff; g_cpu->m_t1.d[2] = 0x00000100;  g_cpu->m_a1.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000002;
    g_cpu->m_a2.d[2] = 0xffffffff; g_cpu->m_t2.d[2] = 0xffffffff;  g_cpu->m_a2.d[0] = 0x7fffffff; g_cpu->m_t2.d[0] = 0x7fffffff;
    g_cpu->m_a3.d[2] = 0x00000000; g_cpu->m_t3.d[2] = 0x00000000;  g_cpu->m_a3.d[0] = 0x80000000; g_cpu->m_t3.d[0] = 0x7fffffff;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x00000000 && g_cpu->m_a0.d[2] == 0x00000001 && g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00000000);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0x00000000 && g_cpu->m_s0.d[2] == 0x00000000 && g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x000008a0);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_ze.d[3] == 0x00000000 && g_cpu->m_ze.d[2] == 0x00000000 && g_cpu->m_ze.d[1] == 0x00000000 && g_cpu->m_ze.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x00000000 && g_cpu->m_a1.d[2] == 0x000000ff && g_cpu->m_a1.d[1] == 0x00000000 && g_cpu->m_a1.d[0] == 0x00000001);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0xffffffff && g_cpu->m_s1.d[2] == 0xffffff00 && g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xfffffffe);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_ze.d[3] == 0x00000000 && g_cpu->m_ze.d[2] == 0x00000000 && g_cpu->m_ze.d[1] == 0x00000000 && g_cpu->m_ze.d[0] == 0x00000000);

    WIN_ASSERT_TRUE(g_cpu->m_a2.d[3] == 0xffffffff && g_cpu->m_a2.d[2] == 0xfffffffe && g_cpu->m_a2.d[1] == 0x00000000 && g_cpu->m_a2.d[0] == 0x3fffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x00000000 && g_cpu->m_s2.d[2] == 0x00000001 && g_cpu->m_s2.d[1] == 0x00000000 && g_cpu->m_s2.d[0] == 0x00000001);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[3] == g_cpu->m_a2.d[2] && g_cpu->m_v0.d[2] == g_cpu->m_s2.d[2] && g_cpu->m_v0.d[1] == g_cpu->m_a2.d[0] && g_cpu->m_v0.d[0] == g_cpu->m_s2.d[0]);

    WIN_ASSERT_TRUE(g_cpu->m_a3.d[3] == 0x00000000 && g_cpu->m_a3.d[2] == 0x00000000 && g_cpu->m_a3.d[1] == 0x00000000 && g_cpu->m_a3.d[0] == 0x3fffffff);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[3] == 0x00000000 && g_cpu->m_s3.d[2] == 0x00000000 && g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x80000000);  // lo
    WIN_ASSERT_TRUE(g_cpu->m_v1.d[3] == g_cpu->m_a3.d[2] && g_cpu->m_v1.d[2] == g_cpu->m_s3.d[2] && g_cpu->m_v1.d[1] == g_cpu->m_a3.d[0] && g_cpu->m_v1.d[0] == g_cpu->m_s3.d[0]);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pdivuw ノーマル
BEGIN_TESTF(r5900_mmi3_pdivuw_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[0] = 0x00000045; g_cpu->m_s0.d[0] = 0x00000020;  // 被除数が正・除数が正
    g_cpu->m_t1.d[0] = 0xfffffffd; g_cpu->m_s1.d[0] = 0x00000002;  // 被除数が負・除数が正
    g_cpu->m_t2.d[0] = 0xfffffffd; g_cpu->m_s2.d[0] = 0xfffffffe;  // 被除数が負・除数が負
    g_cpu->m_t3.d[0] = 0x00000003; g_cpu->m_s3.d[0] = 0xfffffffe;  // 被除数が正・除数が負

    g_cpu->m_t0.d[2] = 0x00001234; g_cpu->m_s0.d[2] = 0x00000000;  // 0除算
    g_cpu->m_t1.d[2] = 0x40000000; g_cpu->m_s1.d[2] = 0x00000001;  // 1除算 商が正
    g_cpu->m_t2.d[2] = 0x80000000; g_cpu->m_s2.d[2] = 0x00000001;  // 1除算 商が負
    g_cpu->m_t3.d[2] = 0x0012001f; g_cpu->m_s3.d[2] = 0x00000040;  // 普通の計算

    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = 0xffffffff;
    g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xeeeeeeee;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = 0xdddddddd;
    g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[3] == 0xffffffff && g_cpu->m_s0.d[2] == 0xffffffff);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0xdddddddd && g_cpu->m_t0.d[2] == 0xdddddddd);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s1.d[3] == 0x00000000 && g_cpu->m_s1.d[2] == 0x00000000);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[3] == 0x00000000 && g_cpu->m_t1.d[2] == 0x40000000);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[3] == 0x00000000 && g_cpu->m_s2.d[2] == 0x00000000);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[3] == 0xffffffff && g_cpu->m_t2.d[2] == 0x80000000);  // lo 商

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[3] == 0x00000000 && g_cpu->m_s3.d[2] == 0x0000001f);  // hi 余
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[3] == 0x00000000 && g_cpu->m_t3.d[2] == 0x00004800);  // lo 商
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pinteh ノーマル
BEGIN_TESTF(r5900_mmi3_pinteh_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[3] = 0x11112222; g_cpu->m_s0.d[2] = 0x33334444; g_cpu->m_s0.d[1] = 0x55556666; g_cpu->m_s0.d[0] = 0x77778888;
    g_cpu->m_t0.d[3] = 0x9999aaaa; g_cpu->m_t0.d[2] = 0xbbbbcccc; g_cpu->m_t0.d[1] = 0xddddeeee; g_cpu->m_t0.d[0] = 0xffff0000;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x2222aaaa && g_cpu->m_a0.d[2] == 0x4444cccc && g_cpu->m_a0.d[1] == 0x6666eeee && g_cpu->m_a0.d[0] == 0x88880000);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pcpyh ノーマル
BEGIN_TESTF(r5900_mmi3_pcpyh_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[3] = 0xffffeeee; g_cpu->m_t0.d[2] = 0xdddd1234; g_cpu->m_t0.d[1] = 0xccccbbbb; g_cpu->m_t0.d[0] = 0xaaaa5678;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x12341234 && g_cpu->m_a0.d[2] == 0x12341234 && g_cpu->m_a0.d[1] == 0x56785678 && g_cpu->m_a0.d[0] == 0x56785678);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pcpyud ノーマル
BEGIN_TESTF(r5900_mmi3_pcpyud_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_s0.d[3] = 0x11111111; g_cpu->m_s0.d[2] = 0x22222222; g_cpu->m_s0.d[1] = 0x33333333; g_cpu->m_s0.d[0] = 0x44444444;
    g_cpu->m_t0.d[3] = 0x55555555; g_cpu->m_t0.d[2] = 0x66666666; g_cpu->m_t0.d[1] = 0x77777777; g_cpu->m_t0.d[0] = 0x88888888;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x55555555 && g_cpu->m_a0.d[2] == 0x66666666 && g_cpu->m_a0.d[1] == 0x11111111 && g_cpu->m_a0.d[0] == 0x22222222);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// pexcw pexch ノーマル
BEGIN_TESTF(r5900_mmi3_pexcw_pexch_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[3] = 0x11112222; g_cpu->m_t0.d[2] = 0x33334444; g_cpu->m_t0.d[1] = 0x55556666; g_cpu->m_t0.d[0] = 0x77778888;
    g_cpu->m_t1.d[3] = 0x11112222; g_cpu->m_t1.d[2] = 0x33334444; g_cpu->m_t1.d[1] = 0x55556666; g_cpu->m_t1.d[0] = 0x77778888;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x11112222 && g_cpu->m_a0.d[2] == 0x55556666 && g_cpu->m_a0.d[1] == 0x33334444 && g_cpu->m_a0.d[0] == 0x77778888);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0x11113333 && g_cpu->m_a1.d[2] == 0x22224444 && g_cpu->m_a1.d[1] == 0x55557777 && g_cpu->m_a1.d[0] == 0x66668888);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// por pnor ノーマル
BEGIN_TESTF(r5900_mmi3_por_pnor_001, r5900_mmi3_fix_01){
  try
  {
    OPEN_ELF("mmi3");
    INIT_GPR(0xcccccccc);

    g_cpu->m_s0.d[3] = 0x00110000; g_cpu->m_s0.d[2] = 0x334466ff; g_cpu->m_s0.d[1] = 0x01030507; g_cpu->m_s0.d[0] = 0x12345678;
    g_cpu->m_t0.d[3] = 0x000088ff; g_cpu->m_t0.d[2] = 0x22cc6688; g_cpu->m_t0.d[1] = 0x20406080; g_cpu->m_t0.d[0] = 0xedcba987;
    
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[3] == 0x001188ff && g_cpu->m_a0.d[2] == 0x33cc66ff && g_cpu->m_a0.d[1] == 0x21436587 && g_cpu->m_a0.d[0] == 0xffffffff);  // or
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[3] == 0xffee7700 && g_cpu->m_a1.d[2] == 0xcc339900 && g_cpu->m_a1.d[1] == 0xdebc9a78 && g_cpu->m_a1.d[0] == 0x00000000);  // nor
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
