#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"

#include <assert.h>

void invalid();

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(10) |  0(6)  |
// |  inst  |   rs   |   rt   |   rd   |  inst  |  inst  |
// +--------+-----------------+--------+--------+--------+
#define _rs  (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt  (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _rd  (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)

void pmadduw() {
#define macro(_idx)                                   \
  mov32_eax_mem(_GPR_D(_rs, _idx));                   \
  mul_mem(_GPR_D(_rt, _idx));                         \
  mov32_reg_mem(_ebx, (u32)&g_cpu->m_lo.d[_idx]);     \
  mov32_reg_mem(_ecx, (u32)&g_cpu->m_hi.d[_idx]);     \
  add32_reg_reg(_eax, _ebx);                          \
  adc32_reg_reg(_ecx, _edx);                          \
  cdq();                                              \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx], _eax);     \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx+1], _edx);   \
  mov32_mem_reg(_GPR_D(_rd, _idx), _eax);             \
  mov32_reg_reg(_eax, _ecx);                          \
  cdq();                                              \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx], _eax);     \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx + 1], _edx); \
  mov32_mem_reg(_GPR_D(_rd, _idx+1), _eax);

  macro(0);  // 63-0
  macro(2);  // 127-64

#undef macro
}

void psravw() {
  // TODO
}

void pmthi() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi.x);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pmtlo() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo.x);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pinteh() {
  xor32_reg_reg(_eax, _eax);
  dec16(_eax);
  pinsrw_xmm_reg_imm8(_xmm0, _eax, 0);
  pshufd_reg_reg(_xmm0, _xmm0, 0, 0, 0, 0);
  movdqa_reg_reg(_xmm1, _xmm0);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  pand_reg_rmem(_xmm0, _eax);
  pslldq_reg_imm(_xmm0, 2);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pand_reg_rmem(_xmm1, _eax);

  por_reg_reg(_xmm0, _xmm1);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pmultuw() {
#define macro(_idx)                                   \
  mov32_eax_mem(_GPR_D(_rs, _idx));                   \
  mul_mem(_GPR_D(_rt, _idx));                         \
  mov32_reg_reg(_ecx, _edx);                          \
  cdq();                                              \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx], _eax);     \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx + 1], _edx); \
  if (_rd) mov32_mem_reg(_GPR_D(_rd, _idx), _eax);    \
  mov32_reg_reg(_eax, _ecx);                          \
  cdq();                                              \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx], _eax);     \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx + 1], _edx); \
  if (_rd) mov32_mem_reg(_GPR_D(_rd, _idx + 1), _eax);

  macro(0);  // 63-0
  macro(2);  // 127-64

#undef macro
}

void pdivuw() {
  enum { _end1, _end2 };

#define macro(_idx, _endlbl)                          \
  mov32_reg_mem(_ecx, _GPR_D(_rt, _idx));             \
  tst32_reg_reg(_ecx, _ecx);                          \
  jz(_endlbl);                                        \
  xor32_reg_reg(_edx, _edx);                          \
  mov32_eax_mem(_GPR_D(_rs, _idx));                   \
  div_reg(_ecx);                                      \
  mov32_reg_reg(_ecx, _edx);                          \
  cdq();                                              \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx], _eax);     \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx + 1], _edx); \
  mov32_reg_reg(_eax, _ecx);                          \
  cdq();                                              \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx], _eax);     \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx + 1], _edx);

  macro(0, _end1);  // 63-0
LABEL_DEF(_end1);
  macro(2, _end2);  // 127-64
LABEL_DEF(_end2);

#undef macro
}

void pcpyud() {
  mov32_eax_mem(_GPR_D(_rt, 2));
  mov32_mem_eax(_GPR_D(_rd, 2));
  mov32_eax_mem(_GPR_D(_rt, 3));
  mov32_mem_eax(_GPR_D(_rd, 3));

  mov32_eax_mem(_GPR_D(_rs, 2));
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem(_GPR_D(_rs, 3));
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void por() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  por_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pnor() {
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  or32_reg_mem(_eax, _GPR_D(_rt, 0));
  not32(_eax);
  mov32_mem_reg(_GPR_D(_rd, 0), _eax);
  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  or32_reg_mem(_eax, _GPR_D(_rt, 1));
  not32(_eax);
  mov32_mem_reg(_GPR_D(_rd, 1), _eax);
  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  or32_reg_mem(_eax, _GPR_D(_rt, 2));
  not32(_eax);
  mov32_mem_reg(_GPR_D(_rd, 2), _eax);
  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  or32_reg_mem(_eax, _GPR_D(_rt, 3));
  not32(_eax);
  mov32_mem_reg(_GPR_D(_rd, 3), _eax);
}

void pexch() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);

  pshufhw_reg_reg(_xmm0, _xmm0, 0, 2, 1, 3);
  pshuflw_reg_reg(_xmm0, _xmm0, 0, 2, 1, 3);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pcpyh() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_edx, _eax);
  shl32_reg_imm8(_eax, 16);
  or16_reg_reg(_ax, _dx);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_mem_eax(_GPR_D(_rd, 1));

  mov32_eax_mem(_GPR_D(_rt, 2));
  mov32_reg_reg(_edx, _eax);
  shl32_reg_imm8(_eax, 16);
  or16_reg_reg(_ax, _dx);
  mov32_mem_eax(_GPR_D(_rd, 2));
  mov32_mem_eax(_GPR_D(_rd, 3));
}

void pexcw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);

  pshufd_reg_reg(_xmm0, _xmm0, 0, 2, 1, 3);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

#undef _rs
#undef _rt
#undef _rd

void mmi3() {
  void (*TABLE[0x20])() = {
    pmadduw, invalid, invalid, psravw,
    invalid, invalid, invalid, invalid,
    pmthi,   pmtlo,   pinteh,  invalid,
    pmultuw, pdivuw,  pcpyud,  invalid,
    invalid, invalid, por,     pnor,
    invalid, invalid, invalid, invalid,
    invalid, invalid, pexch,   pcpyh,
    invalid, invalid, pexcw,   invalid,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 6) & 0x1f]();
}
