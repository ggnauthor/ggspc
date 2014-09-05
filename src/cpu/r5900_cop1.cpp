#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif

#include <assert.h>

enum { // ee relm offset
  RELM_ACC   = 32 * 4,
  RELM_FCR0  = 33 * 4,
  RELM_FCR31 = 34 * 4,
  RELM_V_TMP = 35 * 4,
};

enum { // x87 control word mask
  x87CW_X   = 0x1000,
  x87CW_RC  = 0x0c00,
  x87CW_PC  = 0x0300,

  x87CW_PM  = 0x0020,
  x87CW_UM  = 0x0010,
  x87CW_OM  = 0x0008,
  x87CW_ZM  = 0x0004,
  x87CW_DM  = 0x0002,
  x87CW_IM  = 0x0001,
};

enum { // x87 status flag mask
  x87ST_B   = 0x8000,
  x87ST_C3  = 0x4000,
  x87ST_TOP = 0x3800,
  x87ST_C2  = 0x0400,
  x87ST_C1  = 0x0200,
  x87ST_C0  = 0x0100,
  x87ST_ES  = 0x0080,
  x87ST_SF  = 0x0040,
  x87ST_PE  = 0x0020, // �s���m����(���x)

  x87ST_UE  = 0x0010, // �A���_�[�t���[
  x87ST_OE  = 0x0008, // �I�[�o�[�t���[
  x87ST_ZE  = 0x0004, // 0���Z
  x87ST_DE  = 0x0002, // �f�m�[�}���I�y�����h
  x87ST_IE  = 0x0001, // �������Z
};

// X  =      0B �����吧��
// RC =     00B ���ߒl�ւ̊ۂ߁i�f�t�H���g�j
// PC =     00B �P���x
// EX = 111111B ��O�}�X�N
const u16 FPU_CW_ROUND = 0x003f;

// X  =      0B �����吧��
// RC =     11B �[�������ւ̊ۂ߁i���̏ꍇ�I�[�o�[�t���[�͎����I�Ɂ}FLT_MAX��
//              �N�����v�����B���ʒl�ɂ��I�[�o�[�^�A���_�[�t���[���肷��
//              �̂ł���͊�{�g�p���Ȃ��j�A���Acvt_w_s�ł͐؂�̂Ă�
//              �����������ϊ����s���K�v������̂ňꎞ�I�ɂ��̃��[�h�ɕύX����
// PC =     00B �P���x
// EX = 111111B ��O�}�X�N
const u16 FPU_CW_TRUNC = 0x0c3f;

// �\�����Ȃ��A���_�[�t���[��O�i�P���x���m�ł̉����Z�Ȃǁj���������Ȃ�
#define HANDLE_UNEXPECTED_UNDERFLOW FALSE

void invalid();

//-----------------------------------------------------------------------------
// global

//-----------------------------------------------------------------------------
// function

// cop1
// +--------+-----------------+--------+-----------------+
// |  26(6) |  21(5) |  16(5) |  11(5) |      0(16)      |
// |  inst  |  inst  |   rt   |   fs   |                 |
// +--------+-----------------+--------+-----------------+
#define _rt  (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _fs  (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)

void mfc1() {
  mov32_eax_mem(_FPR_D(_fs));
  cdq();
  mov32_mem_eax(_GPR_D(_rt, 0));
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
  mov32_mem_reg(_GPR_D(_rt, 2), _edx);
  mov32_mem_reg(_GPR_D(_rt, 3), _edx);
}

void cfc1() {
  // fcr0 or fcr31�ȊO�̎��A���ʂ͕s��
  if (_fs == 0x00)      { mov32_eax_mem((u32)&g_cpu->m_fpu.fcr0.d); }
  else if (_fs == 0x1f) { mov32_eax_mem((u32)&g_cpu->m_fpu.fcr31); }
  else return;

  cdq();
  mov32_mem_eax(_GPR_D(_rt, 0));
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
  mov32_mem_reg(_GPR_D(_rt, 2), _edx);
  mov32_mem_reg(_GPR_D(_rt, 3), _edx);
}

void mtc1() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_mem_eax(_FPR_D(_fs));
}

void ctc1() {
  // fcr0 or fcr31�ȊO�̎��A���ʂ͕s��
  if (_fs == 0x00) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    mov32_mem_eax((u32)&g_cpu->m_fpu.fcr0.d);
  } else if (_fs == 0x1f) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    mov32_mem_eax((u32)&g_cpu->m_fpu.fcr31);
  }
}

#undef _rt
#undef _fs

// bc1
// +--------+-----------------+--------------------------+
// |  26(6) |  21(5) |  16(5) |           0(16)          |
// |  inst  |  inst  |  inst  |           offset         |
// +--------+-----------------+--------------------------+
#define _offset  (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void bc1f() {
  enum { _end };
  tst32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jnz(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bc1t() {
  enum { _end };
  tst32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jz(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bc1fl() {
  enum { _notjump, _end };

  tst32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jnz(_notjump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

  jmp(_end);
LABEL_DEF(_notjump);
  // ���򂵂Ȃ��ꍇ�A���̒x���X���b�g�̖��߂����s���Ȃ�
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
LABEL_DEF(_end);
}

void bc1tl() {
  enum { _notjump, _end };

  tst32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jz(_notjump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

  jmp(_end);
LABEL_DEF(_notjump);
  // ���򂵂Ȃ��ꍇ�A���̒x���X���b�g�̖��߂����s���Ȃ�
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
LABEL_DEF(_end);
}

#undef _offset

// c/w
// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(10) |  0(6)  |
// |  inst  |  inst  |   ft   |   fs   |   fd   |  inst  |
// +--------+-----------------+--------+--------+--------+
#define _ft  (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _fs  (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)
#define _fd  (u8)((g_cpu->m_recompiling_opcode >>  6) & 0x1f)

#define _calc(_idx, _fdofs, _fdaddr, _fsofs, _ftofs, _method, _clearflag)  \
  /* �֐����ŕ�����Ă�ł�enum�l���d�����Ȃ��悤�ɒ��ӁI�I */             \
  enum { _skip_ue##_idx = _idx * 8, _end##_idx };                          \
  fnclex(); /* �X�e�[�^�X���[�h�̃N���A*/                                  \
  fldcw_mem((u32)&FPU_CW_ROUND);                                           \
  /* Cause�t���O���N���A */                                                \
  if (_clearflag) {                                                        \
    and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31,                              \
                    ~(COP1::FCR31_O | COP1::FCR31_U));                     \
  }                                                                        \
  fpu_begin();                                                             \
  fld_relm(_ftofs);                                                        \
  fld_relm(_fsofs);                                                        \
  _method(1);                                                              \
  fstp_relm_st0(_fdofs);                                                   \
  mov32_eax_mem(_fdaddr);                                                  \
  and32_eax_imm32(0x7fffffff);                                             \
  jz(_end##_idx);  /* �[���ł���Η�O�̃`�F�b�N�͂��Ȃ� */                \
                                                                           \
  /* �A���_�[�t���[ */                                                     \
  cmp32_eax_imm32(COP1::FLT_MIN_P);                                        \
  jge(_skip_ue##_idx);                                                     \
  /* �w�������S0�ŉ���������0�̏ꍇ�}0�ɏC������B�񐳋K����������ɊY�� */ \
  and32_mem_imm32(_fdaddr, COP1::FLT_SIGN_MASK);                           \
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31,                                 \
                 COP1::FCR31_U | COP1::FCR31_SU);                          \
LABEL_DEF(_skip_ue##_idx);                                                 \
  /* �I�[�o�[�t���[ */                                                     \
  cmp32_eax_imm32(COP1::FLT_MAX_P);                                        \
  jle(_end##_idx);                                                         \
  /* �w���r�b�g�����ׂ�1�̏ꍇ�}FLT_MAX�ɏC������B����NaN������ɊY�� */    \
  /* FLT_MAX �ŕ����r�b�g�ȊO��u�������邱�Ƃ�+/-fmax�ɃN�����v���� */    \
  mov32_eax_mem(_fdaddr);                                                  \
  and32_eax_imm32(COP1::FLT_SIGN_MASK);                                    \
  or32_eax_imm32(COP1::FLT_MAX_P);                                         \
  mov32_mem_eax(_fdaddr);                                                  \
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31,                                 \
                 COP1::FCR31_O | COP1::FCR31_SO);                          \
LABEL_DEF(_end##_idx);                                                     \
  ffree(0);                                                                \
  fpu_end();

void add_s() {
  _calc(0, _fd * 4, _FPR_D(_fd), _fs * 4, _ft * 4, fadd_st0_stx, true);
}

void sub_s() {
  _calc(0, _fd * 4, _FPR_D(_fd), _fs * 4, _ft * 4, fsub_st0_stx, true);
}

void mul_s() {
  _calc(0, _fd * 4, _FPR_D(_fd), _fs * 4, _ft * 4, fmul_st0_stx, true);
}

void div_s() {
  enum { _skip_ze, _skip_ie, _skip_ue, _end };
  fnclex();  // �X�e�[�^�X���[�h�̃N���A
  fldcw_mem((u32)&FPU_CW_ROUND);

  // Cause�t���O���N���A
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31,
                  ~(COP1::FCR31_D | COP1::FCR31_I));

  fpu_begin();
  fld_relm(_ft * 4);
  fld_relm(_fs * 4);
  fdiv_st0_stx(1);
  fstp_relm_st0(_fd * 4);
  fnstsw_ax();

  // �[�����Z
  tst8_eax_imm8(x87ST_ZE);
  jz(_skip_ze);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_D | COP1::FCR31_SD);
  // �������Z�Ƃ͈قȂ蕄�������킾���A�O�̂��߃\�[�X�I�y�����h���畄�����Z�o
  mov32_eax_mem(_FPR_D(_fs));
  xor32_reg_mem(_eax, _FPR_D(_ft));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
  jmp(_end);
LABEL_DEF(_skip_ze);

  // �������Z
  tst8_eax_imm8(x87ST_IE);
  jz(_skip_ie);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_I | COP1::FCR31_SI);
  // IEEE754�ł�NaN�̕����܂ł͋K�肵�Ă��Ȃ��H�}0/�}0�̕�������ɕ����ɂȂ�
  // �d���Ȃ��̂Ń\�[�X�I�y�����h���畄�����Z�o
  mov32_eax_mem(_FPR_D(_fs));
  xor32_reg_mem(_eax, _FPR_D(_ft));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
  jmp(_end);
LABEL_DEF(_skip_ie);

  mov32_eax_mem(_FPR_D(_fd));  // x86���W�X�^�Ɍ��ʂ��i�[���`�F�b�N

  // x87�ł͌��X�폜����FLT_MIN�̏ꍇ�A
  // DEN�����ʂɂȂ��Ă��A���_�[�t���[��O���������Ȃ��炵���B
  // �ŏ�status�t���O�ŃA���_�[�t���[�����o���悤�Ǝv�����������������B
  and32_eax_imm32(0x7fffffff);
  jz(_end); // �[���ł���Η�O�̃`�F�b�N�͂��Ȃ�

  // �A���_�[�t���[�i�t���O�͑��삵�Ȃ��j
  cmp32_eax_imm32(COP1::FLT_MIN_P);
  jge(_skip_ue);
  // �w�������S0�ŉ���������0�̏ꍇ�}0�ɏC������B�񐳋K����������ɊY��
  and32_mem_imm32(_FPR_D(_fd), COP1::FLT_SIGN_MASK);
LABEL_DEF(_skip_ue);

  // �I�[�o�[�t���[�i�t���O�͑��삵�Ȃ��j
  cmp32_eax_imm32(COP1::FLT_MAX_P);
  jle(_end);
  // �w���r�b�g�����ׂ�1�̏ꍇ�}FLT_MAX�ɏC������B����NaN������ɊY��
  // FLT_MAX �ŕ����r�b�g�ȊO��u�������邱�Ƃ�+/-fmax�ɃN�����v����
  mov32_eax_mem(_FPR_D(_fd));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
LABEL_DEF(_end);
  ffree(0);
  fpu_end();
}

void sqrt_s() {
  enum { _notreg, _store };
  fnclex(); // �X�e�[�^�X���[�h�̃N���A
  fldcw_mem((u32)&FPU_CW_ROUND);

  // Cause�t���O���N���A
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_D | COP1::FCR31_I));

  fpu_begin();
  fld_relm(_ft * 4);
  ftest_st0(); // st0��0.0���r�������ʂ������R�[�h�t���O�ɃZ�b�g����
  fnstsw_ax();
  and32_eax_imm32(x87ST_C0 | x87ST_C2 | x87ST_C3);

  // 0.0�̕�����
  cmp32_eax_imm32(x87ST_C3); // [ft] == 0.0
  jz(_store); // �������ێ�����B�v����ɂ��̂܂�ft��fd�ɃX�g�A����
  
  // �����̕�����
  cmp32_eax_imm32(x87ST_C0); // [ft] < 0.0
  jnz(_notreg);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_I | COP1::FCR31_SI);
  fabs_st0(); // ���̏ꍇ�͐�Βl�ɂ��ĕ����������߂�
LABEL_DEF(_notreg);

  fsqrt_st0();

LABEL_DEF(_store);
  fstp_relm_st0(_fd * 4);
  fpu_end();
}

void rsqrt_s() {
  enum { _notzero, _notreg, _skip_ue, _end };
  fnclex(); // �X�e�[�^�X���[�h�̃N���A
  fldcw_mem((u32)&FPU_CW_ROUND);

  // Cause�t���O���N���A
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_D | COP1::FCR31_I));

  fpu_begin();
  fld_relm(_ft * 4);
  ftest_st0(); // st0��0.0���r�������ʂ������R�[�h�t���O�ɃZ�b�g����
  fnstsw_ax();
  and32_eax_imm32(x87ST_C0 | x87ST_C2 | x87ST_C3);

  // 0���Z��0/0�̖������Z(�}0.0�̕�����)
  cmp32_eax_imm32(x87ST_C3); // [ft] == 0.0
  jnz(_notzero); // �������l����MAX�l��fd�ɃX�g�A����
  mov32_eax_mem(_FPR_D(_fs));
  xor32_reg_mem(_eax, _FPR_D(_ft));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
  jmp(_end);
LABEL_DEF(_notzero);

  // �����̕�����
  cmp32_eax_imm32(x87ST_C0); // [ft] < 0.0
  jnz(_notreg);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_I | COP1::FCR31_SI);
  fabs_st0(); // ���̏ꍇ�͐�Βl�ɂ��ĕ����������߂�
LABEL_DEF(_notreg);

  fsqrt_st0();
  fld_relm(_fs * 4);
  fdiv_st0_stx(1);
  fstp_relm_st0(_fd * 4);

  mov32_eax_mem(_FPR_D(_fd)); // x86���W�X�^�Ɍ��ʂ��i�[���`�F�b�N

  // x87�ł͌��X�폜����FLT_MIN�̏ꍇ�A
  // DEN�����ʂɂȂ��Ă��A���_�[�t���[��O���������Ȃ��炵���B
  // �ŏ�status�t���O�ŃA���_�[�t���[�����o���悤�Ǝv�����������������B
  and32_eax_imm32(0x7fffffff);
  jz(_end); // �[���ł���Η�O�̃`�F�b�N�͂��Ȃ�

  // �A���_�[�t���[�i�t���O�͑��삵�Ȃ��j
  cmp32_eax_imm32(COP1::FLT_MIN_P);
  jge(_skip_ue);
  // �w�������S0�ŉ���������0�̏ꍇ�}0�ɏC������B�񐳋K����������ɊY��
  and32_mem_imm32(_FPR_D(_fd), COP1::FLT_SIGN_MASK);
LABEL_DEF(_skip_ue);

  // �I�[�o�[�t���[�i�t���O�͑��삵�Ȃ��j
  cmp32_eax_imm32(COP1::FLT_MAX_P);
  jle(_end);
  // �w���r�b�g�����ׂ�1�̏ꍇ�}FLT_MAX�ɏC������B����NaN������ɊY��
  // FLT_MAX �ŕ����r�b�g�ȊO��u�������邱�Ƃ�+/-fmax�ɃN�����v����
  mov32_eax_mem(_FPR_D(_fd));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
LABEL_DEF(_end);

  ffree(0);
  fpu_end();
}

void abs_s() {
  mov32_eax_mem(_FPR_D(_fs));
  and32_eax_imm32(0x7fffffff);  // �����r�b�g����
  mov32_mem_eax(_FPR_D(_fd));
  // Cause�t���O���N���A
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_O | COP1::FCR31_U));
}

void mov_s() {
  mov32_eax_mem(_FPR_D(_fs));
  mov32_mem_eax(_FPR_D(_fd));
}

void neg_s() {
  mov32_eax_mem(_FPR_D(_fs));
  xor32_eax_imm32(0x80000000);  // �����r�b�g���]
  mov32_mem_eax(_FPR_D(_fd));
  // Cause�t���O���N���A
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_O | COP1::FCR31_U));
}

void adda_s() {
  _calc(0, RELM_ACC, (u32)&g_cpu->m_fpu.acc,
        _fs * 4, _ft * 4, fadd_st0_stx, true);
}

void suba_s() {
  _calc(0, RELM_ACC, (u32)&g_cpu->m_fpu.acc,
        _fs * 4, _ft * 4, fsub_st0_stx, true);
}

void mula_s() {
  _calc(0, RELM_ACC, (u32)&g_cpu->m_fpu.acc,
        _fs * 4, _ft * 4, fmul_st0_stx, true);
}

void madd_s() {
  _calc(0, _fd * 4, _FPR_D(_fd), _fs * 4, _ft * 4, fmul_st0_stx, true);
  _calc(1, _fd * 4, _FPR_D(_fd), RELM_ACC, _fd * 4, fadd_st0_stx, false);
}

void msub_s() {
  _calc(0, _fd * 4, _FPR_D(_fd), _fs * 4, _ft * 4, fmul_st0_stx, true);
  _calc(1, _fd * 4, _FPR_D(_fd), RELM_ACC, _fd * 4, fsub_st0_stx, false);
}

void madda_s() {
  _calc(0, RELM_V_TMP, (u32)&g_cpu->m_fpu.v_tmp,
        _fs * 4, _ft * 4, fmul_st0_stx, true);
  _calc(1, RELM_ACC, (u32)&g_cpu->m_fpu.acc,
        RELM_ACC, RELM_V_TMP, fadd_st0_stx, false);
}

void msuba_s() {
  _calc(0, RELM_V_TMP, (u32)&g_cpu->m_fpu.v_tmp,
        _fs * 4, _ft * 4, fmul_st0_stx, true);
  _calc(1, RELM_ACC, (u32)&g_cpu->m_fpu.acc,
        RELM_ACC, RELM_V_TMP, fsub_st0_stx, false);
}

void max_s() {
  enum { _store_ft };
  fldcw_mem((u32)&FPU_CW_ROUND);
  fpu_begin();

  fld_relm(_ft * 4);
  fld_relm(_fs * 4);
  fcomi_st0_stx(1);
  ja(_store_ft);
  fxch_st0_stx(1);
LABEL_DEF(_store_ft);
  fstp_relm_st0(_fd * 4);
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_O | COP1::FCR31_U));
  ffree(0);
  fpu_end();
}

void min_s() {
enum { _store_ft };
  fldcw_mem((u32)&FPU_CW_ROUND);
  fpu_begin();

  fld_relm(_ft * 4);
  fld_relm(_fs * 4);
  fcomi_st0_stx(1);
  jb(_store_ft);
  fxch_st0_stx(1);
LABEL_DEF(_store_ft);
  fstp_relm_st0(_fd * 4);
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_O | COP1::FCR31_U));
  ffree(0);
  fpu_end();
}

void c_f_s() {
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~COP1::FCR31_C);
}

void c_eq_s() {
  enum { _false1, _false2, _end };
  mov32_eax_mem(_FPR_D(_fs));
  mov32_reg_mem(_ecx, _FPR_D(_ft));
  sub32_reg_reg(_eax, _ecx);
  jnz(_false1);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jmp(_end);

LABEL_DEF(_false1);
  // or�̌��ʂ�0x80000000�ł����0.0�̕����Ⴂ�Ȃ̂ň�v����
  or32_reg_reg(_eax, _ecx);
  cmp32_eax_imm32(0x80000000);
  jnz(_false2);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jmp(_end);

LABEL_DEF(_false2);
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~COP1::FCR31_C);
LABEL_DEF(_end);
}

void c_lt_s() {
  enum { _false, _end };
  fldcw_mem((u32)&FPU_CW_ROUND);
  fpu_begin();

  fld_relm(_ft * 4);
  fld_relm(_fs * 4);
  fcomip_st0_stx(1);
  jae(_false);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jmp(_end);
LABEL_DEF(_false);
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~COP1::FCR31_C);
LABEL_DEF(_end);
  ffree(0);
  fpu_end();
}

void c_le_s() {
  enum { _false, _end };
  fldcw_mem((u32)&FPU_CW_ROUND);
  fpu_begin();

  fld_relm(_ft * 4);
  fld_relm(_fs * 4);
  fcomip_st0_stx(1);
  ja(_false);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_C);
  jmp(_end);
LABEL_DEF(_false);
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~COP1::FCR31_C);
LABEL_DEF(_end);
  ffree(0);
  fpu_end();
}

void cvt_w_s() {
  enum { _clamp, _notneg, _end };
  fldcw_mem((u32)&FPU_CW_TRUNC); // �؂�̂ă��[�h�֐؂�ւ�

  mov32_eax_mem(_FPR_D(_fs));
  mov32_reg_reg(_ecx, _eax);
  and32_eax_imm32(COP1::FLT_EXP_MASK);
  cmp32_eax_imm32(0x9d << 23);
  jg(_clamp);
  fld_mem(_FPR_D(_fs));
  fistp32_mem_st0(_FPR_D(_fd));
  jmp(_end);

LABEL_DEF(_clamp);
  mov32_reg_imm32(_eax, 0x7fffffff);
  tst32_reg_reg(_ecx, _ecx);
  jns(_notneg);
  inc32(_eax);

LABEL_DEF(_notneg);
  mov32_mem_eax(_FPR_D(_fd));
LABEL_DEF(_end);
}

void cvt_s_w() {
  fild32_mem(_FPR_D(_fs));
  fstp_mem_st0(_FPR_D(_fd));
}

#undef _calc
#undef _ft
#undef _fs
#undef _fd

void bc1() {
  static void (*TABLE[0x20])() = {
    bc1f,    bc1t,    bc1fl,   bc1tl,   invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 16) & 0x1f]();
}

void s() {
  static void (*TABLE[0x40])() = {
    add_s,   sub_s,   mul_s,   div_s,   sqrt_s,  abs_s,   mov_s,   neg_s,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, rsqrt_s, invalid,
    adda_s,  suba_s,  mula_s,  invalid, madd_s,  msub_s,  madda_s, msuba_s,
    invalid, invalid, invalid, invalid, cvt_w_s, invalid, invalid, invalid,
    max_s,   min_s,   invalid, invalid, invalid, invalid, invalid, invalid,
    c_f_s,   invalid, c_eq_s,  invalid, c_lt_s,  invalid, c_le_s,  invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
  };
  TABLE[g_cpu->m_recompiling_opcode & 0x3f]();
}

void w() {
  static void (*TABLE[0x40])() = {
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    cvt_s_w, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
  };
  TABLE[g_cpu->m_recompiling_opcode & 0x3f]();
}

void cop1() {
  static void (*TABLE[0x20])() = {
    mfc1,    invalid, cfc1,    invalid, mtc1,    invalid, ctc1,    invalid,
    *bc1,    invalid, invalid, invalid, invalid, invalid, invalid, invalid,
    *s,      invalid, invalid, invalid, *w,      invalid, invalid, invalid,
    invalid, invalid, invalid, invalid, invalid, invalid, invalid, invalid,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 21) & 0x1f]();
}
