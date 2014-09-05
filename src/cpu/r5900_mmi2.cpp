#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"

void invalid();

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(10) |  0(6)  |
// |  inst  |   rs   |   rt   |   rd   |  inst  |  inst  |
// +--------+-----------------+--------+--------+--------+
#define _rs  (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt  (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _rd  (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)

void pmaddw() {
#define macro(_idx)                                 \
  mov32_eax_mem(_GPR_D(_rs, _idx));                 \
  imul_mem(_GPR_D(_rt, _idx));                      \
  mov32_reg_mem(_ebx, (u32)&g_cpu->m_lo.d[_idx]);   \
  mov32_reg_mem(_ecx, (u32)&g_cpu->m_hi.d[_idx]);   \
  add32_reg_reg(_eax, _ebx);                        \
  adc32_reg_reg(_ecx, _edx);                        \
  cdq();                                            \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx], _eax);   \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx+1], _edx); \
  mov32_mem_reg(_GPR_D(_rd, _idx), _eax);           \
  mov32_reg_reg(_eax, _ecx);                        \
  cdq();                                            \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx], _eax);   \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx+1], _edx); \
  mov32_mem_reg(_GPR_D(_rd, _idx+1), _eax);

  macro(0);  // 63-0
  macro(2);  // 127-64

#undef macro
}

void pmaddh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm1, _eax);

  movdqa_reg_reg(_xmm2, _xmm0);
  pmulhw_reg_reg(_xmm2, _xmm1);
  pmullw_reg_reg(_xmm0, _xmm1);

  pshufd_reg_reg(_xmm0, _xmm0, 0, 2, 1, 3);
  pshufd_reg_reg(_xmm2, _xmm2, 0, 2, 1, 3);

  movdqa_reg_reg(_xmm1, _xmm0);
  punpckhwd_reg_reg(_xmm1, _xmm2);
  punpcklwd_reg_reg(_xmm0, _xmm2);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo);
  paddd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi);
  paddd_reg_rmem(_xmm1, _eax);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo);
  movdqa_rmem_reg(_eax, _xmm0);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi);
  movdqa_rmem_reg(_eax, _xmm1);
  if (_rd != 0) {
    mov32_eax_mem((u32)&g_cpu->m_lo.d[0]);
    mov32_mem_eax(_GPR_D(_rd, 0));
    mov32_eax_mem((u32)&g_cpu->m_hi.d[0]);
    mov32_mem_eax(_GPR_D(_rd, 1));
    mov32_eax_mem((u32)&g_cpu->m_lo.d[2]);
    mov32_mem_eax(_GPR_D(_rd, 2));
    mov32_eax_mem((u32)&g_cpu->m_hi.d[2]);
    mov32_mem_eax(_GPR_D(_rd, 3));
  }
}

void pmsubw() {
#define macro(_idx)                                 \
  mov32_eax_mem(_GPR_D(_rs, _idx));                 \
  imul_mem(_GPR_D(_rt, _idx));                      \
  mov32_reg_mem(_ebx, (u32)&g_cpu->m_lo.d[_idx]);   \
  mov32_reg_mem(_ecx, (u32)&g_cpu->m_hi.d[_idx]);   \
  sub32_reg_reg(_ebx, _eax);                        \
  sbb32_reg_reg(_ecx, _edx);                        \
  mov32_reg_reg(_eax, _ebx);                        \
  cdq();                                            \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx], _eax);   \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx+1], _edx); \
  mov32_mem_reg(_GPR_D(_rd, _idx), _eax);           \
  mov32_reg_reg(_eax, _ecx);                        \
  cdq();                                            \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx], _eax);   \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx+1], _edx); \
  mov32_mem_reg(_GPR_D(_rd, _idx+1), _eax);

  macro(0);  // 63-0
  macro(2);  // 127-64

#undef macro
}

void pmsubh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm1, _eax);

  movdqa_reg_reg(_xmm2, _xmm0);
  pmulhw_reg_reg(_xmm2, _xmm1);
  pmullw_reg_reg(_xmm0, _xmm1);

  pshufd_reg_reg(_xmm0, _xmm0, 0, 2, 1, 3);
  pshufd_reg_reg(_xmm2, _xmm2, 0, 2, 1, 3);

  movdqa_reg_reg(_xmm1, _xmm0);
  punpckhwd_reg_reg(_xmm1, _xmm2);
  punpcklwd_reg_reg(_xmm0, _xmm2);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo);
  movdqa_reg_rmem(_xmm2, _eax);
  psubd_reg_reg(_xmm2, _xmm0);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi);
  movdqa_reg_rmem(_xmm3, _eax);
  psubd_reg_reg(_xmm3, _xmm1);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo);
  movdqa_rmem_reg(_eax, _xmm2);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi);
  movdqa_rmem_reg(_eax, _xmm3);
  if (_rd != 0) {
    mov32_eax_mem((u32)&g_cpu->m_lo.d[0]);
    mov32_mem_eax(_GPR_D(_rd, 0));
    mov32_eax_mem((u32)&g_cpu->m_hi.d[0]);
    mov32_mem_eax(_GPR_D(_rd, 1));
    mov32_eax_mem((u32)&g_cpu->m_lo.d[2]);
    mov32_mem_eax(_GPR_D(_rd, 2));
    mov32_eax_mem((u32)&g_cpu->m_hi.d[2]);
    mov32_mem_eax(_GPR_D(_rd, 3));
  }
}

void psllvw() {
}

void psrlvw() {
}

void pmultw() {
#define macro(_idx)                                 \
  mov32_eax_mem(_GPR_D(_rs, _idx));                 \
  imul_mem(_GPR_D(_rt, _idx));                      \
  mov32_reg_reg(_ecx, _edx);                        \
  cdq();                                            \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx], _eax);   \
  mov32_mem_reg((u32)&g_cpu->m_lo.d[_idx+1], _edx); \
  if (_rd) mov32_mem_reg(_GPR_D(_rd, _idx), _eax);  \
  mov32_reg_reg(_eax, _ecx);                        \
  cdq();                                            \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx], _eax);   \
  mov32_mem_reg((u32)&g_cpu->m_hi.d[_idx+1], _edx); \
  if (_rd) mov32_mem_reg(_GPR_D(_rd, _idx+1), _eax);

  macro(0);  // 63-0
  macro(2);  // 127-64

#undef macro
}

void pmulth() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm1, _eax);

  movdqa_reg_reg(_xmm2, _xmm0);
  pmulhw_reg_reg(_xmm2, _xmm1);
  pmullw_reg_reg(_xmm0, _xmm1);

  pshufd_reg_reg(_xmm0, _xmm0, 0, 2, 1, 3);
  pshufd_reg_reg(_xmm2, _xmm2, 0, 2, 1, 3);

  movdqa_reg_reg(_xmm1, _xmm0);
  punpckhwd_reg_reg(_xmm1, _xmm2);
  punpcklwd_reg_reg(_xmm0, _xmm2);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo);
  movdqa_rmem_reg(_eax, _xmm0);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi);
  movdqa_rmem_reg(_eax, _xmm1);
  if (_rd != 0) {
    mov32_eax_mem((u32)&g_cpu->m_lo.d[0]);
    mov32_mem_eax(_GPR_D(_rd, 0));
    mov32_eax_mem((u32)&g_cpu->m_hi.d[0]);
    mov32_mem_eax(_GPR_D(_rd, 1));
    mov32_eax_mem((u32)&g_cpu->m_lo.d[2]);
    mov32_mem_eax(_GPR_D(_rd, 2));
    mov32_eax_mem((u32)&g_cpu->m_hi.d[2]);
    mov32_mem_eax(_GPR_D(_rd, 3));
  }
}

void pdivw() {
}

void pdivbw() {
}

void pmfhi() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_hi.x);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pmflo() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_lo.x);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pinth() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  pslldq_reg_imm(_xmm0, 8);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpckhwd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pcpyld() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem(_GPR_D(_rt, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));

  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax(_GPR_D(_rd, 2));
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax(_GPR_D(_rd, 3));
}

void phmadh() {
}

void phmsbh() {
}

void pand() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pand_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pxor() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pxor_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pexeh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);

  pshufhw_reg_reg(_xmm0, _xmm0, 2, 1, 0, 3);
  pshuflw_reg_reg(_xmm0, _xmm0, 2, 1, 0, 3);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void prevh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);

  pshufhw_reg_reg(_xmm0, _xmm0, 3, 2, 1, 0);
  pshuflw_reg_reg(_xmm0, _xmm0, 3, 2, 1, 0);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pexew() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);

  pshufd_reg_reg(_xmm0, _xmm0, 2, 1, 0, 3);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void prot3w() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);

  pshufd_reg_reg(_xmm0, _xmm0, 1, 2, 0, 3);

  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

#undef _rs
#undef _rt
#undef _rd

void mmi2() {
  void (*TABLE[0x20])() = {
    pmaddw,  invalid, psllvw,  psrlvw,
    pmsubw,  invalid, invalid, invalid,
    pmfhi,   pmflo,   pinth,   invalid,
    pmultw,  pdivw,   pcpyld,  invalid,
    pmaddh,  phmadh,  pand,    pxor,
    pmsubh,  phmsbh,  invalid, invalid,
    invalid, invalid, pexeh,   prevh,
    pmulth,  pdivbw,  pexew,   prot3w,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 6) & 0x1f]();
}
