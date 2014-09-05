#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif

#include <assert.h>

void invalid();

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |   rs   |  inst  |     offset     |
// +--------+--------+--------+----------------+
#define _rs    (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _offset  (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void bltz() {
  // if (_rs.0-63 < 0) delayslot = instaddr + 4 + _offset * 4
  enum { _end };
  
  // 符号ビットが0ならジャンプしない
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  jns(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bgez() {
  // if (_rs.0-63 >= 0) delayslot = instaddr + 4 + _offset * 4
  enum { _end };
  
  // 符号ビットが1ならジャンプしない
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bltzl() {
  // if (_rs.0-63 < 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };
  
  // 符号ビットが1ならジャンプ
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_jump);
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
  jmp(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bgezl() {
  // if (_rs.0-63 >= 0) delayslot = instaddr + 4 + _offset * 4
enum { _jump, _end };
  
  // 符号ビットが0ならジャンプ
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  jns(_jump);
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
  jmp(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bltzal() {
  // if (_rs.0-63 < 0) delayslot = instaddr + 4 + _offset * 4
  enum { _end };
  
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[0], g_cpu->m_recompiling_ee_va + 8);
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[1], 0x00000000);

  // 符号ビットが0ならジャンプしない
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  jns(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPush);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bgezal() {
  // if (_rs.0-63 >= 0) delayslot = instaddr + 4 + _offset * 4
  enum { _end };
  
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[0], g_cpu->m_recompiling_ee_va + 8);
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[1], 0x00000000);

  // 符号ビットが1ならジャンプしない
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPush);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bltzall() {
  // if (_rs.0-63 < 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };
  
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[0], g_cpu->m_recompiling_ee_va + 8);
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[1], 0x00000000);

  // 符号ビットが1ならジャンプ
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_jump);
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
  jmp(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPush);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bgezall() {
  // if (_rs.0-63 >= 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };
  
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[0], g_cpu->m_recompiling_ee_va + 8);
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[1], 0x00000000);

  // 符号ビットが0ならジャンプ
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  jns(_jump);
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
  jmp(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPush);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

#undef _rs
#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |   rs   |  inst  |    immediate   |
// +--------+--------+--------+----------------+
#define _rs    (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _imm  (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void tgei() {
  invalid();
}

void tgeiu() {
  invalid();
}

void tlti() {
  invalid();
}

void tltiu() {
  invalid();
}

void teqi() {
  invalid();
}

void tnei() {
  invalid();
}

void mtsab() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  xor32_reg_imm8(_eax, _imm & 0x0f);
  and32_reg_imm8(_eax, 0x0f);
  shl32_reg_imm8(_eax, 3);
  mov32_mem_eax((u32)&g_cpu->m_sa);
}

void mtsah() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  xor32_reg_imm8(_eax, _imm & 7);
  and32_reg_imm8(_eax, 7);
  shl32_reg_imm8(_eax, 4);
  mov32_mem_eax((u32)&g_cpu->m_sa);
}

#undef _rs
#undef _imm

void regimm() {
  static void (*TABLE[0x20])() = {
    bltz,   bgez,   bltzl,   bgezl,   invalid, invalid, invalid, invalid,
    tgei,   tgeiu,  tlti,    tltiu,   teqi,    invalid, tnei,    invalid,
    bltzal, bgezal, bltzall, bgezall, invalid, invalid, invalid, invalid,
    mtsab,  mtsah,  invalid, invalid, invalid, invalid, invalid, invalid,
  };
  TABLE[(g_cpu->m_recompiling_opcode >> 16) & 0x1f]();
}
