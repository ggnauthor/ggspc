#if defined _UNIT_TEST

#include "test/test_r5900.h"

#if defined DEBUG_NEW
  #define new DEBUG_NEW
#endif // defined DEBUG_NEW

FIXTURE(r5900_mmi_fix_01);

SETUP(r5900_mmi_fix_01) {
  // ���C���X���b�h�����s�t�@�C�o�ɕϊ�
  g_app.set_execute_fiber(ConvertThreadToFiber(NULL));

  g_cpu = new R5900();
  g_cpu->init();

  g_memory = new Memory();
}

TEARDOWN(r5900_mmi_fix_01) {
  delete g_memory;
  delete g_cpu;

  // �t�@�C�o����X���b�h�ɖ߂�
  ConvertFiberToThread();
}

// mflo1 mfhi1 mtlo1 mthi1 �m�[�}��
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

// mult1 �m�[�}��
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

// multu1 �m�[�}��
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

// div1 �m�[�}��
BEGIN_TESTF(r5900_mmi_div1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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

    WIN_ASSERT_TRUE(g_cpu->m_s4.d[1] == g_cpu->m_hi.d[3] && g_cpu->m_s4.d[0] == g_cpu->m_hi.d[2]);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t4.d[1] == g_cpu->m_lo.d[3] && g_cpu->m_t4.d[0] == g_cpu->m_lo.d[2]);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// divu1 �m�[�}��
BEGIN_TESTF(r5900_mmi_divu1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

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

    WIN_ASSERT_TRUE(g_cpu->m_s6.d[1] == g_cpu->m_hi.d[3] && g_cpu->m_s6.d[0] == g_cpu->m_hi.d[2]);  // hi
    WIN_ASSERT_TRUE(g_cpu->m_t6.d[1] == g_cpu->m_lo.d[3] && g_cpu->m_t6.d[0] == g_cpu->m_lo.d[2]);  // lo
  }
  catch (char* msg) { WIN_ASSERT_FAIL(msg); }
}END_TESTF

// madd �m�[�}��
BEGIN_TESTF(r5900_mmi_madd_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 ��Z���ʂ̉���Word�������i�I�[�o�[�t���[�j
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 ��Z���ʂ�����
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 ����Word�̉��Z�ɂăL�����[��������P�[�X
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 ���Word�̌��ʂ�����
    g_cpu->m_s0.d[0] = 0x00000020; g_cpu->m_s1.d[0] = 0x00000015; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0xfffffffe;
    // (hi|lo)=(s7|s6) + s4 * s5 �Q�I�y�����h
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

// madd1 �m�[�}��
BEGIN_TESTF(r5900_mmi_madd1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 ��Z���ʂ̉���Word�������i�I�[�o�[�t���[�j
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 ��Z���ʂ�����
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 ����Word�̉��Z�ɂăL�����[��������P�[�X
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 ���Word�̌��ʂ�����
    g_cpu->m_s0.d[0] = 0x00000020; g_cpu->m_s1.d[0] = 0x00000015; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0xfffffffe;
    // (hi|lo)=(s7|s6) + s4 * s5 �Q�I�y�����h
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

// maddu �m�[�}��
BEGIN_TESTF(r5900_mmi_maddu_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 ��Z���ʂ̉���Word�������i�I�[�o�[�t���[�j
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 ��Z���ʂ�����
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 ����Word�̉��Z�ɂăL�����[��������P�[�X
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 ���Word�̌��ʂ�����
    g_cpu->m_s0.d[0] = 0x80000000; g_cpu->m_s1.d[0] = 0x7fffffff; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0x40000001;
    // (hi|lo)=(s7|s6) + s4 * s5 �Q�I�y�����h
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

// maddu1 �m�[�}��
BEGIN_TESTF(r5900_mmi_maddu1_001, r5900_mmi_fix_01){
  try
  {
    OPEN_ELF("mmi");
    INIT_GPR(0xcccccccc);
    g_cpu->m_hi.d[3] = g_cpu->m_hi.d[2] = g_cpu->m_hi.d[1] = g_cpu->m_hi.d[0] = 0xcccccccc;
    g_cpu->m_lo.d[3] = g_cpu->m_lo.d[2] = g_cpu->m_lo.d[1] = g_cpu->m_lo.d[0] = 0xcccccccc;

    // (a3|a2)=(a3|a2) + a0 * a1 ��Z���ʂ̉���Word�������i�I�[�o�[�t���[�j
    g_cpu->m_a0.d[0] = 0x11000020; g_cpu->m_a1.d[0] = 0x00000045; g_cpu->m_a2.d[0] = 0x00000001; g_cpu->m_a3.d[0] = 0x00000002;
    // (t3|t2)=(t3|t2) + t0 * t1 ��Z���ʂ�����
    g_cpu->m_t0.d[0] = 0xffffffff; g_cpu->m_t1.d[0] = 0x00000045; g_cpu->m_t2.d[0] = 0x00000001; g_cpu->m_t3.d[0] = 0x00000002;
    // (t7|t6)=(t7|t6) + t4 * t5 ����Word�̉��Z�ɂăL�����[��������P�[�X
    g_cpu->m_t4.d[0] = 0x0000ffff; g_cpu->m_t5.d[0] = 0x0001ffff; g_cpu->m_t6.d[0] = 0x00100000; g_cpu->m_t7.d[0] = 0x00000010;
    // (s3|s2)=(s3|s2) + s0 * s1 ���Word�̌��ʂ�����
    g_cpu->m_s0.d[0] = 0x80000000; g_cpu->m_s1.d[0] = 0x7fffffff; g_cpu->m_s2.d[0] = 0x00000001; g_cpu->m_s3.d[0] = 0x40000001;
    // (hi|lo)=(s7|s6) + s4 * s5 �Q�I�y�����h
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
