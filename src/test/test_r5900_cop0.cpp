#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_cop0_fix_01);

SETUP(r5900_cop0_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_cop0_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

// mf0 mt0
BEGIN_TESTF(r5900_cop0_mf0_mt0_001, r5900_cop0_fix_01)
{
  try
  {
    OPEN_ELF("cop0");
    INIT_GPR(0xcccccccc);
    g_cpu->m_cop0.index    = 0x12345678;
    g_cpu->m_cop0.random  = 0x12345679;
    g_cpu->m_cop0.entrylo0  = 0x1234567a;
    g_cpu->m_cop0.entrylo1  = 0x1234567b;
    g_cpu->m_cop0.context  = 0x1234567c;
    g_cpu->m_cop0.pagemask  = 0x4234567d;
    g_cpu->m_cop0.wired    = 0x4234567e;
    g_cpu->m_cop0.badvaddr  = 0x4234567f;
    g_cpu->m_cop0.count    = 0x42345680;
    g_cpu->m_cop0.entryhi  = 0x42345681;
    g_cpu->m_cop0.compare  = 0x82345682;
    g_cpu->m_cop0.status  = 0x82345683;
    g_cpu->m_cop0.cause    = 0x82345684;
    g_cpu->m_cop0.epc    = 0x82345685;
    g_cpu->m_cop0.prid    = 0x82345686;
    g_cpu->m_cop0.config  = 0x82345687;
    g_cpu->m_cop0.badpaddr  = 0x82345688;
    g_cpu->m_cop0.bpc    = 0xf2345689;
    g_cpu->m_cop0.pccr    = 0xf234568a;
    g_cpu->m_cop0.taglo    = 0xf234568b;
    g_cpu->m_cop0.taghi    = 0xf234568c;
    g_cpu->m_cop0.errorepc  = 0xf234568d;

    g_cpu->m_v1.d[0]    = 0xdddddd00;

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x12345678);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x12345679);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x1234567a);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x1234567b);
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x1234567c);
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x4234567d);
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0x00000000 && g_cpu->m_t6.d[0] == 0x4234567e);
    WIN_ASSERT_TRUE(g_cpu->m_t7.d[1] == 0x00000000 && g_cpu->m_t7.d[0] == 0x4234567f);
    WIN_ASSERT_TRUE(g_cpu->m_t8.d[1] == 0x00000000 && g_cpu->m_t8.d[0] == 0x42345680);
    WIN_ASSERT_TRUE(g_cpu->m_t9.d[1] == 0x00000000 && g_cpu->m_t9.d[0] == 0x42345681);
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0xffffffff && g_cpu->m_s0.d[0] == 0x82345682);
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0x82345683);
    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0x82345684);
    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0xffffffff && g_cpu->m_s3.d[0] == 0x82345685);
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0xffffffff && g_cpu->m_s4.d[0] == 0x82345686);
    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0xffffffff && g_cpu->m_s5.d[0] == 0x82345687);
    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0xffffffff && g_cpu->m_s6.d[0] == 0x82345688);
    WIN_ASSERT_TRUE(g_cpu->m_s7.d[1] == 0xffffffff && g_cpu->m_s7.d[0] == 0xf2345689);
    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0xffffffff && g_cpu->m_a0.d[0] == 0xf234568a);
    WIN_ASSERT_TRUE(g_cpu->m_a1.d[1] == 0xffffffff && g_cpu->m_a1.d[0] == 0xf234568b);
    WIN_ASSERT_TRUE(g_cpu->m_a2.d[1] == 0xffffffff && g_cpu->m_a2.d[0] == 0xf234568c);
    WIN_ASSERT_TRUE(g_cpu->m_a3.d[1] == 0xffffffff && g_cpu->m_a3.d[0] == 0xf234568d);

    WIN_ASSERT_TRUE(g_cpu->m_cop0.index    == 0xdddddd00);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.random  == 0xdddddd01);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.entrylo0  == 0xdddddd02);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.entrylo1  == 0xdddddd03);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.context  == 0xdddddd04);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.pagemask  == 0xdddddd05);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.wired    == 0xdddddd06);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.badvaddr  == 0xdddddd07);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.count    == 0xdddddd08);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.entryhi  == 0xdddddd09);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.compare  == 0xdddddd0a);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.status  == 0xdddddd0b);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.cause    == 0xdddddd0c);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.epc    == 0xdddddd0d);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.prid    == 0xdddddd0e);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.config  == 0xdddddd0f);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.badpaddr  == 0xdddddd10);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.bpc    == 0xdddddd11);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.pccr    == 0xdddddd12);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.taglo    == 0xdddddd13);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.taghi    == 0xdddddd14);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.errorepc  == 0xdddddd15);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mf0 mt0
BEGIN_TESTF(r5900_cop0_mf0_mt0_002, r5900_cop0_fix_01)
{
  try
  {
    OPEN_ELF("cop0");
    INIT_GPR(0xcccccccc);
    g_cpu->m_cop0.iab    = 0x12345679;
    g_cpu->m_cop0.iabm    = 0x4234567a;
    g_cpu->m_cop0.dab    = 0x4234567b;
    g_cpu->m_cop0.dabm    = 0x8234567c;
    g_cpu->m_cop0.dvb    = 0x8234567d;
    g_cpu->m_cop0.dvbm    = 0xf234567e;
    g_cpu->m_cop0.pccr    = 0xf234567f;
    g_cpu->m_cop0.pcr0    = 0xf2345680;
    g_cpu->m_cop0.pcr1    = 0xf2345681;

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_cop0.iab  == 0x4234567a);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.iabm  == 0x4234567b);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.dab  == 0x8234567c);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.dabm  == 0x8234567d);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.dvb  == 0xf234567e);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.dvbm  == 0xf234567f);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.pccr  == 0xf2345680);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.pcr0  == 0xf2345681);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.pcr1  == 0x12345679);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bc0f bc0t bc0fl bc0tl
BEGIN_TESTF(r5900_cop0_bc0f_bc0t_bc0fl_bc0tl_001, r5900_cop0_fix_01) {
#if 0 // dmac使ってないしどうでもいい
  try {
    OPEN_ELF("cop0");
    INIT_GPR(0xcccccccc);

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0x00000003);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0x00000002);

    // 転送が終了していないビットがある
    g_cpu->init();

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0x00000003);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0x00000002);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0x00000001);

    // ステータスを出力しないビットは無視される
    g_cpu->init();
    
    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0x00000003);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0x00000002);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
#endif
}END_TESTF

// eret
BEGIN_TESTF(r5900_cop0_eret_001, r5900_cop0_fix_01)
{
  try
  {
    OPEN_ELF("cop0");
    INIT_GPR(0xcccccccc);
    g_cpu->m_cop0.status  = (COP0::STATUS_ERL | COP0::STATUS_EXL | COP0::STATUS_BEV);
    g_cpu->m_cop0.epc    = 0x0000000c;
    g_cpu->m_cop0.errorepc  = 0x00000014;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[0] == 0x00000002);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.status == (COP0::STATUS_EXL | COP0::STATUS_BEV));

    g_cpu->init();
    INIT_GPR(0xcccccccc);
    g_cpu->m_cop0.status  = COP0::STATUS_EXL | COP0::STATUS_BEV;
    g_cpu->m_cop0.epc    = 0x0000000c;
    g_cpu->m_cop0.errorepc  = 0x00000014;
    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_v0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_cop0.status == COP0::STATUS_BEV);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ei di
BEGIN_TESTF(r5900_cop0_ei_di_001, r5900_cop0_fix_01) {
  try {
    OPEN_ELF("cop0");
    INIT_GPR(0xcccccccc);

    // 変化しない
    g_cpu->m_t0.d[0] = COP0::STATUS_KSU;
    g_cpu->m_t1.d[0] = COP0::STATUS_KSU | COP0::STATUS_EIE;
    g_cpu->execute();
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP0::STATUS_EIE) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP0::STATUS_EIE) != 0);

    // EDIフラグあり
    g_cpu->init();
    g_cpu->m_t0.d[0] = COP0::STATUS_KSU | COP0::STATUS_EDI;
    g_cpu->m_t1.d[0] = COP0::STATUS_KSU | COP0::STATUS_EDI | COP0::STATUS_EIE;
    g_cpu->execute();
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP0::STATUS_EIE) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP0::STATUS_EIE) == 0);

    // EXLフラグあり
    g_cpu->init();
    g_cpu->m_t0.d[0] = COP0::STATUS_KSU | COP0::STATUS_EXL;
    g_cpu->m_t1.d[0] = COP0::STATUS_KSU | COP0::STATUS_EXL | COP0::STATUS_EIE;
    g_cpu->execute();
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP0::STATUS_EIE) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP0::STATUS_EIE) == 0);

    // ERLフラグあり
    g_cpu->init();
    g_cpu->m_t0.d[0] = COP0::STATUS_KSU | COP0::STATUS_ERL;
    g_cpu->m_t1.d[0] = COP0::STATUS_KSU | COP0::STATUS_ERL | COP0::STATUS_EIE;
    g_cpu->execute();
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP0::STATUS_EIE) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP0::STATUS_EIE) == 0);

    // KSUが0
    g_cpu->init();
    g_cpu->m_t0.d[0] = 0;
    g_cpu->m_t1.d[0] = COP0::STATUS_EIE;
    g_cpu->execute();
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP0::STATUS_EIE) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP0::STATUS_EIE) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
