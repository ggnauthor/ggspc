#if defined _UNIT_TEST

#include "test/test_r5900.h"

#include "debug_malloc.h"

FIXTURE(r5900_special_fix_01);

SETUP(r5900_special_fix_01) {
  // ���C���X���b�h�����s�t�@�C�o�ɕϊ�
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_special_fix_01) {
  delete g_memory;
  delete g_cpu;

  // �t�@�C�o����X���b�h�ɖ߂�
  ConvertFiberToThread();
}

// jr jalr �m�[�}��
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

// sll srl sra �m�[�}��
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

// sllv srlv srav �m�[�}��
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

// movz movn �m�[�}��
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

// mflo mfhi mtlo mthi �m�[�}��
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

// mult �m�[�}��
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

// multu �m�[�}��
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

// div �m�[�}��
BEGIN_TESTF(r5900_special_div_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[0] = 0x00000045; g_cpu->m_s0.d[0] = 0x00000020;  // �폜�������E���������@���@�������E�]����
    g_cpu->m_t1.d[0] = 0xfffffffd; g_cpu->m_s1.d[0] = 0x00000002;  // �폜�������E���������@���@�������E�]����
    g_cpu->m_t2.d[0] = 0xfffffffd; g_cpu->m_s2.d[0] = 0xfffffffe;  // �폜�������E���������@���@�������E�]����
    g_cpu->m_t3.d[0] = 0x00000003; g_cpu->m_s3.d[0] = 0xfffffffe;  // �폜�������E���������@���@�������E�]����

    g_cpu->m_t4.d[0] = 0x00001234; g_cpu->m_s4.d[0] = 0x00000000;  // 0���Z

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000005);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000002);  // lo ��
    
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0xffffffff && g_cpu->m_s1.d[0] == 0xffffffff);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0xffffffff);  // lo ��

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0xffffffff);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000001);  // lo ��

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000001);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xffffffff && g_cpu->m_t3.d[0] == 0xffffffff);  // lo ��
    
    // 0���Z��hi, lo���ω����Ȃ��̂őO��̉��Z���ʂƓ������̂��i�[�����B�O�̂��߃`�F�b�N
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x00000001);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0xffffffff && g_cpu->m_t4.d[0] == 0xffffffff);  // lo ��
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// divu �m�[�}��
BEGIN_TESTF(r5900_special_divu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[0] = 0x00000045; g_cpu->m_s0.d[0] = 0x00000020;  // �폜�������E��������
    g_cpu->m_t1.d[0] = 0xfffffffd; g_cpu->m_s1.d[0] = 0x00000002;  // �폜�������E��������
    g_cpu->m_t2.d[0] = 0xfffffffd; g_cpu->m_s2.d[0] = 0xfffffffe;  // �폜�������E��������
    g_cpu->m_t3.d[0] = 0x00000003; g_cpu->m_s3.d[0] = 0xfffffffe;  // �폜�������E��������

    g_cpu->m_t4.d[0] = 0x00001234; g_cpu->m_s4.d[0] = 0x00000000;  // 0���Z
    g_cpu->m_t5.d[0] = 0x40000000; g_cpu->m_s5.d[0] = 0x00000001;  // 1���Z ������
    g_cpu->m_t6.d[0] = 0x80000000; g_cpu->m_s6.d[0] = 0x00000001;  // 1���Z ������

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_s0.d[1] == 0x00000000 && g_cpu->m_s0.d[0] == 0x00000005);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00000002);  // lo ��
    
    WIN_ASSERT_TRUE(g_cpu->m_s1.d[1] == 0x00000000 && g_cpu->m_s1.d[0] == 0x00000001);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0x00000000 && g_cpu->m_t1.d[0] == 0x7ffffffe);  // lo ��

    WIN_ASSERT_TRUE(g_cpu->m_s2.d[1] == 0xffffffff && g_cpu->m_s2.d[0] == 0xfffffffd);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0x00000000 && g_cpu->m_t2.d[0] == 0x00000000);  // lo ��

    WIN_ASSERT_TRUE(g_cpu->m_s3.d[1] == 0x00000000 && g_cpu->m_s3.d[0] == 0x00000003);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x00000000);  // lo ��
    
    // 0���Z��hi, lo���ω����Ȃ��̂őO��̉��Z���ʂƓ������̂��i�[�����B�O�̂��߃`�F�b�N
    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == 0x00000000 && g_cpu->m_s4.d[0] == 0x00000003);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == 0x00000000 && g_cpu->m_t4.d[0] == 0x00000000);  // lo ��

    WIN_ASSERT_TRUE(g_cpu->m_s5.d[1] == 0x00000000 && g_cpu->m_s5.d[0] == 0x00000000);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t5.d[1] == 0x00000000 && g_cpu->m_t5.d[0] == 0x40000000);  // lo ��

    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == 0x00000000 && g_cpu->m_s6.d[0] == 0x00000000);  // hi �]
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == 0xffffffff && g_cpu->m_t6.d[0] == 0x80000000);  // lo ��
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// add �m�[�}�����I�[�o�[�t���[�������g��
BEGIN_TESTF(r5900_special_add_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0x00000222;  // ���ʐ� (�[���g��)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0x00000002;  // +���I�[�o�[�t���[
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x80000000;   // ���ʕ� (�����g��)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0xfffffffe;  // -���I�[�o�[�t���[

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xcccccccc && g_cpu->m_t1.d[0] == 0xcccccccc);  // �s��
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);  // �s��

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00007ffe);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// addu �m�[�}�����I�[�o�[�t���[�������g��
BEGIN_TESTF(r5900_special_addu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0x00000222;  // ���ʐ� (�[���g��)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0x00000002;  // +���I�[�o�[�t���[���Ȃ�
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x80000000;   // ���ʕ� (�����g��)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0xfffffffe;  // -���I�[�o�[�t���[���Ȃ�

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000000);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x7fffffff);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00007ffe);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// sub �m�[�}�����I�[�o�[�t���[�������g��
BEGIN_TESTF(r5900_special_sub_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0xfffffdde;  // ���ʐ� (�[���g��)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xfffffffe;  // +���I�[�o�[�t���[
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x7fffffff;   // ���ʕ� (�����g��)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0x00000002;  // -���I�[�o�[�t���[

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xcccccccc && g_cpu->m_t1.d[0] == 0xcccccccc);  // �s��
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0xcccccccc && g_cpu->m_t3.d[0] == 0xcccccccc);  // �s��

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00003ffd);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// subu �m�[�}�����I�[�o�[�t���[�������g��
BEGIN_TESTF(r5900_special_subu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_a0.d[0] = 0x00003fff; g_cpu->m_s0.d[0] = 0xfffffdde;  // ���ʐ� (�[���g��)
    g_cpu->m_a1.d[0] = 0x7fffffff; g_cpu->m_s1.d[0] = 0xfffffffe;  // +���I�[�o�[�t���[
    g_cpu->m_a2.d[0] = 0x00000000; g_cpu->m_s2.d[0] = 0x7fffffff;   // ���ʕ� (�����g��)
    g_cpu->m_a3.d[0] = 0x80000001; g_cpu->m_s3.d[0] = 0x00000002;  // -���I�[�o�[�t���[

    g_cpu->execute();
    WIN_ASSERT_TRUE(g_cpu->m_t0.d[1] == 0x00000000 && g_cpu->m_t0.d[0] == 0x00004221);
    WIN_ASSERT_TRUE(g_cpu->m_t1.d[1] == 0xffffffff && g_cpu->m_t1.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t2.d[1] == 0xffffffff && g_cpu->m_t2.d[0] == 0x80000001);
    WIN_ASSERT_TRUE(g_cpu->m_t3.d[1] == 0x00000000 && g_cpu->m_t3.d[0] == 0x7fffffff);

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x00003ffd);
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// and or xor nor �m�[�}��
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

// slt �m�[�}�����I�[�o�[�t���[
BEGIN_TESTF(r5900_special_slt_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[1] = 0x00000000; g_cpu->m_t0.d[0] = 0x00000000;
    g_cpu->m_t1.d[1] = 0x00000000; g_cpu->m_t1.d[0] = 0x00000001;
    g_cpu->m_t2.d[1] = 0xffffffff; g_cpu->m_t2.d[0] = 0xffffffff;
    g_cpu->m_t3.d[1] = 0x7fffffff; g_cpu->m_t3.d[0] = 0x00000000;  // ���WORD�I�[�o�[�t���[+
    g_cpu->m_t4.d[1] = 0x80000000; g_cpu->m_t4.d[0] = 0x00000000;  // ���WORD�I�[�o�[�t���[-
    g_cpu->m_t5.d[1] = 0x00000000; g_cpu->m_t5.d[0] = 0x7fffffff;  // ����WORD�I�[�o�[�t���[+
    g_cpu->m_t6.d[1] = 0x00000000; g_cpu->m_t6.d[0] = 0x80000000;  // ����WORD�I�[�o�[�t���[-
    g_cpu->m_t7.d[1] = 0x00000001; g_cpu->m_t7.d[0] = 0x00000000;  // ���WORD�ł̑召��r

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

// sltu �m�[�}�����I�[�o�[�t���[
BEGIN_TESTF(r5900_special_sltu_001, r5900_special_fix_01)
{
  try
  {
    OPEN_ELF("special");
    INIT_GPR(0xcccccccc);
    g_cpu->m_t0.d[1] = 0x00000000; g_cpu->m_t0.d[0] = 0x00000000;
    g_cpu->m_t1.d[1] = 0x00000000; g_cpu->m_t1.d[0] = 0x00000001;
    g_cpu->m_t2.d[1] = 0xffffffff; g_cpu->m_t2.d[0] = 0xffffffff;
    g_cpu->m_t3.d[1] = 0x7fffffff; g_cpu->m_t3.d[0] = 0x00000000;  // ���WORD�I�[�o�[�t���[+
    g_cpu->m_t4.d[1] = 0x80000000; g_cpu->m_t4.d[0] = 0x00000000;  // ���WORD�I�[�o�[�t���[-
    g_cpu->m_t5.d[1] = 0x00000000; g_cpu->m_t5.d[0] = 0x7fffffff;  // ����WORD�I�[�o�[�t���[+
    g_cpu->m_t6.d[1] = 0x00000000; g_cpu->m_t6.d[0] = 0x80000000;  // ����WORD�I�[�o�[�t���[-
    g_cpu->m_t7.d[1] = 0x00000001; g_cpu->m_t7.d[0] = 0x00000000;  // ���WORD�ł̑召��r

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

// dsll �m�[�}��
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

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x02468acf && g_cpu->m_a0.d[0] == 0x13579bde);  // src��dst�������P�[�X
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsll32 �m�[�}��
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

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x13579bde && g_cpu->m_a0.d[0] == 0x00000000);  // src��dst�������P�[�X
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsrl �m�[�}��
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

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x0091a2b3 && g_cpu->m_a0.d[0] == 0xc4d5e6f7);  // src��dst�������P�[�X
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsrl32 �m�[�}��
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

    WIN_ASSERT_TRUE(g_cpu->m_a0.d[1] == 0x00000000 && g_cpu->m_a0.d[0] == 0x0091a2b3);  // src��dst�������P�[�X
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// dsra dsra32 �m�[�}��
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

// dsllv �m�[�}��
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

// dsrlv �m�[�}��
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

// dsrav �m�[�}��
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
