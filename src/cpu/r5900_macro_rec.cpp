#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"
#include "memory/memory.h"

#include <assert.h>

#define _va2pa(_base, _offset)  _va2pa_mask(0x000fffff, _base, _offset)

u32 r5900_macro_01(u32 p_va) {
  const int op_count = 3;
  if (p_va + (op_count << 2) > g_code_addr + g_code_size) {
    return 0;
  }
  u32 op[] = {
    *((u32*)VA2PA(p_va + 0)), // nop
    *((u32*)VA2PA(p_va + 4)), // nop
    *((u32*)VA2PA(p_va + 8)), // nop
  };
  if (op[0] != 0x00000000) return 0;
  if (op[1] != 0x00000000) return 0;
  if (op[2] != 0x00000000) return 0;

  enum { _macro_rec = 50, _end };
  cmp32_mem_imm32((u32)&g_cpu->m_x86_jump_addr, 0x00000000);
  jnz32(_end);
LABEL_DEF(_macro_rec);
  add32_mem_imm32((u32)&g_cpu->m_pc, (op_count - 1) << 2);

LABEL_DEF(_end);
  g_cpu->m_recompiling_ee_va += (op_count - 1) << 2;
  return op_count; // パターンに符合する場合は1以上のEE命令数を返す
}

u32 r5900_macro_02(u32 p_va) {
  const int op_count = 7;
  if (p_va + (op_count << 2) > g_code_addr + g_code_size) {
    return 0;
  }
  u32 op[] = {
    *((u32*)VA2PA(p_va + 0x00)), // addiu gpr1, gpr1, _imm
    *((u32*)VA2PA(p_va + 0x04)), // sw gpr3, _imm(gpr2)
    *((u32*)VA2PA(p_va + 0x08)), // sltiu gpr0, gpr1, _imm
    *((u32*)VA2PA(p_va + 0x0c)), // nop
    *((u32*)VA2PA(p_va + 0x10)), // nop
    *((u32*)VA2PA(p_va + 0x14)), // bne gpr0, zero, -5
    *((u32*)VA2PA(p_va + 0x18)), // addiu gpr2, gpr2, _imm
  };
  
  if ((op[0] &  0xfc000000) != 0x24000000) return 0;
  if ((op[1] &  0xfc000000) != 0xac000000) return 0;
  if ((op[2] &  0xfc000000) != 0x2c000000) return 0;
  if (op[3] != 0x00000000) return 0;
  if (op[4] != 0x00000000) return 0;
  if ((op[5] &  0xfc000000) != 0x14000000) return 0;
  if ((op[6] &  0xfc000000) != 0x24000000) return 0;

  int gpr0 = (op[2] >> 16) & 0x1f;
  if ((op[5] >> 21 & 0x1f) != gpr0) return 0;

  int  gpr1 = (op[0] >> 16) & 0x1f;
  if ((op[0] >> 21 & 0x1f) != gpr1) return 0;
  if ((op[2] >> 21 & 0x1f) != gpr1) return 0;
  
  int gpr2 = (op[1] >> 21) & 0x1f;
  if ((op[6] >> 16 & 0x1f) != gpr2) return 0;
  if ((op[6] >> 21 & 0x1f) != gpr2) return 0;

  int gpr3 = (op[1] >> 16) & 0x1f;

  enum { _macro_rec = 50, _end, _label1 };

  cmp32_mem_imm32((u32)&g_cpu->m_x86_jump_addr, 0x00000000);
  jz32(_macro_rec);

  // 通常のリコンパイラを呼ぶときは
  // g_cpu->m_recompiling_opcodeを実行する命令に合わせる事
  g_cpu->m_recompiling_opcode = op[0];
  void addiu();
  addiu();

  jmp32(_end);
LABEL_DEF(_macro_rec);
  u32 gpr0ptr = _GPR_D(gpr0, 0);
  u32 gpr1ptr = _GPR_D(gpr1, 0);
  u32 gpr2ptr = _GPR_D(gpr2, 0);
  u32 gpr3ptr = _GPR_D(gpr3, 0);

  mov32_reg_mem(_edi, gpr3ptr);
LABEL_DEF(_label1);
  // addiu
  add32_mem_imm32(gpr1ptr, (s16)(op[0] & 0xffff));
  // sw
  _va2pa(gpr2, (s16)(op[1] & 0xffff));
  mov32_rmem_reg(_eax, _edi);
  // addiu
  add32_mem_imm32(gpr2ptr, (s16)(op[6] & 0xffff));
  // sltiu bne
  cmp32_mem_imm32(gpr1ptr, (s32)(op[2] & 0xffff));
  jl(_label1);

  // 最後に１度だけ符号拡張すればよい
  cdq();
  mov32_mem_reg(gpr1ptr + 4, _edx);
  // 最後にgpr0は必ず1になる
  mov32_mem_imm32(gpr0ptr, 1);

  add32_mem_imm32((u32)&g_cpu->m_pc, (op_count - 1) << 2);

LABEL_DEF(_end);
  g_cpu->m_recompiling_ee_va += (op_count - 1) << 2;
  return op_count; // パターンに符合する場合は1以上のEE命令数を返す
}

//-----------------------------------------------------------------------------
// global
u32 (*g_r5900_macro_ary[])(u32 p_va) = {
  r5900_macro_01,
  r5900_macro_02,
  NULL
};
