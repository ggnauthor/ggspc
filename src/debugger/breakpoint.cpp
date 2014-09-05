/*---------*/
/* include */
/*---------*/
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/breakpoint.h"
#include "debugger/expr_parser.h"
#include "debugger/gui/window/memory_window.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

bool BreakPointBase::evaluateExpression() {
  if (expr_[0] == '\0') return true;

  if (expr_elem_buf_ == NULL) {
    expr_elem_buf_ = new Expr_Elem[1024];
    expr_elem_count_ = lex(expr_, expr_elem_buf_, 1024);
    if (expr_elem_count_ == 0) {
      deleteExprCache();
      return true;
    }
    expr_tree_root_ = make_tree(expr_elem_buf_, expr_elem_count_);
  }
  Expr_Val result = calculate(expr_tree_root_);
  if (result.m_type == Expr_Val::kUint) {
    return result.tou() != 0;
  } else if (result.m_type == Expr_Val::kInt) {
    return result.toi() != 0;
  } else if (result.m_type == Expr_Val::kFloat) {
    return result.tof() != 0.0;
  } else {
    // 式が評価できない場合もブレークでとめる
    show_parser_error_message();
    return true;
  }
}

void BreakPointBase::deleteExprCache() {
  if (expr_elem_buf_) {
    for (int i = 0; i < expr_elem_count_; i++) {
      free(expr_elem_buf_[i].str);
    }
    delete[] expr_elem_buf_;
    expr_elem_buf_ = NULL;
    expr_elem_count_ = 0;
    expr_tree_root_ = NULL;
  }
}

bool BreakPointBase::is_cur() {
  if (g_dbg->cur_ddb_idx_ < 0) return false;
  return m_owner == NULL || m_owner == g_dbg->get_current_ddb();
}

void CodeBreakPoint::breakOn() {
  // ステップでコールバックを抜けるときはブレーク張れないので無視する
  if (va_ & 0x80000000) return;
  if (!is_cur()) return;

  if (!is_break_) {
    g_cpu->enable_break(va_, &save_byte_);
    is_break_ = true;
  }
}

void CodeBreakPoint::breakOff() {
  // ステップでコールバックを抜けるときはブレーク張れないので無視する
  if (va_ & 0x80000000) return;

  if (is_break_) {
    g_cpu->disable_break(va_, save_byte_);
    is_break_ = false;
  }
}

void MemoryBreakPoint::breakOn() {
  if (!is_cur()) return;

  if (!is_break_) {
    is_break_ = true;
  }
}

void MemoryBreakPoint::breakOff() {
  if (is_break_) {
    is_break_ = false;
  }
}

u32 MemoryBreakPoint::checkBreak() {
  if (g_dbg->without_debugger_inspection()) return 0;

  u32 opcode = *((u32*)VA2PA(g_cpu->m_pc));
  int write = -1, mode = -1, isize = -1;
  switch (opcode >> 26) {
    case 0x1a: { write = 0; mode = 1; isize =  8; break; } // ldl
    case 0x1b: { write = 0; mode = 2; isize =  8; break; } // ldr
    case 0x1e: { write = 0; mode = 0; isize = 16; break; } // lq
    case 0x1f: { write = 1; mode = 0; isize = 16; break; } // sq
    case 0x20: { write = 0; mode = 0; isize =  1; break; } // lb
    case 0x21: { write = 0; mode = 0; isize =  2; break; } // lh
    case 0x22: { write = 0; mode = 1; isize =  4; break; } // lwl
    case 0x23: { write = 0; mode = 0; isize =  4; break; } // lw
    case 0x24: { write = 0; mode = 0; isize =  1; break; } // lbu
    case 0x25: { write = 0; mode = 0; isize =  2; break; } // lhu
    case 0x26: { write = 0; mode = 2; isize =  4; break; } // lwr
    case 0x27: { write = 0; mode = 0; isize =  4; break; } // lwu
    case 0x28: { write = 1; mode = 0; isize =  1; break; } // sb
    case 0x29: { write = 1; mode = 0; isize =  2; break; } // sh
    case 0x2a: { write = 1; mode = 1; isize =  4; break; } // swl
    case 0x2b: { write = 1; mode = 0; isize =  4; break; } // sw
    case 0x2c: { write = 1; mode = 1; isize =  8; break; } // sdl
    case 0x2d: { write = 1; mode = 2; isize =  8; break; } // sdr
    case 0x2e: { write = 1; mode = 2; isize =  8; break; } // swr
    case 0x31: { write = 0; mode = 0; isize =  4; break; } // lwc1
    case 0x37: { write = 0; mode = 0; isize =  8; break; } // ld
    case 0x39: { write = 1; mode = 0; isize =  4; break; } // swc1
    case 0x3f: { write = 1; mode = 0; isize =  8; break; } // sd
  }

  if (mode != -1) {
    u32 va = g_cpu->m_gpr[(opcode >> 21) & 0x1f].d[0] + (s16)(opcode & 0xffff);
    if (opcode == 0x1e || opcode == 0x1f) {
      va &= 0xfffffff0;  // lq,sqなら下位4bitをマスク
    }
    int st, size;
    switch (mode) {
      case 0:
        size = isize;
        st = 0;
        break;
      case 1:
        size = va % isize + 1;
        st = 1 - size;
        break;
      case 2:
        size = isize - va % isize;
        st = 0;
        break;
    }

    // メモリウインドウでスクロールイン
    if (g_dbg->memory_window()->jump_ref_addr()) {
      g_dbg->memory_window()->SelectAddress(va, false);
    }

    // カレントDDBのメモリブレークと接触したら実行する前に停止
    for (int i = 0; i < g_dbg->get_breakpoint_count(); i++) {
      if (g_dbg->get_breakpoint(i)->enable() == false) continue;
      if (g_dbg->get_breakpoint(i)->is_cur() == false) continue;
      if (g_dbg->get_breakpoint(i)->is_break() == false) continue;
      if (g_dbg->get_breakpoint(i)->type() != kType_Memory) continue;
      
      MemoryBreakPoint* membreak = dynamic_cast<MemoryBreakPoint*>(g_dbg->get_breakpoint(i));

      // 参照メモリがブレーク範囲外
      u32 pa1s = VA2PA(va + st);
      u32 pa1e = VA2PA(va + st + size - 1);
      u32 pa2s = VA2PA(membreak->va());
      u32 pa2e = VA2PA(membreak->va() + membreak->length());
      if (pa1s <  pa2s && pa1e <  pa2s) continue;
      if (pa1s >= pa2e && pa1e >= pa2e) continue;
      
      if (write == 0 && membreak->read_enable() ||
          write == 1 && membreak->write_enable()) {
        DBGOUT_MEM("0x%08x[%d]の書き込みでブレークポイントを発見しました。処理を中断します。\n", va + st, size);
        g_dbg->hit_mem_break_ = true;
        g_dbg->hit_mem_break_va_ = membreak->va();
        return 1;
      }
    }

    // 始点～終点が有効なアドレスか？
    if (VA2PA(va + st) && VA2PA(va + st + size - 1)) {
      static char modestr[2][3][16] = {
        "READ",  "READ.L",  "READ.R",
        "WRITE", "WRITE.L", "WRITE.R"
      };

      char str[1024];
      sprintf(str, "0x%08x[%d]を %s します。(0x", va + st, size, modestr[write][mode]);
      for (int i = 0; i < size; i++) {
        char tmp[32];
        sprintf(tmp, "%02x", ((u8*)VA2PA(va))[st + i]);
        strcat(str, tmp);
      }
      DBGOUT_MEM("%s)\n", str);
    } else {
      DBGOUT_MEM("無効なアドレス0x%08x[%d]にアクセスしました！\n", va, size);
      g_dbg->hit_mem_break_ = true;
      g_dbg->hit_mem_break_va_ = va;
      return 1;  // このまま実行すると落ちてしまうので中断
    }
  }
  return 0;
}
