#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"

#include <assert.h>

void invalid();

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(10) |  0(6)  |
// |  inst  |   rs   |   rt   |   rd   |   fmt  |  inst  |
// +--------+-----------------+--------+--------+--------+
#define _rs  (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt  (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _rd  (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)
#define _fmt (u8)((g_cpu->m_recompiling_opcode >> 6)  & 0x1f)

void madd() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  imul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  // ここでキャリーが立ったら上位ビットに繰り上がる
  add32_reg_mem(_eax, (u32)&g_cpu->m_lo.d[0]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[1], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  adc32_reg_mem(_eax, (u32)&g_cpu->m_hi.d[0]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[1], _edx);
}

void madd1() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  imul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  // ここでキャリーが立ったら上位ビットに繰り上がる
  add32_reg_mem(_eax, (u32)&g_cpu->m_lo.d[2]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[3], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  adc32_reg_mem(_eax, (u32)&g_cpu->m_hi.d[2]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[3], _edx);
}

void maddu() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  // ここでキャリーが立ったら上位ビットに繰り上がる
  add32_reg_mem(_eax, (u32)&g_cpu->m_lo.d[0]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[1], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  adc32_reg_mem(_eax, (u32)&g_cpu->m_hi.d[0]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[1], _edx);
}

void maddu1() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  // ここでキャリーが立ったら上位ビットに繰り上がる
  add32_reg_mem(_eax, (u32)&g_cpu->m_lo.d[2]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[3], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  adc32_reg_mem(_eax, (u32)&g_cpu->m_hi.d[2]);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[3], _edx);
}

void plzcw() {
  invalid();
}

void mfhi1() {
  mov32_eax_mem((u32)&g_cpu->m_hi.d[2]);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem((u32)&g_cpu->m_hi.d[3]);
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void mthi1() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_hi.d[2]);
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax((u32)&g_cpu->m_hi.d[3]);
}

void mflo1() {
  mov32_eax_mem((u32)&g_cpu->m_lo.d[2]);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem((u32)&g_cpu->m_lo.d[3]);
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void mtlo1() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_lo.d[2]);
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax((u32)&g_cpu->m_lo.d[3]);
}

void mult1() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  imul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[3], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[3], _edx);
}

void multu1() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[3], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[3], _edx);
}

void div1() {
  // 整数オーバーフローは発生しないはず
  enum { _end };

  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  tst32_reg_reg(_ecx, _ecx);
  jz(_end);

  mov32_reg_mem(_eax, _GPR_D(_rs, 0));
  cdq();
  idiv_reg(_ecx);
  mov32_reg_reg(_ecx, _edx);
  // 商をloに格納
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[3], _edx);

  mov32_reg_reg(_eax, _ecx);
  // 余をhiに格納
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[3], _edx);
LABEL_DEF(_end);
}

void divu1() {
  // 整数オーバーフローは発生しないはず
  enum { _1div, _end };

  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  tst32_reg_reg(_ecx, _ecx);
  jz(_end);

  mov32_reg_mem(_eax, _GPR_D(_rs, 0));
  xor32_reg_reg(_edx, _edx);
  div_reg(_ecx);
  mov32_reg_reg(_ecx, _edx);
  // 商をloに格納
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[3], _edx);

  mov32_reg_reg(_eax, _ecx);
  // 余をhiに格納
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[2], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[3], _edx);
LABEL_DEF(_end);
}

void pmfhl() {
  invalid();
}

void pmthl() {
  invalid();
}

void psllh() {
  invalid();
}

void psrlh() {
  invalid();
}

void psrah() {
  invalid();
}

void psllw() {
  invalid();
}

void psrlw() {
  invalid();
}

void psraw() {
  invalid();
}

#undef _rs
#undef _rt
#undef _rd
#undef _fmt

void mmi() {
  void mmi0();
  void mmi1();
  void mmi2();
  void mmi3();
  void (*TABLE[0x40])() = {
    madd,    maddu,   invalid, invalid, plzcw,   invalid, invalid, invalid,
    *mmi0,   *mmi2,   invalid, invalid, invalid, invalid, invalid, invalid,
    mfhi1,   mthi1,   mflo1,   mtlo1,   invalid, invalid, invalid, invalid,
    mult1,   multu1,  div1,    divu1,   invalid, invalid, invalid, invalid,
    madd1,   maddu1,  invalid, invalid, invalid, invalid, invalid, invalid,
    *mmi1,   *mmi3,   invalid, invalid, invalid, invalid, invalid, invalid,
    pmfhl,   pmthl,   invalid, invalid, psllh,   invalid, psrlh,   psrah,
    invalid, invalid, invalid, invalid, psllw,   invalid, psrlw,   psraw,
  };
  TABLE[g_cpu->m_recompiling_opcode & 0x3f]();
}
