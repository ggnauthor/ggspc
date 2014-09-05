#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_cop1_fix_01);

SETUP(r5900_cop1_fix_01) {
  // メインスレッドを実行ファイバに変換
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_cop1_fix_01) {
  delete g_memory;
  delete g_cpu;

  // ファイバからスレッドに戻す
  ConvertFiberToThread();
}

void execute_with_fprtag_check()
{
  // 最初にFPRタグをクリアし、実行後にもオールクリアであることを確認
  _asm finit;
  g_cpu->execute();
  u8 fenv[28];
  _asm fstenv fenv;
  WIN_ASSERT_TRUE(fenv[8] == 0xff && fenv[9] == 0xff);
}

// mfc1 mtc1
BEGIN_TESTF(r5900_cop1_mfc1_mtc1_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    *((u32*)&g_cpu->m_fpu.fpr[0])  = 0x12345679;
    *((u32*)&g_cpu->m_fpu.fpr[1])  = 0x80000000;
    *((u32*)&g_cpu->m_fpu.fpr[31])  = 0x7fffffff;
    g_cpu->m_a0.d[0]        = 0x12121212;
    g_cpu->m_a1.d[0]        = 0x34343434;
    g_cpu->m_a2.d[0]        = 0x56565656;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x00000000 && g_cpu->m_t0.d[2] == 0x00000000 && g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x12345679);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[3] == 0xffffffff && g_cpu->m_t1.d[2] == 0xffffffff && g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[3] == 0x00000000 && g_cpu->m_t2.d[2] == 0x00000000 && g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x7fffffff);

    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fpr[3]) == 0x12121212);
    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fpr[7]) == 0x34343434);
    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fpr[11]) == 0x56565656);

  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// cfc1 ctc1
BEGIN_TESTF(r5900_cop1_cfc1_ctc1_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    *((u32*)&g_cpu->m_fpu.fcr0.d)  = 0x7fffffff;
    *((u32*)&g_cpu->m_fpu.fcr31)  = 0x80000000;
    g_cpu->m_a0.d[0]        = 0x12121212;
    g_cpu->m_a1.d[0]        = 0x34343434;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[3] == 0x00000000 && g_cpu->m_t0.d[2] == 0x00000000 && g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x7fffffff);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[3] == 0xffffffff && g_cpu->m_t1.d[2] == 0xffffffff && g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0x80000000);

    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fcr0.d) == 0x12121212);
    WIN_ASSERT_TRUE(*((u32*)&g_cpu->m_fpu.fcr31) == 0x34343434);

  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// bc1f bc1t bc1fl bc1tl
BEGIN_TESTF(r5900_cop1_bc1f_bc1t_bc1fl_bc1tl_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fcr31 = 0;

    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0x00000003);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0x00000002);

    g_cpu->init();
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fcr31 = COP1::FCR31_C;
    g_cpu->execute();

    WIN_ASSERT_TRUE(g_cpu->m_t0.d[0] == 0x00000003);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[0] == 0x00000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[0] == 0x00000002);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[0] == 0x00000001);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// add_s
BEGIN_TESTF(r5900_cop1_add_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] = 1.0f;
    g_cpu->m_fpu.fpr[1] = 1.5f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == 2.5f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_N);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sub_s
BEGIN_TESTF(r5900_cop1_sub_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[0] = 1.5f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == -0.5f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MAX_N);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MIN_P);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mul_s
BEGIN_TESTF(r5900_cop1_mul_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == -1.25f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // +オーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 1000.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // -オーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = -1000.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // +アンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = 0.001f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
    // -アンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = -0.001f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_ZERO_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// div_s
BEGIN_TESTF(r5900_cop1_div_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_I | COP1::FCR31_SI | COP1::FCR31_D | COP1::FCR31_SD;
    const u32 STICKYFLG  = COP1::FCR31_SI | COP1::FCR31_SD;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 7.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;
    g_cpu->m_fpu.fcr31  = 0xffffffff;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == -15.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // +オーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 0.0001f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);

    // -オーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = -0.0001f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);

    // +アンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 0.001f;
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MAX_P);

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);

    // -アンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = -0.001f;
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MAX_P);

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_ZERO_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // +0除算
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 3.0f;
    g_cpu->m_fpu.fpr[0] = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_D | STICKYFLG));

    // -0除算
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = -3.0f;
    g_cpu->m_fpu.fpr[0] = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_D | STICKYFLG));

    // +不正演算
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 0.0f;
    g_cpu->m_fpu.fpr[0] = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_I | STICKYFLG));

    // -不正演算
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = -0.0f;
    g_cpu->m_fpu.fpr[0] = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_I | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mov_s
BEGIN_TESTF(r5900_cop1_mov_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] = 1.0f;
    g_cpu->m_fpu.fpr[1] = 1.5f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[0] == 1.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[1] == 1.0f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// abs_s neg_s
BEGIN_TESTF(r5900_cop1_abs_neg_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_v0.d[0]  = 0xffffffff;
    g_cpu->m_fpu.fpr[0] =  1.0f;
    g_cpu->m_fpu.fpr[2] = -2.0f;
    g_cpu->m_fpu.fpr[4] =  1.5f;
    g_cpu->m_fpu.fpr[6] = -2.5f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[1] ==  1.0f);
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[3] ==  2.0f);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[5] == -1.5f);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[7] ==  2.5f);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sqrt_s
BEGIN_TESTF(r5900_cop1_sqrt_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_I | COP1::FCR31_SI | COP1::FCR31_D | COP1::FCR31_SD;
    const u32 STICKYFLG  = COP1::FCR31_SI | COP1::FCR31_SD;

    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_v0.d[0]  = 0xffffffff;
    g_cpu->m_fpu.fpr[0] =  3.0f;
    g_cpu->m_fpu.fpr[1] = -2.0f;
    g_cpu->m_fpu.fpr[2] =  0.0f;
    g_cpu->m_fpu.fpr[3] = -0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[10] == 1.7320508f);
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[11] == 1.41421356f);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & AFFECTFLG) == (COP1::FCR31_I | STICKYFLG));

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[12]) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[13]) == COP1::FLT_ZERO_N);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// rsqrt_s
BEGIN_TESTF(r5900_cop1_rsqrt_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_I | COP1::FCR31_SI | COP1::FCR31_D | COP1::FCR31_SD;
    const u32 STICKYFLG  = COP1::FCR31_SI | COP1::FCR31_SD;

    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_v0.d[0]  = 0xffffffff;
    g_cpu->m_fpu.fpr[10] =  1.0f;            g_cpu->m_fpu.fpr[0] = 16.0f;
    g_cpu->m_fpu.fpr[11] =  1.0f;            g_cpu->m_fpu.fpr[1] = -4.0f;
    g_cpu->m_fpu.fpr[12] = -1.0f;            g_cpu->m_fpu.fpr[2] =  0.0f;  // +0の平方根 & 非除算数の符号が考慮されること
    g_cpu->m_fpu.fpr[13] = -1.0f;            g_cpu->m_fpu.fpr[3] = -0.0f;  // -0の平方根 & 非除算数の符号が考慮されること
    g_cpu->m_fpu.fpr[14] = 12.0f;            g_cpu->m_fpu.fpr[4] = 16.0f;
    g_cpu->m_fpu.fpr[15] = -8.0f;            g_cpu->m_fpu.fpr[5] = 16.0f;
    g_cpu->m_fpu.fpr[16] = CASTFLOAT(COP1::FLT_MAX_P);  g_cpu->m_fpu.fpr[6] = 0.5f;    // +オーバーフロー
    g_cpu->m_fpu.fpr[17] = CASTFLOAT(COP1::FLT_MAX_N);  g_cpu->m_fpu.fpr[7] = 0.5f;    // -オーバーフロー
    g_cpu->m_fpu.fpr[18] = CASTFLOAT(COP1::FLT_MIN_P);  g_cpu->m_fpu.fpr[8] = 4.0f;    // +アンダーフロー
    g_cpu->m_fpu.fpr[19] = CASTFLOAT(COP1::FLT_MIN_N);  g_cpu->m_fpu.fpr[9] = 4.0f;    // -アンダーフロー

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[20] == 0.25f);
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[21] == 0.5f);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & AFFECTFLG) == (COP1::FCR31_I | STICKYFLG));

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[22]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[23]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[24] == 3.0f);
    WIN_ASSERT_TRUE((g_cpu->m_t4.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t4.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[25] == -2.0f);
    WIN_ASSERT_TRUE((g_cpu->m_t5.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t5.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[26]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_t6.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t6.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[27]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_t7.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t7.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[28]) == COP1::FLT_ZERO_P);  
    WIN_ASSERT_TRUE((g_cpu->m_t8.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t8.d[0] & AFFECTFLG) == STICKYFLG);

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[29]) == COP1::FLT_ZERO_N);
    WIN_ASSERT_TRUE((g_cpu->m_t9.d[0] & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_t9.d[0] & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// min_s
BEGIN_TESTF(r5900_cop1_min_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[0] = 1.5f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == 1.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);

    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.0f;
    g_cpu->m_fpu.fpr[0] = 1.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == 1.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);

    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = -0.0012f;
    g_cpu->m_fpu.fpr[0] = -0.0022f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == -0.0022f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// max_s
BEGIN_TESTF(r5900_cop1_max_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] = 1.5f;
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[2] = 0.0f;
    g_cpu->m_fpu.fpr[3] = -1.0f;
    g_cpu->m_fpu.fpr[4] = -7.5f;
    g_cpu->m_fpu.fpr[5] = 1.5f;
    g_cpu->m_fpu.fpr[6] = -1.0f;
    g_cpu->m_fpu.fpr[19] = 1.0f;
    g_cpu->m_fpu.fpr[20] = 8.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[10] == 1.5f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[11] == 1.5f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[12] == -1.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[13] == -1.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[14] == 1.5f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[15] == 0.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[16] == 1.5f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[17] == -1.0f);

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[18] == 1.5f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[19] == 1.5f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[20] == 8.0f);

    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// adda_s
BEGIN_TESTF(r5900_cop1_adda_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[0] = 1.5f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == 2.5f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MAX_P);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MIN_N);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// suba_s
BEGIN_TESTF(r5900_cop1_suba_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[0] = 1.5f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == -0.5f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MAX_N);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = CASTFLOAT(COP1::FLT_MIN_P);
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// mula_s
BEGIN_TESTF(r5900_cop1_mula_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == -1.25f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // +オーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 1000.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // -オーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = -1000.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // +アンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = 0.001f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_ZERO_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
    // -アンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = -0.001f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_ZERO_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// madd_s
BEGIN_TESTF(r5900_cop1_madd_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;
    g_cpu->m_fpu.acc  = 1.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == -0.25f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // 乗算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 1000.0f;
    g_cpu->m_fpu.acc  = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 加算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = -1.0f;
    g_cpu->m_fpu.fpr[0] = 1.0e+038f;
    g_cpu->m_fpu.acc  = CASTFLOAT(COP1::FLT_MAX_N);

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 乗算でアンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = 0.001f;
    g_cpu->m_fpu.acc  = 123.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == 123.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// madda_s
BEGIN_TESTF(r5900_cop1_madda_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;
    g_cpu->m_fpu.acc  = 1.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == -0.25f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // 乗算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 1000.0f;
    g_cpu->m_fpu.acc  = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_P);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 加算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = -1.0f;
    g_cpu->m_fpu.fpr[0] = 1.0e+038f;
    g_cpu->m_fpu.acc  = CASTFLOAT(COP1::FLT_MAX_N);

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 乗算でアンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = 0.001f;
    g_cpu->m_fpu.acc  = 123.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == 123.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// msub_s
BEGIN_TESTF(r5900_cop1_msub_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;
    g_cpu->m_fpu.acc  = 1.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == 2.25f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // 乗算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 1000.0f;
    g_cpu->m_fpu.acc  = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 減算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[0] = 1.0e+038f;
    g_cpu->m_fpu.acc  = CASTFLOAT(COP1::FLT_MAX_N);

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.fpr[2]) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 乗算でアンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = 0.001f;
    g_cpu->m_fpu.acc  = 123.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[2] == 123.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// msuba_s
BEGIN_TESTF(r5900_cop1_msuba_001, r5900_cop1_fix_01)
{
  try
  {
    const u32 AFFECTFLG = COP1::FCR31_O | COP1::FCR31_SO | COP1::FCR31_U | COP1::FCR31_SU;
    const u32 STICKYFLG  = COP1::FCR31_SO | COP1::FCR31_SU;

    OPEN_ELF("cop1");
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 2.5f;
    g_cpu->m_fpu.fpr[0] = -0.5f;
    g_cpu->m_fpu.acc  = 1.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == 2.25f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == STICKYFLG);
    
    // 乗算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MAX_P);
    g_cpu->m_fpu.fpr[0] = 1000.0f;
    g_cpu->m_fpu.acc  = 0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 減算でオーバーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = 1.0f;
    g_cpu->m_fpu.fpr[0] = 1.0e+038f;
    g_cpu->m_fpu.acc  = CASTFLOAT(COP1::FLT_MAX_N);

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTUINT(g_cpu->m_fpu.acc) == COP1::FLT_MAX_N);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_O | STICKYFLG));

    // 乗算でアンダーフロー
    g_cpu->init();
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[1] = CASTFLOAT(COP1::FLT_MIN_P);
    g_cpu->m_fpu.fpr[0] = 0.001f;
    g_cpu->m_fpu.acc  = 123.0f;
    
    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.acc == 123.0f);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & ~AFFECTFLG) == ~AFFECTFLG);
    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & AFFECTFLG) == (COP1::FCR31_U | STICKYFLG));
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// cf_s
BEGIN_TESTF(r5900_cop1_cf_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] =  2.5f;
    g_cpu->m_fpu.fpr[1] =  2.5f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE((g_cpu->m_fpu.fcr31 & COP1::FCR31_C) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// ceq_s
BEGIN_TESTF(r5900_cop1_ceq_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] =  2.5f;
    g_cpu->m_fpu.fpr[1] =  2.5f;
    g_cpu->m_fpu.fpr[2] =  2.5003f;
    g_cpu->m_fpu.fpr[3] =  2.5005f;
    g_cpu->m_fpu.fpr[4] =  2.5f;
    g_cpu->m_fpu.fpr[5] = -2.5f;
    g_cpu->m_fpu.fpr[6] =  0.0f;
    g_cpu->m_fpu.fpr[7] = -0.0f;
    g_cpu->m_fpu.fpr[8] = -0.0f;
    g_cpu->m_fpu.fpr[9] =  0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP1::FCR31_C) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP1::FCR31_C) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & COP1::FCR31_C) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & COP1::FCR31_C) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t4.d[0] & COP1::FCR31_C) != 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// clt_s
BEGIN_TESTF(r5900_cop1_clt_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] =  2.5f;
    g_cpu->m_fpu.fpr[1] =  2.5f;
    g_cpu->m_fpu.fpr[2] =  2.5003f;
    g_cpu->m_fpu.fpr[3] =  2.5005f;
    g_cpu->m_fpu.fpr[4] = -2.5003f;
    g_cpu->m_fpu.fpr[5] = -2.5005f;
    g_cpu->m_fpu.fpr[6] =  0.0f;
    g_cpu->m_fpu.fpr[7] = -0.0f;
    g_cpu->m_fpu.fpr[8] = -0.0f;
    g_cpu->m_fpu.fpr[9] =  0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP1::FCR31_C) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP1::FCR31_C) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & COP1::FCR31_C) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & COP1::FCR31_C) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t4.d[0] & COP1::FCR31_C) == 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// cle_s
BEGIN_TESTF(r5900_cop1_cle_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] =  2.5f;
    g_cpu->m_fpu.fpr[1] =  2.5f;
    g_cpu->m_fpu.fpr[2] =  2.5003f;
    g_cpu->m_fpu.fpr[3] =  2.5005f;
    g_cpu->m_fpu.fpr[4] = -2.5003f;
    g_cpu->m_fpu.fpr[5] = -2.5005f;
    g_cpu->m_fpu.fpr[6] =  0.0f;
    g_cpu->m_fpu.fpr[7] = -0.0f;
    g_cpu->m_fpu.fpr[8] = -0.0f;
    g_cpu->m_fpu.fpr[9] =  0.0f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE((g_cpu->m_t0.d[0] & COP1::FCR31_C) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t1.d[0] & COP1::FCR31_C) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t2.d[0] & COP1::FCR31_C) == 0);
    WIN_ASSERT_TRUE((g_cpu->m_t3.d[0] & COP1::FCR31_C) != 0);
    WIN_ASSERT_TRUE((g_cpu->m_t4.d[0] & COP1::FCR31_C) != 0);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// cvt_w_s
BEGIN_TESTF(r5900_cop1_cvtws_001, r5900_cop1_fix_01)
{
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    g_cpu->m_fpu.fpr[0] =  123.456f;
    g_cpu->m_fpu.fpr[2] = -123.456f;
    CASTUINT(g_cpu->m_fpu.fpr[4]) = (0x9d << 23);        // ギリギリクランプ対象外
    CASTUINT(g_cpu->m_fpu.fpr[6]) = (0x9d << 23) | 0x80000000;  // ギリギリクランプ対象外
    CASTUINT(g_cpu->m_fpu.fpr[8]) = (0x9e << 23);        // クランプ対象
    CASTUINT(g_cpu->m_fpu.fpr[10])= (0x9e << 23) | 0x80000000;  // クランプ対象
    g_cpu->m_fpu.fpr[12] =  1.8f;
    g_cpu->m_fpu.fpr[14] = -1.8f;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[1]) ==  123);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[3]) == -123);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[5]) == 0x40000000);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[7]) == 0xc0000000);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[9]) == 0x7fffffff);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[11])== 0x80000000);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[13])==  1);
    WIN_ASSERT_TRUE(CASTINT(g_cpu->m_fpu.fpr[15])== -1);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// cvt_s_w
BEGIN_TESTF(r5900_cop1_cvtsw_001, r5900_cop1_fix_01){
  try
  {
    OPEN_ELF("cop1");
    INIT_GPR(0xcccccccc);
    INIT_FPR(0xcccccccc);
    CASTUINT(g_cpu->m_fpu.fpr[0]) = 123;
    CASTUINT(g_cpu->m_fpu.fpr[2]) = 0;
    CASTUINT(g_cpu->m_fpu.fpr[4]) = 0x00000001;
    CASTUINT(g_cpu->m_fpu.fpr[6]) = 0xffffffff;

    execute_with_fprtag_check();

    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[1] == 123.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[3] == 0.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[5] == 1.0f);
    WIN_ASSERT_TRUE(g_cpu->m_fpu.fpr[7] == -1.0f);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

#endif // #if defined _UNIT_TEST
