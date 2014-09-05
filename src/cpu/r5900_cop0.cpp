#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"

#if defined _DEBUGGER
#include "debugger/debugger.h"
#endif

#include <assert.h>
#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

void invalid();

// +--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |      0(16)     |
// |  inst  |  inst  |  inst  |     offset     |
// +--------+--------+--------+----------------+
#define _offset  (g_cpu->m_recompiling_opcode & 0x0ffff)

void bc0f() {
  // CPCOND[0]信号により分岐する
  // (現在dma転送はストア命令に付随しているため、
  //  実際にはCPCOND[0]は常にtrueとなるはず)
}

void bc0t() {
  // CPCOND[0]信号により分岐する
  // (現在dma転送はストア命令に付随しているため、
  //  実際にはCPCOND[0]は常にtrueとなるはず)
}

void bc0fl() {
  // CPCOND[0]信号により分岐する
  // (現在dma転送はストア命令に付随しているため、
  //  実際にはCPCOND[0]は常にtrueとなるはず)
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
}

void bc0tl() {
  // CPCOND[0]信号により分岐する
  // (現在dma転送はストア命令に付随しているため、
  //  実際にはCPCOND[0]は常にtrueとなるはず)
  // 分岐しない場合、次の遅延スロットの命令を実行しない
  mov8_mem_imm8((u32)&g_cpu->m_null_current_inst, 1);
}

#undef _offset

// +--------+-----------------------------------+--------+
// |  26(6) |  21(5) |           6(15)          |  0(6)  |
// |  inst  |  inst  |             0            |  inst  |
// +--------+-----------------------------------+--------+
void tlbr() {
}

void tlbwi() {
}

void tlbwr() {
}

void tlbp() {
}

#if defined _DEBUGGER
void _eret(u32 p_jaddr) {
  Debugger::callStackPop(p_jaddr);

  if (g_dbg &&
      g_dbg->step_breakpoint()->is_break() &&
      g_dbg->step_breakpoint()->va() == g_cpu->m_pc) {
    // ステップインかステップオーバーなら
    // 飛び先の先頭にステップブレークを張り替える。
    g_dbg->step_breakpoint()->breakOff();
    g_dbg->step_breakpoint()->set_va(p_jaddr);
    g_dbg->step_breakpoint()->breakOn();
  }
}
#endif

void eret() {
enum { _noerr, _end };
  mov32_reg_imm32(_eax, (u32)&g_cpu->m_cop0.status);
  tst32_rmem_imm32(_eax, COP0::STATUS_ERL);
  jz(_noerr);
  mov32_reg_mem(_edx, (u32)&g_cpu->m_cop0.errorepc);
  and32_rmem_imm32(_eax, ~COP0::STATUS_ERL);
  jmp(_end);

LABEL_DEF(_noerr);
  mov32_reg_mem(_edx, (u32)&g_cpu->m_cop0.epc);
  and32_rmem_imm32(_eax, ~COP0::STATUS_EXL);

LABEL_DEF(_end);

  // ステップ実行された場合、飛び先でブレークする
#if defined _DEBUGGER
  push_reg(_edx);  // 引数として飛び先のアドレスを渡す
  mov32_reg_imm32(_eax, (u32)_eret);
  call_reg(_eax);
  pop_reg(_edx);
#endif // defined _DEBUGGER

  mov32_mem_reg((u32)&g_cpu->m_pc, _edx);

  // eretの場合、ジャンプにm_x86_jump_addrを経由しないので
  // 飛び先のコードがコンパイル済かどうか判定し無ければ
  // リコンパイラへ処理が行われない。
  // そこでretを挟んで強制的に実行中断することによって
  // executeメソッド内でリコンパイル判定を実行される。
  ret();
}

void ei() {
enum { _set, _end };
  mov32_eax_mem((u32)&g_cpu->m_cop0.status);
  tst32_eax_imm32(COP0::STATUS_EDI | COP0::STATUS_EXL | COP0::STATUS_ERL);
  jnz(_set);
  tst32_eax_imm32(COP0::STATUS_KSU);
  jnz(_end);
LABEL_DEF(_set);
  or32_eax_imm32(COP0::STATUS_EIE);
  mov32_mem_eax((u32)&g_cpu->m_cop0.status);
LABEL_DEF(_end);
}

void di() {
enum { _set, _end };
  mov32_eax_mem((u32)&g_cpu->m_cop0.status);
  tst32_eax_imm32(COP0::STATUS_EDI | COP0::STATUS_EXL | COP0::STATUS_ERL);
  jnz(_set);
  tst32_eax_imm32(COP0::STATUS_KSU);
  jnz(_end);
LABEL_DEF(_set);
  and32_eax_imm32(~COP0::STATUS_EIE);
  mov32_mem_eax((u32)&g_cpu->m_cop0.status);
LABEL_DEF(_end);
}

// +--------+--------+--------+--------+----------------+
// |  26(6) |  21(5) |  16(5) |  11(5) |      0(11)     |
// |  inst  |  inst  |   rt   |   rd   |        0       |
// +--------+--------+--------+--------+----------------+
#define _rt    (u8)((g_cpu->m_recompiling_opcode >> 16) & 0x1f)
#define _rd    (u8)((g_cpu->m_recompiling_opcode >> 11) & 0x1f)

void mf0(int param) {
  u32* ptr = NULL;
  switch (param) {
    case 0: ptr = &g_cpu->m_cop0.cpr[_rd]; break; // mfc0
    case 1: ptr = &g_cpu->m_cop0.iab; break;      // mfiab
    case 2: ptr = &g_cpu->m_cop0.iabm; break;     // mfiabm
    case 3: ptr = &g_cpu->m_cop0.dab; break;      // mfdab
    case 4: ptr = &g_cpu->m_cop0.dabm; break;     // mfdabm
    case 5: ptr = &g_cpu->m_cop0.dvb; break;      // mfdvb
    case 6: ptr = &g_cpu->m_cop0.dvbm; break;     // mfdvbm
    case 7: ptr = &g_cpu->m_cop0.pcr0; break;     // mfpc
    case 8: ptr = &g_cpu->m_cop0.pcr1; break;     // mfpc
    case 9: ptr = &g_cpu->m_cop0.pccr; break;     // mfps
  }
  mov32_eax_mem((u32)ptr);
  mov32_mem_eax(_GPR_D(_rt, 0));
  cdq();
  mov32_mem_reg(_GPR_D(_rt, 1), _edx);
}

void mt0(int param) {
  u32* ptr = NULL;
  switch (param) {
    case 0: ptr = &g_cpu->m_cop0.cpr[_rd]; break; // mtc0
    case 1: ptr = &g_cpu->m_cop0.iab; break;      // mtiab
    case 2: ptr = &g_cpu->m_cop0.iabm; break;     // mtiabm
    case 3: ptr = &g_cpu->m_cop0.dab; break;      // mtdab
    case 4: ptr = &g_cpu->m_cop0.dabm; break;     // mfdabm
    case 5: ptr = &g_cpu->m_cop0.dvb; break;      // mfdvb
    case 6: ptr = &g_cpu->m_cop0.dvbm; break;     // mfdvbm
    case 7: ptr = &g_cpu->m_cop0.pcr0; break;     // mfpc
    case 8: ptr = &g_cpu->m_cop0.pcr1; break;     // mfpc
    case 9: ptr = &g_cpu->m_cop0.pccr; break;     // mfps
  }
  mov32_eax_mem(_GPR_D(_rt, 0));
  mov32_mem_eax((u32)ptr);
}

#undef _rt
#undef _rd

void cop0() {
  switch ((g_cpu->m_recompiling_opcode >> 21) & 0x1f) {
    case 0x00:  // mf0
      switch (g_cpu->m_recompiling_opcode & 0xffff) {
        case 0xc002: mf0(1); break;
        case 0xc003: mf0(2); break;
        case 0xc004: mf0(3); break;
        case 0xc005: mf0(4); break;
        case 0xc006: mf0(5); break;
        case 0xc007: mf0(6); break;
        case 0xc801: mf0(7); break;
        case 0xc803: mf0(8); break;
        case 0xc800: mf0(9); break;
        default:     mf0(0); break;
      }
      break;
    case 0x04:  // mt0
      switch (g_cpu->m_recompiling_opcode & 0xffff) {
        case 0xc002: mt0(1); break;
        case 0xc003: mt0(2); break;
        case 0xc004: mt0(3); break;
        case 0xc005: mt0(4); break;
        case 0xc006: mt0(5); break;
        case 0xc007: mt0(6); break;
        case 0xc801: mt0(7); break;
        case 0xc803: mt0(8); break;
        case 0xc800: mt0(9); break;
        default:     mt0(0); break;
      }
      break;
    case 0x08:  // bc0
      switch ((g_cpu->m_recompiling_opcode >> 16) & 0x1f) {
        case 0x00: bc0f(); break;    // bc0f
        case 0x01: bc0t(); break;    // bc0t
        case 0x02: bc0fl(); break;   // bc0fl
        case 0x03: bc0tl(); break;   // bc0tl
        default:   invalid(); break;
      }
      break;
    case 0x10:  // c0
      switch (g_cpu->m_recompiling_opcode & 0x3f) {
        case 0x01: tlbr(); break;    // tlbr
        case 0x02: tlbwi(); break;   // tlbwi
        case 0x06: tlbwr(); break;   // tlbwr
        case 0x08: invalid(); break; // tlbp
        case 0x18: eret(); break;    // eret
        case 0x38: ei(); break;      // ei
        case 0x39: di(); break;      // di
        default:   invalid(); break;
      }
      break;
    default: invalid(); break;
  }
}
