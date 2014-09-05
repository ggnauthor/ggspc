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

void padduw() {
  enum { _store0, _store1, _store2, _store3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  add32_reg_mem(_eax, _GPR_D(_rt, 0));
  jnc(_store0);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
  dec32(_eax);
LABEL_DEF(_store0);
  mov32_mem_reg(_GPR_D(_rd, 0), _eax);

  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  add32_reg_mem(_eax, _GPR_D(_rt, 1));
  jnc(_store1);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
  dec32(_eax);
LABEL_DEF(_store1);
  mov32_mem_reg(_GPR_D(_rd, 1), _eax);

  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  add32_reg_mem(_eax, _GPR_D(_rt, 2));
  jnc(_store2);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
  dec32(_eax);
LABEL_DEF(_store2);
  mov32_mem_reg(_GPR_D(_rd, 2), _eax);

  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  add32_reg_mem(_eax, _GPR_D(_rt, 3));
  jnc(_store3);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
  dec32(_eax);
LABEL_DEF(_store3);
  mov32_mem_reg(_GPR_D(_rd, 3), _eax);
}

void padduh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddusw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void paddub() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddusb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubuw() {
  enum { _store0, _store1, _store2, _store3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  sub32_reg_mem(_eax, _GPR_D(_rt, 0));
  jnc(_store0);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
LABEL_DEF(_store0);
  mov32_mem_reg(_GPR_D(_rd, 0), _eax);

  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  sub32_reg_mem(_eax, _GPR_D(_rt, 1));
  jnc(_store1);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
LABEL_DEF(_store1);
  mov32_mem_reg(_GPR_D(_rd, 1), _eax);

  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  sub32_reg_mem(_eax, _GPR_D(_rt, 2));
  jnc(_store2);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
LABEL_DEF(_store2);
  mov32_mem_reg(_GPR_D(_rd, 2), _eax);

  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  sub32_reg_mem(_eax, _GPR_D(_rt, 3));
  jnc(_store3);
  // –O˜aˆ—
  xor32_reg_reg(_eax, _eax);
LABEL_DEF(_store3);
  mov32_mem_reg(_GPR_D(_rd, 3), _eax);
}

void psubuh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubusw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubub() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubusb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void padsbh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  movdqa_reg_reg(_xmm1, _xmm0);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm2, _eax);
  paddw_reg_reg(_xmm1, _xmm2);
  psrldq_reg_imm(_xmm1, 8);
  psubw_reg_reg(_xmm0, _xmm2);
  punpcklqdq_reg_reg(_xmm0, _xmm1);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pabsw() {
  enum { _store0, _store1, _store2, _store3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rt, 0));
  tst32_reg_reg(_eax, _eax);
  jns(_store0); // ³”‚È‚ç‚Î‚»‚Ì‚Ü‚ÜƒXƒgƒA
  neg32(_eax);
  jns(_store0);
  // 0x80000000‚Ìê‡‚Ì‚Ý‚»‚Ì‚Ü‚Ü•‰‚È‚Ì‚Å0x7fffffff‚ÉƒNƒ‰ƒ“ƒv‚ª•K—v
  dec32(_eax);
LABEL_DEF(_store0);
  mov32_mem_reg(_GPR_D(_rd, 0), _eax);

  // 63-32
  mov32_eax_mem(_GPR_D(_rt, 1));
  tst32_reg_reg(_eax, _eax);
  jns(_store1); // ³”‚È‚ç‚Î‚»‚Ì‚Ü‚ÜƒXƒgƒA
  neg32(_eax);
  jns(_store1);
  // 0x80000000‚Ìê‡‚Ì‚Ý‚»‚Ì‚Ü‚Ü•‰‚È‚Ì‚Å0x7fffffff‚ÉƒNƒ‰ƒ“ƒv‚ª•K—v
  dec32(_eax);
LABEL_DEF(_store1);
  mov32_mem_reg(_GPR_D(_rd, 1), _eax);

  // 95-64
  mov32_eax_mem(_GPR_D(_rt, 2));
  tst32_reg_reg(_eax, _eax);
  jns(_store2); // ³”‚È‚ç‚Î‚»‚Ì‚Ü‚ÜƒXƒgƒA
  neg32(_eax);
  jns(_store2);
  // 0x80000000‚Ìê‡‚Ì‚Ý‚»‚Ì‚Ü‚Ü•‰‚È‚Ì‚Å0x7fffffff‚ÉƒNƒ‰ƒ“ƒv‚ª•K—v
  dec32(_eax);
LABEL_DEF(_store2);
  mov32_mem_reg(_GPR_D(_rd, 2), _eax);

  // 127-96
  mov32_eax_mem(_GPR_D(_rt, 3));
  tst32_reg_reg(_eax, _eax);
  jns(_store3); // ³”‚È‚ç‚Î‚»‚Ì‚Ü‚ÜƒXƒgƒA
  neg32(_eax);
  jns(_store3);
  // 0x80000000‚Ìê‡‚Ì‚Ý‚»‚Ì‚Ü‚Ü•‰‚È‚Ì‚Å0x7fffffff‚ÉƒNƒ‰ƒ“ƒv‚ª•K—v
  dec32(_eax);
LABEL_DEF(_store3);
  mov32_mem_reg(_GPR_D(_rd, 3), _eax);
}

void pabsh() {
#define macro(_idx)                 \
  mov32_eax_mem(_GPR_D(_rt, _idx)); \
  mov32_reg_reg(_ecx, _eax);        \
  tst32_reg_reg(_eax, _eax);        \
  jns(_idx*2);                      \
  not32(_eax);                      \
  add32_reg_reg(_eax, _esi);        \
  jns(_idx*2);                      \
  sub32_reg_reg(_eax, _esi);        \
LABEL_DEF(_idx*2);                  \
  shl32_reg_imm8(_ecx, 16);         \
  tst32_reg_reg(_ecx, _ecx);        \
  jns(_idx*2+1);                    \
  not32(_ecx);                      \
  add32_reg_reg(_ecx, _esi);        \
  jns(_idx*2+1);                    \
  sub32_reg_reg(_ecx, _esi);        \
LABEL_DEF(_idx*2+1);                \
  shr32_reg_imm8(_ecx, 16);         \
  mov16_reg_reg(_ax, _cx);          \
  mov32_mem_reg(_GPR_D(_rd, _idx), _eax);

  mov32_reg_imm32(_esi, 0x00010000);
  
  macro(0); // 31-0
  macro(1); // 63-32
  macro(2); // 95-64
  macro(3); // 127-96

#undef macro
}

void pceqw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pcmpeqd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pceqh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pcmpeqw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pceqb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pcmpeqb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pextuw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpckhdq_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pextuh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpckhwd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pextub() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpckhbw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pminw() {
  enum { _store_rt0, _store_rt1, _store_rt2, _store_rt3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  cmp32_reg_reg(_eax, _ecx);
  jle(_store_rt0);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt0);
  mov32_mem_eax(_GPR_D(_rd, 0));

  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 1));
  cmp32_reg_reg(_eax, _ecx);
  jle(_store_rt1);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt1);
  mov32_mem_eax(_GPR_D(_rd, 1));

  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 2));
  cmp32_reg_reg(_eax, _ecx);
  jle(_store_rt2);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt2);
  mov32_mem_eax(_GPR_D(_rd, 2));

  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 3));
  cmp32_reg_reg(_eax, _ecx);
  jle(_store_rt3);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt3);
  mov32_mem_eax(_GPR_D(_rd, 3));
}

void pminh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pminsw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void qfsrv() {
}

#undef _rs
#undef _rt
#undef _rd

void mmi1() {
  void (*TABLE[0x20])() = {
    invalid, pabsw,   pceqw,   pminw,
    padsbh,  pabsh,   pceqh,   pminh,
    invalid, invalid, pceqb,   invalid,
    invalid, invalid, invalid, invalid,
    padduw,  psubuw,  pextuw,  invalid,
    padduh,  psubuh,  pextuh,  invalid,
    paddub,  psubub,  pextub,  qfsrv,
    invalid, invalid, invalid, invalid,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 6) & 0x1f]();
}
