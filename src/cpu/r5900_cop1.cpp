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
  x87ST_PE  = 0x0020, // 不正確結果(精度)

  x87ST_UE  = 0x0010, // アンダーフロー
  x87ST_OE  = 0x0008, // オーバーフロー
  x87ST_ZE  = 0x0004, // 0除算
  x87ST_DE  = 0x0002, // デノーマルオペランド
  x87ST_IE  = 0x0001, // 無効演算
};

// X  =      0B 無限大制御
// RC =     00B 直近値への丸め（デフォルト）
// PC =     00B 単精度
// EX = 111111B 例外マスク
const u16 FPU_CW_ROUND = 0x003f;

// X  =      0B 無限大制御
// RC =     11B ゼロ方向への丸め（この場合オーバーフローは自動的に±FLT_MAXに
//              クランプされる。結果値によりオーバー／アンダーフロー判定する
//              のでこれは基本使用しない）但し、cvt_w_sでは切り捨てで
//              少数→整数変換を行う必要があるので一時的にこのモードに変更する
// PC =     00B 単精度
// EX = 111111B 例外マスク
const u16 FPU_CW_TRUNC = 0x0c3f;

// 予期しないアンダーフロー例外（単精度同士での加減算など）を処理しない
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
  // fcr0 or fcr31以外の時、結果は不定
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
  // fcr0 or fcr31以外の時、結果は不定
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
  // 分岐しない場合、次の遅延スロットの命令を実行しない
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
  // 分岐しない場合、次の遅延スロットの命令を実行しない
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
  /* 関数内で複数回呼んでもenum値が重複しないように注意！！ */             \
  enum { _skip_ue##_idx = _idx * 8, _end##_idx };                          \
  fnclex(); /* ステータスワードのクリア*/                                  \
  fldcw_mem((u32)&FPU_CW_ROUND);                                           \
  /* Causeフラグをクリア */                                                \
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
  jz(_end##_idx);  /* ゼロであれば例外のチェックはしない */                \
                                                                           \
  /* アンダーフロー */                                                     \
  cmp32_eax_imm32(COP1::FLT_MIN_P);                                        \
  jge(_skip_ue##_idx);                                                     \
  /* 指数部が全0で仮数部が非0の場合±0に修正する。非正規化数がこれに該当 */ \
  and32_mem_imm32(_fdaddr, COP1::FLT_SIGN_MASK);                           \
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31,                                 \
                 COP1::FCR31_U | COP1::FCR31_SU);                          \
LABEL_DEF(_skip_ue##_idx);                                                 \
  /* オーバーフロー */                                                     \
  cmp32_eax_imm32(COP1::FLT_MAX_P);                                        \
  jle(_end##_idx);                                                         \
  /* 指数ビットがすべて1の場合±FLT_MAXに修正する。∞かNaNがこれに該当 */    \
  /* FLT_MAX で符号ビット以外を置き換えることで+/-fmaxにクランプする */    \
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
  fnclex();  // ステータスワードのクリア
  fldcw_mem((u32)&FPU_CW_ROUND);

  // Causeフラグをクリア
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31,
                  ~(COP1::FCR31_D | COP1::FCR31_I));

  fpu_begin();
  fld_relm(_ft * 4);
  fld_relm(_fs * 4);
  fdiv_st0_stx(1);
  fstp_relm_st0(_fd * 4);
  fnstsw_ax();

  // ゼロ除算
  tst8_eax_imm8(x87ST_ZE);
  jz(_skip_ze);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_D | COP1::FCR31_SD);
  // 無効演算とは異なり符号も正常だが、念のためソースオペランドから符号を算出
  mov32_eax_mem(_FPR_D(_fs));
  xor32_reg_mem(_eax, _FPR_D(_ft));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
  jmp(_end);
LABEL_DEF(_skip_ze);

  // 無効演算
  tst8_eax_imm8(x87ST_IE);
  jz(_skip_ie);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_I | COP1::FCR31_SI);
  // IEEE754ではNaNの符号までは規定していない？±0/±0の符号が常に負数になる
  // 仕方ないのでソースオペランドから符号を算出
  mov32_eax_mem(_FPR_D(_fs));
  xor32_reg_mem(_eax, _FPR_D(_ft));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
  jmp(_end);
LABEL_DEF(_skip_ie);

  mov32_eax_mem(_FPR_D(_fd));  // x86レジスタに結果を格納しチェック

  // x87では元々被除数がFLT_MINの場合、
  // DENが結果になってもアンダーフロー例外が発生しないらしい。
  // 最初statusフラグでアンダーフローを検出しようと思ったが無理だった。
  and32_eax_imm32(0x7fffffff);
  jz(_end); // ゼロであれば例外のチェックはしない

  // アンダーフロー（フラグは操作しない）
  cmp32_eax_imm32(COP1::FLT_MIN_P);
  jge(_skip_ue);
  // 指数部が全0で仮数部が非0の場合±0に修正する。非正規化数がこれに該当
  and32_mem_imm32(_FPR_D(_fd), COP1::FLT_SIGN_MASK);
LABEL_DEF(_skip_ue);

  // オーバーフロー（フラグは操作しない）
  cmp32_eax_imm32(COP1::FLT_MAX_P);
  jle(_end);
  // 指数ビットがすべて1の場合±FLT_MAXに修正する。∞かNaNがこれに該当
  // FLT_MAX で符号ビット以外を置き換えることで+/-fmaxにクランプする
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
  fnclex(); // ステータスワードのクリア
  fldcw_mem((u32)&FPU_CW_ROUND);

  // Causeフラグをクリア
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_D | COP1::FCR31_I));

  fpu_begin();
  fld_relm(_ft * 4);
  ftest_st0(); // st0と0.0を比較した結果を条件コードフラグにセットする
  fnstsw_ax();
  and32_eax_imm32(x87ST_C0 | x87ST_C2 | x87ST_C3);

  // 0.0の平方根
  cmp32_eax_imm32(x87ST_C3); // [ft] == 0.0
  jz(_store); // 符号を維持する。要するにこのままftをfdにストアする
  
  // 負数の平方根
  cmp32_eax_imm32(x87ST_C0); // [ft] < 0.0
  jnz(_notreg);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_I | COP1::FCR31_SI);
  fabs_st0(); // 負の場合は絶対値にして平方根を求める
LABEL_DEF(_notreg);

  fsqrt_st0();

LABEL_DEF(_store);
  fstp_relm_st0(_fd * 4);
  fpu_end();
}

void rsqrt_s() {
  enum { _notzero, _notreg, _skip_ue, _end };
  fnclex(); // ステータスワードのクリア
  fldcw_mem((u32)&FPU_CW_ROUND);

  // Causeフラグをクリア
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_D | COP1::FCR31_I));

  fpu_begin();
  fld_relm(_ft * 4);
  ftest_st0(); // st0と0.0を比較した結果を条件コードフラグにセットする
  fnstsw_ax();
  and32_eax_imm32(x87ST_C0 | x87ST_C2 | x87ST_C3);

  // 0除算＆0/0の無効演算(±0.0の平方根)
  cmp32_eax_imm32(x87ST_C3); // [ft] == 0.0
  jnz(_notzero); // 符号を考慮しMAX値をfdにストアする
  mov32_eax_mem(_FPR_D(_fs));
  xor32_reg_mem(_eax, _FPR_D(_ft));
  and32_eax_imm32(COP1::FLT_SIGN_MASK);
  or32_eax_imm32(COP1::FLT_MAX_P);
  mov32_mem_eax(_FPR_D(_fd));
  jmp(_end);
LABEL_DEF(_notzero);

  // 負数の平方根
  cmp32_eax_imm32(x87ST_C0); // [ft] < 0.0
  jnz(_notreg);
  or32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, COP1::FCR31_I | COP1::FCR31_SI);
  fabs_st0(); // 負の場合は絶対値にして平方根を求める
LABEL_DEF(_notreg);

  fsqrt_st0();
  fld_relm(_fs * 4);
  fdiv_st0_stx(1);
  fstp_relm_st0(_fd * 4);

  mov32_eax_mem(_FPR_D(_fd)); // x86レジスタに結果を格納しチェック

  // x87では元々被除数がFLT_MINの場合、
  // DENが結果になってもアンダーフロー例外が発生しないらしい。
  // 最初statusフラグでアンダーフローを検出しようと思ったが無理だった。
  and32_eax_imm32(0x7fffffff);
  jz(_end); // ゼロであれば例外のチェックはしない

  // アンダーフロー（フラグは操作しない）
  cmp32_eax_imm32(COP1::FLT_MIN_P);
  jge(_skip_ue);
  // 指数部が全0で仮数部が非0の場合±0に修正する。非正規化数がこれに該当
  and32_mem_imm32(_FPR_D(_fd), COP1::FLT_SIGN_MASK);
LABEL_DEF(_skip_ue);

  // オーバーフロー（フラグは操作しない）
  cmp32_eax_imm32(COP1::FLT_MAX_P);
  jle(_end);
  // 指数ビットがすべて1の場合±FLT_MAXに修正する。∞かNaNがこれに該当
  // FLT_MAX で符号ビット以外を置き換えることで+/-fmaxにクランプする
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
  and32_eax_imm32(0x7fffffff);  // 符号ビット消去
  mov32_mem_eax(_FPR_D(_fd));
  // Causeフラグをクリア
  and32_mem_imm32((u32)&g_cpu->m_fpu.fcr31, ~(COP1::FCR31_O | COP1::FCR31_U));
}

void mov_s() {
  mov32_eax_mem(_FPR_D(_fs));
  mov32_mem_eax(_FPR_D(_fd));
}

void neg_s() {
  mov32_eax_mem(_FPR_D(_fs));
  xor32_eax_imm32(0x80000000);  // 符号ビット反転
  mov32_mem_eax(_FPR_D(_fd));
  // Causeフラグをクリア
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
  // orの結果が0x80000000であれば0.0の符号違いなので一致判定
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
  fldcw_mem((u32)&FPU_CW_TRUNC); // 切り捨てモードへ切り替え

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
