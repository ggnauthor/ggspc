#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"
#include "memory/memory.h"
#include "thread/thread.h"
#include "device/device.h"
#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif // _DEBUGGER

#include <assert.h>

extern PS2ThreadMgr*    thread_mgr_;

void invalid();

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(10) |  0(6)  |
// |  inst  |   rs   |   rt   |   rd   |   sa   |  inst  |
// +--------+-----------------+--------+--------+--------+
#define _rs  (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt  (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _rd  (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)
#define _sa  (u8)((g_cpu->m_recompiling_opcode >> 6) & 0x1f)

void sll() {
  // nopとして使用されることが多いためその場合はコード生成しない
  if (g_cpu->m_recompiling_opcode != 0x00000000) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    shl32_reg_imm8(_eax, _sa);
    mov32_mem_eax(_GPR_D(_rd, 0));
    cdq();
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
}

void srl() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  shr32_reg_imm8(_eax, _sa);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void sra() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  sar32_reg_imm8(_eax, _sa);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void sllv() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  and32_reg_imm8(_ecx, 0x1f);
  shl32_reg_cl(_eax);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void srlv() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  and32_reg_imm8(_ecx, 0x1f);
  shr32_reg_cl(_eax);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void srav() {
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  and32_reg_imm8(_ecx, 0x1f);
  sar32_reg_cl(_eax);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void jr() {
//  g_cpu->m_ee_jump_addr = g_cpu->m_gpr[_rs].d[0];
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPop);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif
}

void jalr() {
// delayslot = rs.0-31
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_ee_jump_addr);
// rd.0-63 = instaddr + 8;
  mov32_mem_imm32(_GPR_D(_rd, 0), g_cpu->m_recompiling_ee_va + 8);
  mov32_mem_imm32(_GPR_D(_rd, 1), 0x00000000);

#if defined _DEBUGGER
  push_reg(_eax);  // eaxはジャンプ先
  mov32_reg_imm32(_eax, (u32)Debugger::callStackPush);
  call_reg(_eax);
  pop_reg(_eax);

  mov32_reg_imm32(_eax, (u32)Debugger::jumpLogPush);
  call_reg(_eax);
#endif
}

void sync() {
}

void movz() {
enum { _end };
  tst32_mem_imm32(_GPR_D(_rt, 0), 0xffffffff);
  jnz(_end);
  tst32_mem_imm32(_GPR_D(_rt, 1), 0xffffffff);
  jnz(_end);
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));
LABEL_DEF(_end);
}

void movn() {
enum { _mov, _end };
  tst32_mem_imm32(_GPR_D(_rt, 0), 0xffffffff);
  jnz(_mov);
  tst32_mem_imm32(_GPR_D(_rt, 1), 0xffffffff);
  jz(_end);
LABEL_DEF(_mov);
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));
LABEL_DEF(_end);
}

void _syscall() {
  // EEから呼ばれて内部でリコンパイルを行う可能性のあるものは
  // リコンパイルに伴うコードバッファのrealloc時にアドレスを書き換えるため
  u32* fp;
  _asm mov fp, ebp;
  thread_mgr_->cur_thread()->push_x86_retaddr(fp + 1);
  g_dev_emu->handleSyscall(g_cpu->m_v1.w[0] & 0xffff);
  thread_mgr_->cur_thread()->pop_x86_retaddr();
}

void syscall() {
  mov32_reg_imm32(_eax, (u32)_syscall);
  call_reg(_eax);
  ret();
}

void brk() {
}

void mfhi() {
  mov32_eax_mem((u32)&g_cpu->m_hi.d[0]);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem((u32)&g_cpu->m_hi.d[1]);
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void mthi() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_hi.d[0]);
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax((u32)&g_cpu->m_hi.d[1]);
}

void mflo() {
  mov32_eax_mem((u32)&g_cpu->m_lo.d[0]);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem((u32)&g_cpu->m_lo.d[1]);
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void mtlo() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_lo.d[0]);
  mov32_eax_mem(_GPR_D(_rs, 1));
  mov32_mem_eax((u32)&g_cpu->m_lo.d[1]);
}

void dsllv() {
  // _rd.0-63 = _rt.0-63 << _rs.0-5
  enum { _sa0, _sa32over, _end };
  
  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  tst8_reg_imm8(_cl, 0x3f);
  mov32_eax_mem(_GPR_D(_rt, 0));
  jz(_sa0);
  tst8_reg_imm8(_cl, 0x20);
  jnz(_sa32over);
  
  and32_reg_imm8(_ecx, 0x1f);
  mov32_reg_reg(_edx, _eax);
  
  shl32_reg_cl(_edx);
  mov32_mem_reg(_GPR_D(_rd, 0), _edx);
  mov32_reg_mem(_edx, _GPR_D(_rt, 1));
  shl32_reg_cl(_edx);

  sub32_reg_imm8(_ecx, 32);
  neg32(_ecx);
  shr32_reg_cl(_eax);
  
  or32_reg_reg(_eax, _edx);
  mov32_mem_eax(_GPR_D(_rd, 1));
  jmp(_end);
LABEL_DEF(_sa0);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_eax_mem(_GPR_D(_rt, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));
  jmp(_end);
LABEL_DEF(_sa32over);
  and32_reg_imm8(_ecx, 0x1f);
  xor32_reg_reg(_edx, _edx);
  mov32_mem_reg(_GPR_D(_rd, 0), _edx);
  shl32_reg_cl(_eax);
  mov32_mem_eax(_GPR_D(_rd, 1));
LABEL_DEF(_end);
}

void dsrlv() {
  // _rd.0-63 = _rt.0-63 >> _rs.0-5
  enum { _sa0, _sa32over, _end };
  
  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  tst8_reg_imm8(_cl, 0x3f);
  mov32_eax_mem(_GPR_D(_rt, 1));
  jz(_sa0);
  tst8_reg_imm8(_cl, 0x20);
  jnz(_sa32over);
  
  and32_reg_imm8(_ecx, 0x1f);
  mov32_reg_reg(_edx, _eax);
  
  shr32_reg_cl(_edx);
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  mov32_reg_mem(_edx, _GPR_D(_rt, 0));
  shr32_reg_cl(_edx);

  sub32_reg_imm8(_ecx, 32);
  neg32(_ecx);
  shl32_reg_cl(_eax);
  
  or32_reg_reg(_eax, _edx);
  mov32_mem_eax(_GPR_D(_rd, 0));
  jmp(_end);
LABEL_DEF(_sa0);
  mov32_mem_eax(_GPR_D(_rd, 1));
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  jmp(_end);
LABEL_DEF(_sa32over);
  and32_reg_imm8(_ecx, 0x1f);
  xor32_reg_reg(_edx, _edx);
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  shr32_reg_cl(_eax);
  mov32_mem_eax(_GPR_D(_rd, 0));
LABEL_DEF(_end);
}

void dsrav() {
  // _rd.0-63 = _rt.0-63 >> _rs.0-5
  enum { _sa0, _sa32over, _end };
  
  mov32_reg_mem(_ecx, _GPR_D(_rs, 0));
  tst8_reg_imm8(_cl, 0x3f);
  mov32_eax_mem(_GPR_D(_rt, 1));
  jz(_sa0);
  tst8_reg_imm8(_cl, 0x20);
  jnz(_sa32over);
  
  and32_reg_imm8(_ecx, 0x1f);
  mov32_reg_reg(_edx, _eax);
  
  sar32_reg_cl(_edx);
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  mov32_reg_mem(_edx, _GPR_D(_rt, 0));
  shr32_reg_cl(_edx);

  sub32_reg_imm8(_ecx, 32);
  neg32(_ecx);
  shl32_reg_cl(_eax);
  
  or32_reg_reg(_eax, _edx);
  mov32_mem_eax(_GPR_D(_rd, 0));
  jmp(_end);
LABEL_DEF(_sa0);
  mov32_mem_eax(_GPR_D(_rd, 1));
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  jmp(_end);
LABEL_DEF(_sa32over);
  and32_reg_imm8(_ecx, 0x1f);
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  sar32_reg_cl(_eax);
  mov32_mem_eax(_GPR_D(_rd, 0));
LABEL_DEF(_end);
}

void mult() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  imul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[1], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[1], _edx);
}

void multu() {
  mov32_eax_mem(_GPR_D(_rs, 0));
  mul_mem(_GPR_D(_rt, 0));
  mov32_reg_reg(_ecx, _edx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_lo.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[1], _edx);
  if (_rd != 0) {
    mov32_mem_reg(_GPR_D(_rd, 0), _eax);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
  mov32_reg_reg(_eax, _ecx);
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[1], _edx);
}

void div() {
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
  mov32_mem_reg((u32)&g_cpu->m_lo.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[1], _edx);

  mov32_reg_reg(_eax, _ecx);
  // 余をhiに格納
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[1], _edx);
LABEL_DEF(_end);
}

void divu() {
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
  mov32_mem_reg((u32)&g_cpu->m_lo.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_lo.d[1], _edx);

  mov32_reg_reg(_eax, _ecx);
  // 余をhiに格納
  cdq();
  mov32_mem_reg((u32)&g_cpu->m_hi.d[0], _eax);
  mov32_mem_reg((u32)&g_cpu->m_hi.d[1], _edx);
LABEL_DEF(_end);
}

void add() {
  // _rt.0-63 = sx64(_rs.0-31 + _rt.0-31)
  enum { _overflow, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  add32_reg_mem(_eax, _GPR_D(_rt, 0));
  jo(_overflow);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
LABEL_DEF(_overflow);
  // Signal Exception発生
  // rtレジスタに変更は行わない
}

void addu() {
  // _rt.0-63 = sx64(_rs.0-31 + _rt.0-31)

  mov32_eax_mem(_GPR_D(_rs, 0));
  add32_reg_mem(_eax, _GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void sub() {
  // _rt.0-63 = sx64(_rs.0-31 - _rt.0-31)
  enum { _overflow, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  sub32_reg_mem(_eax, _GPR_D(_rt, 0));
  jo(_overflow);
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
LABEL_DEF(_overflow);
  // Signal Exception発生
  // rtレジスタに変更は行わない
}

void subu() {
  // _rt.0-63 = sx64(_rs.0-31 - _rt.0-31)

  mov32_eax_mem(_GPR_D(_rs, 0));
  sub32_reg_mem(_eax, _GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

void and() {
  // _rd.0-63 = _rs.0-63 & _rt.0-63

  mov32_eax_mem(_GPR_D(_rs, 0));
  and32_reg_mem(_eax, _GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));

  mov32_eax_mem(_GPR_D(_rs, 1));
  and32_reg_mem(_eax, _GPR_D(_rt, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void or() {
  // _rd.0-63 = _rs.0-63 | _rt.0-63

  mov32_eax_mem(_GPR_D(_rs, 0));
  or32_reg_mem(_eax, _GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));

  mov32_eax_mem(_GPR_D(_rs, 1));
  or32_reg_mem(_eax, _GPR_D(_rt, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void xor() {
  // _rd.0-63 = _rs.0-63 | _rt.0-63

  mov32_eax_mem(_GPR_D(_rs, 0));
  xor32_reg_mem(_eax, _GPR_D(_rt, 0));
  mov32_mem_eax(_GPR_D(_rd, 0));

  mov32_eax_mem(_GPR_D(_rs, 1));
  xor32_reg_mem(_eax, _GPR_D(_rt, 1));
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void nor() {
  // _rd.0-63 = _rs.0-63 | _rt.0-63
  
  mov32_eax_mem(_GPR_D(_rs, 0));
  or32_reg_mem(_eax, _GPR_D(_rt, 0));
  not32(_eax);
  mov32_mem_eax(_GPR_D(_rd, 0));

  mov32_eax_mem(_GPR_D(_rs, 1));
  or32_reg_mem(_eax, _GPR_D(_rt, 1));
  not32(_eax);
  mov32_mem_eax(_GPR_D(_rd, 1));
}

void mfsa() {
  // 汎用レジスタにSAレジスタの内容を退避する
  mov32_eax_mem((u32)&g_cpu->m_sa);
  mov32_mem_eax(_GPR_D(_rd, 0));
}

void mtsa() {
  // 汎用レジスタからSAレジスタへ内容を復帰する
  mov32_eax_mem(_GPR_D(_rs, 0));
  mov32_mem_eax((u32)&g_cpu->m_sa);
}

void slt() {
  enum { _ne, _store };

  xor32_reg_reg(_ecx, _ecx);
  mov32_eax_mem(_GPR_D(_rs, 1));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 1));
  mov32_mem_reg(_GPR_D(_rd, 1), _ecx);
  jnz(_ne);
  // さらに下位wordの比較を行う
  mov32_eax_mem(_GPR_D(_rs, 0));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 0));
  setb(_cl);
  jmp(_store);
LABEL_DEF(_ne);
  setl(_cl);
LABEL_DEF(_store);
  mov32_mem_reg(_GPR_D(_rd, 0), _ecx);
}

void sltu() {
  enum { _ne, _store };

  xor32_reg_reg(_ecx, _ecx);
  mov32_eax_mem(_GPR_D(_rs, 1));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 1));
  mov32_mem_reg(_GPR_D(_rd, 1), _ecx);
  jnz(_ne);
  // さらに下位wordの比較を行う
  mov32_eax_mem(_GPR_D(_rs, 0));
  cmp32_reg_mem(_eax, _GPR_D(_rt, 0));
LABEL_DEF(_ne);
  setb(_cl);
  mov32_mem_reg(_GPR_D(_rd, 0), _ecx);
}

void dadd() {
  // _rd.0-63 = _rs.0-63 + _rt.0-63
  enum { _overflow, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  add32_reg_mem(_eax, _GPR_D(_rt, 0));

  mov32_reg_mem(_ecx, _GPR_D(_rs, 1));
  adc32_reg_mem(_ecx, _GPR_D(_rt, 1));

  jo(_overflow);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_mem_reg(_GPR_D(_rd, 1), _ecx);
  //jmp(_end);
LABEL_DEF(_overflow);
  // Signal Exception発生
  // rtレジスタに変更は行わない
//LABEL_DEF(_end);
}

void daddu() {
  // _rd.0-63 = _rs.0-63 + _rt.0-63

  mov32_eax_mem(_GPR_D(_rs, 0));
  add32_reg_mem(_eax, _GPR_D(_rt, 0));

  mov32_reg_mem(_ecx, _GPR_D(_rs, 1));
  adc32_reg_mem(_ecx, _GPR_D(_rt, 1));

  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_mem_reg(_GPR_D(_rd, 1), _ecx);
}

void dsub() {
  // _rd.0-63 = _rs.0-63 - _rt.0-63
  enum { _overflow, _end };

  mov32_eax_mem(_GPR_D(_rs, 0));
  sub32_reg_mem(_eax, _GPR_D(_rt, 0));

  mov32_reg_mem(_ecx, _GPR_D(_rs, 1));
  sbb32_reg_mem(_ecx, _GPR_D(_rt, 1));

  jo(_overflow);
  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_mem_reg(_GPR_D(_rd, 1), _ecx);
  //jmp(_end);
LABEL_DEF(_overflow);
  // Signal Exception発生
  // rtレジスタに変更は行わない
//LABEL_DEF(_end);
}

void dsubu() {
  // _rd.0-63 = _rs.0-63 - _rt.0-63

  mov32_eax_mem(_GPR_D(_rs, 0));
  sub32_reg_mem(_eax, _GPR_D(_rt, 0));

  mov32_reg_mem(_ecx, _GPR_D(_rs, 1));
  sbb32_reg_mem(_ecx, _GPR_D(_rt, 1));

  mov32_mem_eax(_GPR_D(_rd, 0));
  mov32_mem_reg(_GPR_D(_rd, 1), _ecx);
}

#undef _rs
#undef _rt
#undef _rd

// +--------+--------+--------+-----------------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(5)  |  0(6)  |
// |  inst  |    0   |   rt   |   rd   |   sa   |  inst  |
// +--------+--------+--------+-----------------+--------+
#define _rt    (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _rd    (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)
#define _sa    (u8)((g_cpu->m_recompiling_opcode >>  6) & 0x1f)

void dsll() {
  // _rd.0-63 = _rt.0-63 << _sa

  if (_sa == 0) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    mov32_mem_eax(_GPR_D(_rd, 0));
    mov32_eax_mem(_GPR_D(_rt, 1));
    mov32_mem_eax(_GPR_D(_rd, 1));
  } else {
    mov32_reg_mem(_ecx, _GPR_D(_rt, 0));
    mov32_reg_reg(_edx, _ecx);
    shl32_reg_imm8(_edx, _sa);
    mov32_mem_reg(_GPR_D(_rd, 0), _edx);
    shr32_reg_imm8(_ecx, 32 - _sa);
    mov32_reg_mem(_edx, _GPR_D(_rt, 1));
    shl32_reg_imm8(_edx, _sa);
    or32_reg_reg(_edx, _ecx);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
  }
}

void dsll32() {
  // _rd.0-63 = _rt.0-63 << (_sa + 32)

  if (_sa == 0) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    mov32_mem_eax(_GPR_D(_rd, 1));
  } else {
    mov32_eax_mem(_GPR_D(_rt, 0));
    shl32_reg_imm8(_eax, _sa);
    mov32_mem_eax(_GPR_D(_rd, 1));
  }
  mov32_mem_imm32(_GPR_D(_rd, 0), 0);
}

void dsrl() {
  // _rd.0-63 = _rt.0-63 >> _sa

  if (_sa == 0) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    mov32_mem_eax(_GPR_D(_rd, 0));
    mov32_eax_mem(_GPR_D(_rt, 1));
    mov32_mem_eax(_GPR_D(_rd, 1));
  } else {
    mov32_reg_mem(_ecx, _GPR_D(_rt, 1));
    mov32_reg_reg(_edx, _ecx);
    shr32_reg_imm8(_edx, _sa);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
    shl32_reg_imm8(_ecx, 32 - _sa);
    mov32_reg_mem(_edx, _GPR_D(_rt, 0));
    shr32_reg_imm8(_edx, _sa);
    or32_reg_reg(_edx, _ecx);
    mov32_mem_reg(_GPR_D(_rd, 0), _edx);
  }
}

void dsrl32() {
  // _rd.0-63 = _rt.0-63 >> (_sa + 32)

  if (_sa == 0) {
    mov32_eax_mem(_GPR_D(_rt, 1));
    mov32_mem_eax(_GPR_D(_rd, 0));
  } else {
    mov32_eax_mem(_GPR_D(_rt, 1));
    shr32_reg_imm8(_eax, _sa);
    mov32_mem_eax(_GPR_D(_rd, 0));
  }
  mov32_mem_imm32(_GPR_D(_rd, 1), 0);
}

void dsra() {
  // _rd.0-63 = _rt.0-63 >> _sa

  if (_sa == 0) {
    mov32_eax_mem(_GPR_D(_rt, 0));
    mov32_mem_eax(_GPR_D(_rd, 0));
    mov32_eax_mem(_GPR_D(_rt, 1));
    mov32_mem_eax(_GPR_D(_rd, 1));
  } else {
    mov32_reg_mem(_ecx, _GPR_D(_rt, 1));
    mov32_reg_reg(_edx, _ecx);
    sar32_reg_imm8(_edx, _sa);
    mov32_mem_reg(_GPR_D(_rd, 1), _edx);
    shl32_reg_imm8(_ecx, 32 - _sa);
    mov32_reg_mem(_edx, _GPR_D(_rt, 0));
    shr32_reg_imm8(_edx, _sa);
    or32_reg_reg(_edx, _ecx);
    mov32_mem_reg(_GPR_D(_rd, 0), _edx);
  }
}

void dsra32() {
  // _rd.0-63 = _rt.0-63 >> (_sa + 32)

  if (_sa == 0) {
    mov32_eax_mem(_GPR_D(_rt, 1));
    mov32_mem_eax(_GPR_D(_rd, 0));
  } else {
    mov32_eax_mem(_GPR_D(_rt, 1));
    sar32_reg_imm8(_eax, _sa);
    mov32_mem_eax(_GPR_D(_rd, 0));
  }
  cdq();
  mov32_mem_reg(_GPR_D(_rd, 1), _edx);
}

#undef _rt
#undef _rd
#undef _sa

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(10) |  0(6)  |
// |  inst  |   rs   |   rt   |   rd   |   sa   |  inst  |
// +--------+-----------------+--------+--------+--------+
#define _rs    (u8)((g_cpu->m_recompiling_opcode >> 21) & 0x1f)
#define _rt    (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _code  (u16)((g_cpu->m_recompiling_opcode >> 6) & 0xffff)

void tge() {
  invalid();
}

void tgeu() {
  invalid();
}

void tlt() {
  invalid();
}

void tltu() {
  invalid();
}

void teq() {
  invalid();
}

void tne() {
  invalid();
}

#undef _rs
#undef _rt
#undef _code

void special() {
  static void (*TABLE[0x40])() = {
    sll,  invalid, srl,  sra,  sllv,    invalid, srlv,    srav,
    jr,   jalr,    movz, movn, syscall, brk,     invalid, sync,
    mfhi, mthi,    mflo, mtlo, dsllv,   invalid, dsrlv,   dsrav,
    mult, multu,   div,  divu, invalid, invalid, invalid, invalid,
    add,  addu,    sub,  subu, and,     or,      xor,     nor,
    mfsa, mtsa,    slt,  sltu, dadd,    daddu,   dsub,    dsubu,
    tge,  tgeu,    tlt,  tltu, teq,     invalid, tne,     invalid,
    dsll, invalid, dsrl, dsra, dsll32,  invalid, dsrl32,  dsra32,
  };
  TABLE[g_cpu->m_recompiling_opcode & 0x3f]();
}
