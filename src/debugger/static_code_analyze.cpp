#include "debugger/static_code_analyze.h"

#include "main.h"
#include "cpu/r5900.h"
#include "memory/memory.h"
#include "debugger/debugger.h"
#include "debugger/analyze_info.h"
#include "debugger/source_info.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

class AnalyzeContext* const kAnalyzeContext_Unfixed  = (class AnalyzeContext*)0xffffffff;
class AnalyzeContext* const kAnalyzeContext_Fixed  = (class AnalyzeContext*)0;

AnalyzeContext::AnalyzeContext(AnalyzeContext& p_org) {
  pc_ = p_org.pc_;
  jump_addr_[0] = p_org.jump_addr_[0];
  jump_addr_[1] = p_org.jump_addr_[1];
  memcpy(gpr_, p_org.gpr_, sizeof(u32) * 32);
}

void AnalyzeContext::ClearGPR(void) {
  memset(gpr_, 0x00, sizeof(u32) * 32);
}

void AnalyzeContext::Init(u32 p_entry) {
  memset(gpr_, 0x00, sizeof(u32) * 32);
  pc_ = p_entry;
  jump_addr_[0] = 0xffffffff;
  jump_addr_[1] = 0xffffffff;
  while (!branch_stack_.empty()) {
    delete branch_stack_.top();
    branch_stack_.pop();
  }
}

void AnalyzeContext::AnalyzeFragment(
    hash_map<int, AnalyzeInfo>& p_ana_inf_map,
    bool p_enable) {
  // 関数レベルで解析する
  while (1) {
    //u32 idx = (pc_ - p_module_base) / 4;

    AnalyzeInfoMapItr itr = p_ana_inf_map.find(pc_);
    if (itr == g_dbg->analyze_map_.end()) {
      p_ana_inf_map.insert(std::pair<int, AnalyzeInfo>(pc_, AnalyzeInfo()));
      itr = p_ana_inf_map.find(pc_);
    }

    // リターンか解析済コードに到達したら
    if (pc_ == 0xfffffffe || // return
        (
          jump_addr_[1] == 0xffffffff &&
          p_ana_inf_map.find(pc_) != p_ana_inf_map.end() &&
          p_ana_inf_map.find(pc_ + 4) != p_ana_inf_map.end()
        )) {
      // ローカル分岐をすべて処理してからリターン
      while (!branch_stack_.empty()) {
        AnalyzeContext* ctx = branch_stack_.top();
        ctx->AnalyzeFragment(p_ana_inf_map, p_enable);
        delete ctx;
        branch_stack_.pop();
      }
      return;
    }

    u32 opcode = *((u32*)VA2PA(pc_));
    u32 rd     = opcode >> 11 & 0x1f;
    u32 rs     = opcode >> 21 & 0x1f;
    u32 rt     = opcode >> 16 & 0x1f;
    u32 sa     = opcode >>  6 & 0x1f;
    u32 imm    = (u16)(opcode & 0xffff);

    // 解析済フラグを立てる
    itr->second.NoEstimate();

    // 使用フラグを立てる
    if (p_enable) {
      itr->second.set_enable(true);
    }

    u32 inst1 = opcode >> 26;
    switch (inst1) {
    case 0x00:  // special
      switch (opcode & 0x3f) {
      case 0x00:  // sll
        gpr_[rd] = gpr_[rt] << sa;
        break;
      case 0x02:  // srl
      case 0x03:  // sra
        gpr_[rd] = gpr_[rt] >> sa;
        break;
      case 0x04:  // sll
        gpr_[rd] = gpr_[rt] << gpr_[rs];
        break;
      case 0x06:  // srl
      case 0x07:  // sra
        gpr_[rd] = gpr_[rt] >> gpr_[rs];
        break;
      case 0x08:  // jr
        if (rs == 0x1f) {
          jump_addr_[0] = 0xfffffffe;  // 0xfffffffeはリターン
        } else {
#define _rs(_op)    ((_op) >> 21 & 0x1f)
#define _rbase(_op) ((_op) >> 21 & 0x1f)
#define _rt(_op)    ((_op) >> 16 & 0x1f)
#define _rd(_op)    ((_op) >> 11 & 0x1f)
#define _spc(_op)   ((_op) & 0x3f)
#define _inst(_op)  ((_op) >> 26)
#define _sa(_op)    ((_op) >> 6 & 0x1f)
          // switch文かもしれないのでパターン照合する(多分CodeWarrior限定)
          bool is_switch = false;
          do {
            int i;
            u32 op;

            // lw
            op = *((u32*)VA2PA(pc_ - 4));
            if (_inst(op) != 0x23) break;
            u32 lw_addr = pc_ - 4;
            u32 lw_base = _rbase(op);
            u32 lw_imm  = (op & 0xffff);
            u32 lw_rt   = _rt(op);

            // addu
            op = *((u32*)VA2PA(lw_addr - 4));
            if (_inst(op) != 0x00 || _spc(op) != 0x21) break;
            u32 addu_addr = lw_addr - 4;
            u32 addu_rd   = _rd(op);
            u32 addu_rs   = _rs(op);
            u32 addu_rt   = _rt(op);

            // addiu (なくてもいい)
            u32 addiu_addr = addu_addr - 4;
            for (i = 0; i < 3; i++) {
              op = *((u32*)VA2PA(addiu_addr));
              if (_inst(op) == 0x09) {
                break;
              }
              
              if (i == 2) {
                addiu_addr = 0xffffffff;
              } else {
                addiu_addr -= 4;
              }
            }
            u32 addiu_rt  = _rt(op);
            u32 addiu_rs  = _rs(op);
            u32 addiu_imm = (op & 0xffff);

            // lui
            u32 lui_addr = addu_addr - 4;
            for (i = 0; i < 3; i++) {  // アドレス固定ではないので余裕を持たせる。luiとsllはswap可能
              op = *((u32*)VA2PA(lui_addr));
              if (_inst(op) == 0x0f) {
                break;
              }
              lui_addr -= 4;
            }
            if (i == 3) break;
            u32 lui_rt  = _rt(op);
            u32 lui_imm = (op & 0xffff);

            // sll
            u32 sll_addr = addu_addr - 4;
            for (i = 0; i < 3; i++) {  // アドレス固定ではないので余裕を持たせる。luiとsllはswap可能
              op = *((u32*)VA2PA(sll_addr));
              if (_inst(op) == 0x00 && _spc(op) == 0x00 && _sa(op) == 2) {
                break;
              }
              sll_addr -= 4;
            }
            if (i == 3) break;
            u32 sll_rd = _rd(op);
            u32 sll_rt = _rt(op);

            // beq/beql
            u32 beq_addr = (lui_addr < sll_addr ? lui_addr : sll_addr) - 4;
            for (i = 0; i < 3; i++) {  // アドレス固定ではないので余裕を持たせる
              op = *((u32*)VA2PA(beq_addr));
              if ((_inst(op) == 0x04 || _inst(op) == 0x14) && _rt(op) == 0) {
                break;
              }
              beq_addr -= 4;
            }
            if (i == 3) break;
            u32 beq_rs  = _rs(op);
            u32  beq_ofs = (op & 0xffff);

            // sltiu
            u32 sltiu_addr = beq_addr - 4;
            for (i = 0; i < 32; i++) {  // アドレス固定ではないので余裕を持たせる
              op = *((u32*)VA2PA(sltiu_addr));
              if (_inst(op) == 0x0b) {
                break;
              }
              sltiu_addr -= 4;
            }
            if (i == 32) break;
            u32 sltiu_rt  = _rt(op);
            u32 sltiu_rs  = _rs(op);
            u32 sltiu_imm = (op & 0xffff);

            if (lw_rt != rs) {
              break;
            }
            if (lw_base != addu_rd) {
              break;
            }
            if (addu_rs != lui_rt && addu_rt != lui_rt) {
              break;
            }
            if (addu_rs != sll_rd && addu_rt != sll_rd) {
              break;
            }
            if (sll_rt != sltiu_rs) {
              break;
            }
            if (sltiu_rt != beq_rs) {
              break;
            }

            // estimate値にまともな値を期待するのは無理があるので
            // 別途テーブルのアドレスを計算する
            u32 tab_head = (lui_imm << 16) + (s16)lw_imm;
            if (addiu_addr != 0xffffffff && addiu_addr > sll_addr) {
              tab_head += (s16)addiu_imm;
            }

            DBGOUT_ANL("switch発見 %08x table=%08x count=%d\n", pc_, tab_head, sltiu_imm);
            
            u32 default_addr = beq_addr + 4 + (s16)beq_ofs * 4;

            bool* chkary = (bool*)malloc(sltiu_imm + 1);
            memset(chkary, 0, sizeof(bool) * sltiu_imm + 1);

            s8 tmp[256];
            for (u32 i = 0; i < sltiu_imm; i++) {
              u32* addrptr = (u32*)VA2PA(tab_head + i * 4);
              
              if (addrptr == NULL || g_dbg->isInModuleCodeSection(*addrptr) == false) {
                DBGOUT_ANL("  case(%d)の分岐先が不正です\n", i);
                continue;
              }
              Branch(*addrptr);

              // 分岐先にシンボルとしてcaseラベルをつける
              s8 str[1024] = "";
              for (u32 j = i; j < sltiu_imm; j++) {
                if (chkary[j]) continue;
                u32* addrptr2 = (u32*)VA2PA(tab_head + j * 4);
                if (addrptr2 == NULL) continue;
                if (addrptr != addrptr2) continue;

                if (str[0] == '\0') {
                  sprintf(str, "case %d", j);
                } else {
                  sprintf(tmp, ", %d", j);
                  strcat(str, tmp);
                }
                chkary[j] = true;
              }
              if (chkary[sltiu_imm] == false && *addrptr == default_addr) {
                if (str[0] == '\0') {
                  strcpy(str, "case default");
                } else {
                  strcat(str, ", default");
                }
                chkary[sltiu_imm] = true;
              }
              if (str[0]) {
                sprintf(tmp, " of switch %08x", pc_);
                strcat(str, tmp);
                SymbolMapItr itr = g_dbg->symbol_map_.find(*addrptr);
                if (itr != g_dbg->symbol_map_.end()) {
                  delete itr->second;
                  g_dbg->symbol_map_.erase(itr);
                }
                Symbol* symbol = new Symbol(*addrptr, str);
                g_dbg->symbol_map_.insert(SymbolMapValue(*addrptr, symbol));
                tmp[0] = '\0';
              }
            }
            if (chkary[sltiu_imm] == false) {
              sprintf(tmp, "case default of switch %08x", pc_);
              SymbolMapItr itr = g_dbg->symbol_map_.find(default_addr);
              if (itr != g_dbg->symbol_map_.end()) {
                delete itr->second;
                g_dbg->symbol_map_.erase(itr);
              }
              Symbol* symbol = new Symbol(default_addr, tmp);
              g_dbg->symbol_map_.insert(SymbolMapValue(default_addr, symbol));
            }
            free(chkary);

            jump_addr_[0] = 0xfffffffe;  // 0xfffffffeはリターン

            is_switch = true;
          } while (0);

          if (is_switch == false) {
            // リターンでもswitchでもなければ単なるジャンプと見なす
            DBGOUT_ANL("%08xで用途不明のjrを発見\n", pc_);
            if (g_dbg->isInModuleCodeSection(gpr_[rs])) {
              jump_addr_[0] = gpr_[rs];
            } else {
              DBGOUT_ANL("%08xでコードセクション外のジャンプを検出しました！\n", pc_);
            }
          }
#undef _rs
#undef _rt
#undef _rd
#undef _spc
        }
        break;
      case 0x09:  // jalr
        break;
      case 0x0b:  // movn
        if (gpr_[rt] != 0) {
          gpr_[rd] = gpr_[rs];
        }
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x0c:  // movz
        if (gpr_[rt] == 0) {
          gpr_[rd] = gpr_[rs];
        }
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x20:  // add
      case 0x21:  // addu
      case 0x2d:  // daddu
        gpr_[rd] = gpr_[rs] + gpr_[rt];
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x22:  // sub
      case 0x23:  // subu
      case 0x2f:  // dsubu
        gpr_[rd] = gpr_[rs] - gpr_[rt];
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x24:  // and
        gpr_[rd] = gpr_[rs] & gpr_[rt];
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x25:  // or
        gpr_[rd] = gpr_[rs] | gpr_[rt];
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x26:  // xor
        gpr_[rd] = gpr_[rs] ^ gpr_[rt];
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x27:  // nor
        gpr_[rd] = ~(gpr_[rs] | gpr_[rt]);
        itr->second.SetEstimate(gpr_[rd]);
        break;
      case 0x2a:  // slt
        gpr_[rd] = (s32)gpr_[rs] < (s32)gpr_[rt];
        break;
      case 0x2b:  // sltu
        gpr_[rd] = gpr_[rs] < gpr_[rt];
        break;
      }
      break;
    case 0x01:  // regimm
      switch (opcode >> 16 & 0x1f) {
      case 0x00:  // bltz
      case 0x01:  // bgez
      case 0x02:  // bltzl
      case 0x03:  // bgezl
      case 0x11:  // bltzal
      case 0x12:  // bgezal
      case 0x13:  // bltzall
      case 0x14:  // bgezall
        u32 addr = pc_ + 4 + ((s16)imm) * 4;
        Branch(addr);
        itr->second.SetEstimate(addr);
      }
      break;
    case 0x02: {// j
      u32 addr = (pc_ + 4) & 0xf0000000 | (opcode & 0x03ffffff) << 2;
      itr->second.SetEstimate(addr);
      jump_addr_[0] = addr;
    } break;
    case 0x03: {//jal
      u32 addr = (pc_ + 4) & 0xf0000000 | (opcode & 0x03ffffff) << 2;
      itr->second.SetEstimate(addr);

      AnalyzeContext* new_ctx = new AnalyzeContext(*this);
      new_ctx->jump_addr_[0] = addr;
      new_ctx->Step();
      new_ctx->AnalyzeFragment(p_ana_inf_map, p_enable);
      memcpy(gpr_, new_ctx->gpr_, sizeof(u32) * 32);
      delete new_ctx;

      jump_addr_[1] = pc_ + 8;
    } break;
    case 0x04:  // beq
    case 0x05:  // bne
    case 0x06:  // blez
    case 0x07: {// bgtz
      u32 addr = pc_ + 4 + ((s16)imm) * 4;
      itr->second.SetEstimate(addr);
      if (inst1 == 0x04 && rt == rs) { // beqでrtとrsが同じ
        // 無条件ジャンプ
        jump_addr_[0] = addr;
        break;
      } else {
        // ジャンプする場合のコンテキストを積む
        Branch(addr);
      }
      // ジャンプしない場合
      if (inst1 == 0x05) {
        // 条件に合うようにコンテキストを修正(焼け石に水状態なんだが…)
        if (rt == 0) {
          gpr_[rs] = gpr_[rt];
        } else {
          gpr_[rt] = gpr_[rs];
        }
      }
    } break;
    case 0x08:  // addi
    case 0x09:  // addiu
    case 0x18:  // daddi
    case 0x19:  // daddiu
      gpr_[rt] = gpr_[rs] + (s16)imm;
      itr->second.SetEstimate(gpr_[rt]);
      UncertainBranch(gpr_[rt]);
      break;
    case 0x0a:  // slti
      gpr_[rd] = (s32)gpr_[rs] < (s16)imm;
      break;
    case 0x0b:  // sltiu
      gpr_[rd] = gpr_[rs] < (u16)imm;
      break;
    case 0x0c:  // andi      
      gpr_[rt] = gpr_[rs] & imm;
      itr->second.SetEstimate(gpr_[rt]);
      break;
    case 0x0d:  // ori      
      gpr_[rt] = gpr_[rs] | imm;
      itr->second.SetEstimate(gpr_[rt]);
      UncertainBranch(gpr_[rt]);
      break;
    case 0x0e:  // xori      
      gpr_[rt] = gpr_[rs] ^ imm;
      itr->second.SetEstimate(gpr_[rt]);
      break;
    case 0x0f:  // lui
      gpr_[rt] = imm << 16;
      itr->second.SetEstimate(gpr_[rt]);
      UncertainBranch(gpr_[rt]);
      break;
    case 0x10:  // cop0
      if ((opcode >> 21 & 0x1f) == 8) {
        // bc0
        switch (opcode >> 16 & 0x1f) {
        case 0x00:  // bc0f
        case 0x01:  // bc0t
        case 0x02:  // bc0fl
        case 0x03:  // bc0tl
          u32 addr = pc_ + 4 + ((s16)imm) * 4;
          Branch(addr);
          itr->second.SetEstimate(addr);
        }
      }
      break;
    case 0x11:  // cop1
      if ((opcode >> 21 & 0x1f) == 8) {
        // bc1
        switch (opcode >> 16 & 0x1f) {
        case 0x00:  // bc1f
        case 0x01:  // bc1t
        case 0x02:  // bc1fl
        case 0x03:  // bc1tl
          u32 addr = pc_ + 4 + ((s16)imm) * 4;
          Branch(addr);
          itr->second.SetEstimate(addr);
        }
      }
      break;
    case 0x14:  // beql
    case 0x15:  // bnel
    case 0x16:  // blezl
    case 0x17: {// bgtzl
      u32 addr = pc_ + 4 + ((s16)imm) * 4;
      itr->second.SetEstimate(addr);
      if (inst1 == 0x14 && rt == rs) { // beqlでrtとrsが同じ
        // 無条件ジャンプ
        jump_addr_[0] = addr;
        break;
      } else {
        // ジャンプする場合のコンテキストを積む
        Branch(addr);
      }
      // ジャンプしない場合
      jump_addr_[1] = pc_ + 8; // ディレイスロットをスキップ
      break; }
    case 0x1a: case 0x1b: case 0x1e: case 0x20:  // ldl ldr  lq  lb
    case 0x21: case 0x22: case 0x23: case 0x24:  // lh  lwl  lw  lbu
    case 0x25: case 0x26: case 0x27: case 0x31:  // lhu lwr  lwu lwc1
    case 0x37: case 0x1f: case 0x28: case 0x29:  // ld  sq   sb  sh
    case 0x2a: case 0x2b: case 0x2c: case 0x2d:  // swl sw   sdl sdr
    case 0x2e: case 0x39: case 0x3f: {      // swr swc1 sd
      u32  base = opcode >> 21 & 0x1f;
      u32  ofs = (u16)(opcode & 0xffff);
      u32 value = gpr_[base] + (s16)ofs;
      itr->second.SetEstimate(value);

      // loadの場合は解析用レジスタを更新する
      // l/r系はめんどくさいので必要を感じた時にやる
      // GPRの32bit以上も解析ではまず参照しないので無視
      u32 mem = VA2PA(value);
      if (mem) {
        switch (inst1) {
        case 0x20:  // lbu
          *((u8*)&gpr_[rt])  = *((u8*)mem);
          break;
        case 0x21:  // lhu
          *((u16*)&gpr_[rt]) = *((u16*)mem);
          break;
        case 0x23:  // lw
        case 0x25:  // lwu
        case 0x1e:  // lq
        case 0x37:  // ld
          gpr_[rt] = *((u32*)mem);
          break;
        }
      }
      break; }
    }
    Step();
  }
}

void AnalyzeContext::Step() {
  if (jump_addr_[1] == 0xfffffffe) {
    pc_ = 0xfffffffe;
  } else {
    if (jump_addr_[1] == 0xffffffff) {
      pc_ += 4;
    } else {
      pc_ = jump_addr_[1];
    }

    if (g_dbg->isInModuleCodeSection(pc_) == false) {
      DBGOUT_ANL("%08xで不正なアドレスへのジャンプが発生しました\n", pc_);
      pc_ = 0xfffffffe; // returnに置き換える
    }
  }
  jump_addr_[1] = jump_addr_[0];
  jump_addr_[0] = 0xffffffff;
}

void AnalyzeContext::Branch(u32 p_addr) {
  AnalyzeContext* new_ctx = new AnalyzeContext(*this);
  //new_ctx->ClearGPR();
  new_ctx->jump_addr_[0] = p_addr;
  new_ctx->Step();
  branch_stack_.push(new_ctx);
}

void AnalyzeContext::UncertainBranch(u32 p_addr) {
  // estimate値で示されるアドレスがコードっぽい場合、それも解析
  if (p_addr % 4 == 0 && g_dbg->isInModuleCodeSection(p_addr)) {
    DBGOUT_ANL("演算予測値%08xでコードを発見しました。解析します！\n", p_addr);
    Branch(p_addr);
  }
}
