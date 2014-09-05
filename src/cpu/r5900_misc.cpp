//-----------------------------------------------------------------------------
// include
#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"
#include "memory/memory.h"

#if defined _DEBUGGER
#include "debugger/debugger.h"
#include "debugger/disasm.h"
#include "debugger/gui/window/memory_window.h"
#endif

#include <utils/ds_util.h>

//-----------------------------------------------------------------------------
// define

//-----------------------------------------------------------------------------
// function
void displayInvalidInstruction() {
  DBGOUT_CPU("invalid instruction!! ADDRESS:%08x OPCODE:%08x\n",
             g_cpu->m_pc, *((u32*)VA2PA(g_cpu->m_pc)));
}

void invalid() {
  //char inst[1024];
  //disasm_plain(g_cpu->m_recompiling_ee_va, g_opcode, inst);
  //DBGOUT_CPU("%s is not implemented! ADDRESS:%08x\n",
  //           inst, g_cpu->m_recompiling_ee_va);

  mov32_reg_imm32(_eax, (u32)displayInvalidInstruction);
  call_reg(_eax);
}

// +--------+----------------------------------+
// |  26(6) |              0(26)               |
// |  inst  |             offset               |
// +--------+----------------------------------+
#define _offset (g_cpu->m_recompiling_opcode & 0x03ffffff)

void j() {
  // delayslot = ((instaddr + 4) & 0xf0000000) | (_offset << 2);
  mov32_reg_imm32(_eax, ((g_cpu->m_recompiling_ee_va + 4) & 0xf0000000) |
                        (_offset << 2));
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif
}

void jal() {
  // delayslot = ((instaddr + 4) & 0xf0000000) | (_offset << 2);
  mov32_reg_imm32(_eax, ((g_cpu->m_recompiling_ee_va + 4) & 0xf0000000) |
                        (_offset << 2));
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);
  // ra.0-63 = instaddr + 8;
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[0], g_cpu->m_recompiling_ee_va + 8);
  mov32_mem_imm32((u32)&g_cpu->m_ra.d[1], 0x00000000);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPush);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif
}

#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |   rs   |   rt   |     offset     |
// +--------+--------+--------+----------------+
#define _rs     (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt     (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _offset (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void beq() {
  // if (_rs.0-31 == _rt.0-31 && _rs.32-63 == _rt.32-63)
  //   delayslot = instaddr + 4 + _offset * 4
  enum { _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 0));
  jnz(_end);
  mov32_eax_mem(_GPR_D(_rs, 1));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 1));
  jnz(_end);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bne() {
  // if (_rs.0-31 != _rt.0-31 || _rs.32-63 != _rt.32-63)
  //   delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 0));
  jnz(_jump);
  mov32_eax_mem(_GPR_D(_rs, 1));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 1));
  jz(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void blez() {
  // if (_rs.0-63 <= 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };
  
  // 符号ビットが1ならジャンプ
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_jump);
  // 上位word値が0でなければ終了
  jnz(_end);
  // 下位word値が0でなければ終了
  mov32_eax_mem(_GPR_D(_rs, 0));
  tst32_reg_reg(_eax, _eax);
  jnz(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void bgtz() {
  // if (_rs.0-63 > 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };
  
  // 符号ビットが1なら終了
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_end);
  // 上位word値が0でなければジャンプ
  jnz(_jump);
  // 下位word値が0でなければジャンプ
  mov32_eax_mem(_GPR_D(_rs, 0));
  tst32_reg_reg(_eax, _eax);
  jz(_end);
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

#undef _rs
#undef _rt
#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |   rs   |   rt   |    immediate   |
// +--------+--------+--------+----------------+
#define _rs    (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt    (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _imm   (u16)(g_cpu->m_recompiling_opcode & 0xffff)
#define _imm_s (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void addi() {
  // _rt.0-63 = sx64(_rs.0-31 + sx32(_imm))
  enum { _overflow, _end };

  if (_imm != 0) {
    mov32_reg_imm32(_eax, _imm);
    cwde();
    add32_reg_mem(_eax, _GPR_D(_rs, 0));
    jo(_overflow);
  } else {
    mov32_eax_mem(_GPR_D(_rs, 0));
  }
  mov32_mem_eax(_GPR_D(_rt, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
LABEL_DEF(_overflow);
  // Signal Exception発生
  // rtレジスタに変更は行わない
}

void addiu() {
  // オーバーフロー例外を発生しない
  // _rt.0-63 = sx64(_rs.0-31 + sx32(_imm))
  if (_imm != 0) {
    mov32_reg_imm32(_eax, _imm);
    cwde();
    add32_reg_mem(_eax, _GPR_D(_rs, 0));
  } else {
    mov32_eax_mem(_GPR_D(_rs, 0));
  }
  mov32_mem_eax(_GPR_D(_rt, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void slti() {
  // _rt.0-63 = _rs.0-63 < sx64(_imm)
  enum { _lowword, _store };

  xor32_reg_reg(_ecx, _ecx);
  mov32_reg_imm32(_eax, _imm);
  cwde();
  cdq();
  cmp32_mem_reg(_GPR_D(_rs, 1), _edx);
  mov32_mem_reg(_GPR_D(_rt, 1), _ecx);
  jz(_lowword);
  setl(_cl); // 上位wordは符号有り比較
  jmp(_store);
LABEL_DEF(_lowword);
  // さらに下位wordの比較を行う
  cmp32_mem_reg(_GPR_D(_rs, 0), _eax);
  setb(_cl); // 下位wordは符号無し比較
LABEL_DEF(_store);
  mov32_mem_reg(_GPR_D(_rt, 0), _ecx);
}

void sltiu() {
  // _rt.0-63 = _rs.0-63 < sx64(_imm)
  // _immは符号拡張はされるが無符号として比較する
  enum { _store };

  xor32_reg_reg(_ecx, _ecx);
  mov32_reg_imm32(_eax, _imm);
  cwde();
  cdq();
  cmp32_mem_reg(_GPR_D(_rs, 1), _edx);
  mov32_mem_reg(_GPR_D(_rt, 1), _ecx);
  jnz(_store);
  // さらに下位wordの比較を行う
  cmp32_mem_reg(_GPR_D(_rs, 0), _eax);
LABEL_DEF(_store);
  setb(_cl);    // 上位下位word共に符号無し比較
  mov32_mem_reg(_GPR_D(_rt, 0), _ecx);
}

void andi() {
  // _rt.0-63 = _rs.0-63 & _imm
  xor32_reg_reg(_eax, _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
  mov32_eax_mem(_GPR_D(_rs, 0));
  and32_reg_imm32(_eax, _imm);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
}

void ori() {
  // _rt.0-63 = _rs.0-63 | _imm
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
  mov32_eax_mem(_GPR_D(_rs, 0));
  or32_reg_imm32(_eax, _imm);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
}

void xori() {
  // _rt.0-63 = _rs.0-63 ^ _imm
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
  mov32_eax_mem(_GPR_D(_rs, 0));
  xor32_reg_imm32(_eax, _imm);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
}

void lui() {
  // オーバーフロー例外を発生しない
  // _rt.0-63 = sx64(_imm << 16)
  enum { _sign, _end };

  u32 imm = _imm;
  if (imm & 0x8000) {
    mov32_mem_imm32(_GPR_D(_rt, 0), imm << 16);
    mov32_mem_imm32(_GPR_D(_rt, 1), 0xffffffff);
  } else {
    mov32_mem_imm32(_GPR_D(_rt, 0), imm << 16);
    mov32_mem_imm32(_GPR_D(_rt, 1), 0x00000000);
  }
}

void daddi() {
  // _rt.0-63 = _rs.0-63 + sx32(_imm)
  enum { _overflow, _end };

  mov32_reg_mem(_ecx, _GPR_D(_rs, 1));
  if (_imm_s >= 0) {
    mov32_eax_mem(_GPR_D(_rs, 0));
    add32_eax_imm32(_imm_s);
    adc32_reg_imm8(_ecx, 0);
  } else {
    mov32_eax_mem(_GPR_D(_rs, 0));
    sub32_eax_imm32(-_imm_s);
    sbb32_reg_imm8(_ecx, 0);
  }
  jo(_overflow);
  mov32_mem_eax(_GPR_D(_rt, 0));
  mov32_mem_reg(_GPR_D(_rt, 1), _ecx);
  //jmp(_end);
LABEL_DEF(_overflow);
  // Signal Exception発生
  // rtレジスタに変更は行わない
//LABEL_DEF(_end);
}

void daddiu() {
  // _rt.0-63 = _rs.0-63 + sx32(_imm)
  mov32_reg_mem(_ecx, _GPR_D(_rs, 1));
  if (_imm_s >= 0) {
    mov32_eax_mem(_GPR_D(_rs, 0));
    add32_eax_imm32(_imm_s);
    adc32_reg_imm8(_ecx, 0);
  } else {
    mov32_eax_mem(_GPR_D(_rs, 0));
    sub32_eax_imm32(-_imm_s);
    sbb32_reg_imm8(_ecx, 0);
  }
  mov32_mem_eax(_GPR_D(_rt, 0));
  mov32_mem_reg(_GPR_D(_rt, 1), _ecx);
}

#undef _rs
#undef _rt
#undef _imm

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |   rs   |   rt   |     offset     |
// +--------+--------+--------+----------------+
#define _rs     (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt     (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _offset (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void beql() {
  // if (_rs.0-31 == _rt.0-31 && _rs.32-63 == _rt.32-63)
  //   delayslot = instaddr + 4 + _offset * 4
  enum { _notjump, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 0));
  jnz(_notjump);
  mov32_eax_mem(_GPR_D(_rs, 1));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 1));
  jnz(_notjump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

  jmp(_end)
LABEL_DEF(_notjump);
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
LABEL_DEF(_end);
}

void bnel() {
  // if (_rs.0-31 != _rt.0-31 || _rs.32-63 != _rt.32-63)
  //   delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 0));
  jnz(_jump);
  mov32_eax_mem(_GPR_D(_rs, 1));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 1));
  jnz(_jump);
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
  jmp(_end)
LABEL_DEF(_jump);
  mov32_reg_imm32(_eax, (g_cpu->m_recompiling_ee_va + 4) + _offset * 4);
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif

LABEL_DEF(_end);
}

void blezl() {
  // if (_rs.0-63 <= 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _notjump, _end };
  
  // 符号ビットが1ならジャンプ
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_jump);
  // 上位word値が0でなければ終了
  jnz(_notjump);
  // 下位word値が0でなければ終了
  mov32_eax_mem(_GPR_D(_rs, 0));
  tst32_reg_reg(_eax, _eax);
  jnz(_notjump);
LABEL_DEF(_jump);
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

void bgtzl() {
  // if (_rs.0-63 > 0) delayslot = instaddr + 4 + _offset * 4
  enum { _jump, _notjump, _end };
  
  // 符号ビットが1なら終了
  mov32_eax_mem(_GPR_D(_rs, 1));
  tst32_reg_reg(_eax, _eax);
  js(_notjump);
  // 上位word値が0でなければジャンプ
  jnz(_jump);
  // 下位word値が0でなければジャンプ
  mov32_eax_mem(_GPR_D(_rs, 0));
  tst32_reg_reg(_eax, _eax);
  jz(_notjump);
LABEL_DEF(_jump);
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

#undef _rs
#undef _rt
#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |  base  |   rt   |     offset     |
// +--------+--------+--------+----------------+
#define _base   (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt     (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _offset (s16)(g_cpu->m_recompiling_opcode & 0xffff)

#define _va2pa() _va2pa_mask(0x000fffff, _base, _offset)

void sb() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-7
  _va2pa();
  mov32_reg_mem(_edx, _GPR_D(_rt, 0));
  mov_rmem_reg_8(_eax, _dl);
}

void sh() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-15
  _va2pa();
  mov32_reg_mem(_edx, _GPR_D(_rt, 0));
  mov_rmem_reg_16(_eax, _dx);
}

void sw() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-31
  _va2pa();
  mov32_reg_mem(_edx, _GPR_D(_rt, 0));
  mov32_rmem_reg(_eax, _edx);
}

void sd() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-63
  _va2pa();
  mov32_reg_mem(_edx, _GPR_D(_rt, 0));
  mov32_rmem_reg(_eax, _edx);
  mov32_reg_mem(_edx, _GPR_D(_rt, 1));
  mov32_rmem_ofs8_reg(_eax, 4, _edx);
}

void sq() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-127

  // sqの場合、アドレスの下位4ビットは勝手に切り捨てられる
  _va2pa_mask(0x000ffff0, _base, _offset);
  mov32_reg_imm32(_esi, _GPR_D(_rt, 0));
  mov32_reg_rmem(_edx, _esi);
  mov32_rmem_reg(_eax, _edx);
  mov32_reg_rmem_ofs8(_edx, _esi, 4);
  mov32_rmem_ofs8_reg(_eax, 4, _edx);
  mov32_reg_rmem_ofs8(_edx, _esi, 8);
  mov32_rmem_ofs8_reg(_eax, 8, _edx);
  mov32_reg_rmem_ofs8(_edx, _esi, 12);
  mov32_rmem_ofs8_reg(_eax, 12, _edx);
}

void swl() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-31
  enum { _0, _1, _2, _store };

  _va2pa();
  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  mov32_reg_reg(_edx, _eax);

  and32_reg_imm8(_eax, -4); // 0xfffffffc
  mov32_reg_rmem(_ebx, _eax);

  and32_reg_imm8(_edx, 3);
  jz(_0);
  dec32(_edx);
  jz(_1);
  dec32(_edx);
  jz(_2);
  mov32_reg_reg(_ebx, _ecx);
  jmp(_store);
LABEL_DEF(_0);
  shr32_reg_imm8(_ecx, 24);
  mov8_reg_reg(_bl, _cl);
  jmp(_store);
LABEL_DEF(_1);
  shr32_reg_imm8(_ecx, 16);
  mov16_reg_reg(_bx, _cx);
  jmp(_store);
LABEL_DEF(_2);
  shr32_reg_imm8(_ecx, 8);
  and32_reg_imm32(_ebx, 0xff000000);
  or32_reg_reg(_ebx, _ecx);
LABEL_DEF(_store);
  mov32_rmem_reg(_eax, _ebx);
}

void swr() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-31
  enum { _0, _1, _2, _store };

  _va2pa();
  mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
  mov32_reg_reg(_edx, _eax);

  and32_reg_imm8(_eax, -4); // 0xfffffffc
  mov32_reg_rmem(_ebx, _eax);

  and32_reg_imm8(_edx, 3);
  jz(_0);
  dec32(_edx);
  jz(_1);
  dec32(_edx);
  jz(_2);
  shl32_reg_imm8(_ecx, 24);
  and32_reg_imm32(_ebx, 0x00ffffff);
  or32_reg_reg(_ecx, _ebx);
  jmp(_store);
LABEL_DEF(_0);
  mov32_reg_reg(_ebx, _ecx);
  jmp(_store);
LABEL_DEF(_1);
  shl32_reg_imm8(_ecx, 8);
  mov8_reg_reg(_cl, _bl);
  jmp(_store);
LABEL_DEF(_2);
  shl32_reg_imm8(_ecx, 16);
  mov16_reg_reg(_cx, _bx);
LABEL_DEF(_store);
  mov32_rmem_reg(_eax, _ecx);
}

void sdl() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-63
  _va2pa();
  mov32_reg_reg(_edi, _eax);
  and32_reg_imm8(_edi, -8); // 0xfffffff8
  mov32_reg_reg(_ecx, _eax);
  and32_reg_imm8(_ecx, 7);
  inc32(_ecx);
  mov32_reg_imm32(_esi, _GPR_D(_rt, 0) + 8);
  sub32_reg_reg(_esi, _ecx);

  rep_movsb();
}

void sdr() {
  // mem[_base.0-31 + sx32(_offset)] = _rt.0-63
  _va2pa();
  mov32_reg_reg(_edi, _eax);
  mov32_reg_reg(_ecx, _eax);
  not32(_ecx);
  and32_reg_imm8(_ecx, 7);
  inc32(_ecx);
  mov32_reg_imm32(_esi, _GPR_D(_rt, 0));

  rep_movsb();
}

void lb() {
  // _rt.0-63 = sx64(mem[_base.0-31 + sx32(_offset)])
  _va2pa();
  movsx32_reg_rmem8(_eax, _eax);
  cdq();
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void lbu() {
  // _rt.0-63 = z.ex64(mem[_base.0-31 + sx32(_offset)])
  _va2pa();
  movzx32_reg_rmem8(_eax, _eax);
  xor32_reg_reg(_edx, _edx);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void lh() {
  // _rt.0-63 = sx64(mem[_base.0-31 + sx32(_offset)])
  _va2pa();
  movsx32_reg_rmem16(_eax, _eax);
  cdq();
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}
void lhu() {
  // _rt.0-63 = z.ex64(mem[_base.0-31 + sx32(_offset)])
  _va2pa();
  movzx32_reg_rmem16(_eax, _eax);
  xor32_reg_reg(_edx, _edx);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void lw() {
  // _rt.0-63 = sx64(mem[_base.0-31 + sx32(_offset)])
  _va2pa();
  mov32_reg_rmem(_eax, _eax);
  cdq();
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void lwu() {
  // _rt.0-63 = z.ex64(mem[_base.0-31 + sx32(_offset)])
  _va2pa();
  mov32_reg_rmem(_eax, _eax);
  xor32_reg_reg(_edx, _edx);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void ld() {
  // _rt.0-63 = mem[_base.0-31 + sx32(_offset)]
  _va2pa();
  mov32_reg_rmem(_edx, _eax);
  mov32_reg_rmem_ofs8(_eax, _eax, 4);
  mov32_mem_reg(_GPR_D(_rt, 0), _edx);
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
}

void lq() {
  // _rt.0-127 = mem[_base.0-31 + sx32(_offset)]

  // lqの場合、アドレスの下位4ビットは勝手に切り捨てられる
  _va2pa_mask(0x000ffff0, _base, _offset);
  mov32_reg_imm32(_ecx, _GPR_D(_rt, 0));

  mov32_reg_rmem(_edx, _eax);
  mov32_rmem_reg(_ecx, _edx);
  mov32_reg_rmem_ofs8(_edx, _eax, 4);
  mov32_rmem_ofs8_reg(_ecx, 4, _edx);
  mov32_reg_rmem_ofs8(_edx, _eax, 8);
  mov32_rmem_ofs8_reg(_ecx, 8, _edx);
  mov32_reg_rmem_ofs8(_edx, _eax, 12);
  mov32_rmem_ofs8_reg(_ecx, 12, _edx);
}

void lwl() {
  // _rt.0-63 = sx64(mem[_base.0-31 + sx32(_offset)])
  enum { _nosign };

  _va2pa();
  mov32_reg_reg(_esi, _eax);
  and32_reg_imm8(_esi, -4);  // 0xfffffffc
  mov32_reg_reg(_ecx, _eax);
  and32_reg_imm8(_ecx, 3);
  inc32(_ecx);
  mov32_reg_imm32(_edi, _GPR_D(_rt, 0) + 4);
  sub32_reg_reg(_edi, _ecx);

  rep_movsb();
#if 0 // 分岐しない版 なぜかこちらの方が20ms程遅い
  mov8_reg_rmem_ofs8(_al, _esi, -1);
  and8_reg_imm8(_al, 0x80);
  neg8(_al);
  sbb32_reg_reg(_eax, _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
#else
  xor32_reg_reg(_eax, _eax);
  tst8_rmem_ofs8_imm8(_esi, -1, 0xff);
  jns(_nosign);
  dec32(_eax);
LABEL_DEF(_nosign);
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
#endif
}

void lwr() {
  // _rt.0-63 = sx64(mem[_base.0-31 + sx32(_offset)])
  enum { _4byte, _nosign, _end };

  _va2pa();
  tst8_reg_imm8(_al, 3);
  jz(_4byte);

  mov32_reg_reg(_ecx, _eax);
  not32(_ecx);
  and32_reg_imm8(_ecx, 3);
  inc32(_ecx);
  
  mov32_reg_imm32(_edi, _GPR_D(_rt, 0));
  mov32_reg_reg(_esi, _eax);
  rep_movsb();

  jmp(_end);
LABEL_DEF(_4byte);
  // ４バイトコピーし、上位WORDは符号拡張を行う
  mov32_reg_rmem(_eax, _eax);
  mov32_mem_reg(_GPR_D(_rt, 0), _eax);
  and32_eax_imm32(0x80000000);
  neg32(_eax);
  sbb32_reg_reg(_eax, _eax);
  mov32_mem_reg(_GPR_D(_rt, 1), _eax);
LABEL_DEF(_end);
}

void ldl() {
  // _rt.0-63 = mem[_base.0-31 + sx32(_offset)]
  enum { _end };

  _va2pa();
  mov32_reg_reg(_esi, _eax);
  and32_reg_imm8(_esi, -8); // 0xfffffff8
  mov32_reg_reg(_ecx, _eax);
  and32_reg_imm8(_ecx, 7);
  inc32(_ecx);
  mov32_reg_imm32(_edi, _GPR_D(_rt, 0) + 8);
  sub32_reg_reg(_edi, _ecx);

  rep_movsb();
LABEL_DEF(_end);
}

void ldr() {
  // _rt.0-63 = mem[_base.0-31 + sx32(_offset)]
  enum { _end };

  _va2pa();
  mov32_reg_reg(_esi, _eax);
  mov32_reg_reg(_ecx, _eax);
  not32(_ecx);
  and32_reg_imm8(_ecx, 7);
  inc32(_ecx);
  mov32_reg_imm32(_edi, _GPR_D(_rt, 0));

  rep_movsb();
LABEL_DEF(_end);
}

#undef _base
#undef _rt
#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |  base  |  hint  |     offset     |
// +--------+--------+--------+----------------+
#define _base   (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _hint   (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _offset (u16)(g_cpu->m_recompiling_opcode & 0xffff)

void pref() {
}

#undef _base
#undef _hint
#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |  base  |   op   |     offset     |
// +--------+--------+--------+----------------+
#define _base   (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _op     (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _offset (u16)(g_cpu->m_recompiling_opcode & 0xffff)

void cache() {
}

#undef _base
#undef _op
#undef _offset

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |  base  |   ft   |     offset     |
// +--------+--------+--------+----------------+
#define _base   (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _ft     (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _offset (s16)(g_cpu->m_recompiling_opcode & 0xffff)

void swc1() {
  _va2pa();
  mov32_reg_mem(_edx, _FPR_D(_ft));
  mov32_rmem_reg(_eax, _edx);
}

void lwc1() {
  _va2pa();
  mov32_reg_rmem(_eax, _eax);
  mov32_mem_eax(_FPR_D(_ft));
}

void sqc2() {
  invalid();
}

void lqc2() {
  invalid();
}

#undef _base
#undef _ft
#undef _offset
