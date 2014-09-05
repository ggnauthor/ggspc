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

void paddw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void paddh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void paddb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void paddsw() {
enum { _store0, _store1, _store2, _store3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  add32_reg_mem(_eax, _GPR_D(_rt, 0));
  jno(_store0);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000); // 負数のオーバーフローなら0x80000000
  jns(_store0)
  dec32(_eax);                       // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store0);
  mov32_mem_reg(_GPR_D(_rd, 0), _eax);

  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  add32_reg_mem(_eax, _GPR_D(_rt, 1));
  jno(_store1);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000); // 負数のオーバーフローなら0x80000000
  jns(_store1)
  dec32(_eax);                       // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store1);
  mov32_mem_reg(_GPR_D(_rd, 1), _eax);

  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  add32_reg_mem(_eax, _GPR_D(_rt, 2));
  jno(_store2);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000); // 負数のオーバーフローなら0x80000000
  jns(_store2)
  dec32(_eax);                       // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store2);
  mov32_mem_reg(_GPR_D(_rd, 2), _eax);

  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  add32_reg_mem(_eax, _GPR_D(_rt, 3));
  jno(_store3);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000); // 負数のオーバーフローなら0x80000000
  jns(_store3)
  dec32(_eax);                       // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store3);
  mov32_mem_reg(_GPR_D(_rd, 3), _eax);
}

void paddsh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddsw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void paddsb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  paddsb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubsw() {
enum { _store0, _store1, _store2, _store3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  sub32_reg_mem(_eax, _GPR_D(_rt, 0));
  jno(_store0);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000); // 負数のオーバーフローなら0x80000000
  jns(_store0)
  dec32(_eax);                       // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store0);
  mov32_mem_reg(_GPR_D(_rd, 0), _eax);

  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  sub32_reg_mem(_eax, _GPR_D(_rt, 1));
  jno(_store1);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000); // 負数のオーバーフローなら0x80000000
  jns(_store1)
  dec32(_eax);                       // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store1);
  mov32_mem_reg(_GPR_D(_rd, 1), _eax);

  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  sub32_reg_mem(_eax, _GPR_D(_rt, 2));
  jno(_store2);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000);  // 負数のオーバーフローなら0x80000000
  jns(_store2)
  dec32(_eax);                        // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store2);
  mov32_mem_reg(_GPR_D(_rd, 2), _eax);

  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  sub32_reg_mem(_eax, _GPR_D(_rt, 3));
  jno(_store3);
  // 符号付き飽和処理
  mov32_reg_imm32(_eax, 0x80000000);  // 負数のオーバーフローなら0x80000000
  jns(_store3)
  dec32(_eax);                        // 正数のオーバーフローなら0x7fffffff
LABEL_DEF(_store3);
  mov32_mem_reg(_GPR_D(_rd, 3), _eax);
}

void psubsh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubsw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void psubsb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  psubsb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pcgtw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pcmpgtd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pcgth() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pcmpgtw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pcgtb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pcmpgtb_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pextlw() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpckldq_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pextlh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpcklwd_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pextlb() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  punpcklbw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void pext5() {
#define macro(_idx)                  \
  mov32_eax_mem(_GPR_D(_rt, _idx));  \
  mov32_reg_reg(_ecx, _eax);         \
  shl32_reg_imm8(_eax, 3);           \
  and32_eax_imm32(0x000000f8);       \
  mov32_reg_reg(_edx, _ecx);         \
  shl32_reg_imm8(_edx, 6);           \
  and32_reg_imm32(_edx, 0x0000f800); \
  or32_reg_reg(_eax, _edx);          \
  mov32_reg_reg(_edx, _ecx);         \
  shl32_reg_imm8(_edx, 9);           \
  and32_reg_imm32(_edx, 0x00f80000); \
  or32_reg_reg(_eax, _edx);          \
  shl32_reg_imm8(_ecx, 16);          \
  and32_reg_imm32(_ecx, 0x80000000); \
  or32_reg_reg(_eax, _ecx);          \
  mov32_mem_eax(_GPR_D(_rd, _idx));

  macro(0);
  macro(1);
  macro(2);
  macro(3);

#undef macro
}

void pmaxw() {
enum { _store_rt0, _store_rt1, _store_rt2, _store_rt3 };
  // 31-0
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  cmp32_reg_reg(_eax, _ecx);
  jge(_store_rt0);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt0);
  mov32_mem_eax(_GPR_D(_rd, 0));

  // 63-32
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 1));
  cmp32_reg_reg(_eax, _ecx);
  jge(_store_rt1);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt1);
  mov32_mem_eax(_GPR_D(_rd, 1));

  // 95-64
  mov32_eax_mem(_GPR_D(_rs, 2));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 2));
  cmp32_reg_reg(_eax, _ecx);
  jge(_store_rt2);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt2);
  mov32_mem_eax(_GPR_D(_rd, 2));

  // 127-96
  mov32_eax_mem(_GPR_D(_rs, 3));
  mov32_reg_mem(_ecx, _GPR_D(_rt, 3));
  cmp32_reg_reg(_eax, _ecx);
  jge(_store_rt3);
  mov32_reg_reg(_eax, _ecx);
LABEL_DEF(_store_rt3);
  mov32_mem_eax(_GPR_D(_rd, 3));
}

void pmaxh() {
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rs]);
  movdqa_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rt]);
  pmaxsw_reg_rmem(_xmm0, _eax);
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_gpr[_rd]);
  movdqa_rmem_reg(_eax, _xmm0);
}

void ppacw() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax(_GPR_D(_rd, 2));
  mov32_eax_mem(_GPR_D(_rs, 2));
  mov32_mem_eax(_GPR_D(_rd, 3));

  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem(_GPR_D(_rt, 2));
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void ppach() {
  mov32_reg_imm32(_esi, 0x0000ffff);

  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  and32_reg_reg(_ecx, _esi);
  mov32_eax_mem(_GPR_D(_rt, 1));
  shl32_reg_imm8(_eax, 16);
  or32_reg_reg(_eax, _ecx);
  mov32_mem_eax(_GPR_D(_rd, 0));

  mov32_reg_mem(_ecx, _GPR_D(_rt, 2));
  and32_reg_reg(_ecx, _esi);
  mov32_eax_mem(_GPR_D(_rt, 3));
  shl32_reg_imm8(_eax, 16);
  or32_reg_reg(_eax, _ecx);
  mov32_mem_eax(_GPR_D(_rd, 1));

  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  and32_reg_reg(_ecx, _esi);
  mov32_eax_mem(_GPR_D(_rs, 1));
  shl32_reg_imm8(_eax, 16);
  or32_reg_reg(_eax, _ecx);
  mov32_mem_eax(_GPR_D(_rd, 2));

  mov32_reg_mem(_ecx, _GPR_D(_rs, 2));
  and32_reg_reg(_ecx, _esi);
  mov32_eax_mem(_GPR_D(_rs, 3));
  shl32_reg_imm8(_eax, 16);
  or32_reg_reg(_eax, _ecx);
  mov32_mem_eax(_GPR_D(_rd, 3));
}

void ppacb() {
#define macro(_reg, _idx1, _idx2)               \
  mov32_reg_mem(_ecx, _GPR_D(_reg, _idx1));     \
  mov32_reg_reg(_eax, _ecx);                    \
  and32_reg_reg(_eax, _edi);                    \
  and32_reg_reg(_ecx, _esi);                    \
  shr32_reg_imm8(_ecx, 8);                      \
  or32_reg_reg(_eax, _ecx);                     \
  mov32_reg_mem(_ecx, _GPR_D(_reg, _idx1 + 1)); \
  mov32_reg_reg(_edx, _ecx);                    \
  and32_reg_reg(_edx, _edi);                    \
  shl32_reg_imm8(_edx, 16);                     \
  or32_reg_reg(_eax, _edx);                     \
  and32_reg_reg(_ecx, _esi);                    \
  shl32_reg_imm8(_ecx, 8);                      \
  or32_reg_reg(_eax, _ecx);                     \
  mov32_mem_eax(_GPR_D(_rd, _idx2));

  mov32_reg_imm32(_esi, 0x00ff0000);
  mov32_reg_imm32(_edi, 0x000000ff);

  macro(_rt, 0, 0);
  macro(_rt, 2, 1);
  macro(_rs, 0, 2);
  macro(_rs, 2, 3);

#undef macro
}

void ppac5() {
#define macro(_idx)                 \
  mov32_eax_mem(_GPR_D(_rt, _idx)); \
  mov32_reg_reg(_ecx, _eax);        \
  shr32_reg_imm8(_eax, 3);          \
  and32_eax_imm32(0x001f);          \
  mov32_reg_reg(_edx, _ecx);        \
  shr32_reg_imm8(_edx, 6);          \
  and32_reg_imm32(_edx, 0x03e0);    \
  or32_reg_reg(_eax, _edx);         \
  mov32_reg_reg(_edx, _ecx);        \
  shr32_reg_imm8(_edx, 9);          \
  and32_reg_imm32(_edx, 0x7c00);    \
  or32_reg_reg(_eax, _edx);         \
  shr32_reg_imm8(_ecx, 16);         \
  and32_reg_imm32(_ecx, 0x8000);    \
  or32_reg_reg(_eax, _ecx);         \
  mov32_mem_eax(_GPR_D(_rd, _idx));

  macro(0);
  macro(1);
  macro(2);
  macro(3);

#undef macro
}

#undef _rs
#undef _rt
#undef _rd

void mmi0() {
  void (*TABLE[0x20])() = {
    paddw,   psubw,   pcgtw,   pmaxw,
    paddh,   psubh,   pcgth,   pmaxh,
    paddb,   psubb,   pcgtb,   invalid,
    invalid, invalid, invalid, invalid,
    paddsw,  psubsw,  pextlw,  ppacw,
    paddsh,  psubsh,  pextlh,  ppach,
    paddsb,  psubsb,  pextlb,  ppacb,
    invalid, invalid, pext5,   ppac5,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 6) & 0x1f]();
}
