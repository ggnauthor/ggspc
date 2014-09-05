//-----------------------------------------------------------------------------
// include
#include "main.h"
#include "cpu/r5900.h"
#include "cpu/x86macro.h"
#include "memory/memory.h"
#include "thread/thread.h"
#include "device/device.h"

#if defined _DEBUGGER
#include "debugger/debugger.h"
#include "debugger/disasm.h"
#include "debugger/gui/window/reference_window.h"
#endif

#if defined _UNIT_TEST
#include "test/test_r5900.h"
#endif

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

//-----------------------------------------------------------------------------
// global
R5900* g_cpu = NULL;

extern PS2ThreadMgr*    thread_mgr_;

//-----------------------------------------------------------------------------
// function
void R5900REGS::dump() {
  #define DUMP_GPR(_reg, _str) \
    DBGOUT_CPU(#_reg":%08x %08x %08x %08x"##_str, \
    m_##_reg.d[3], m_##_reg.d[2], m_##_reg.d[1], m_##_reg.d[0])

  DUMP_GPR(ze, "  "); DUMP_GPR(at, "\n");
  DUMP_GPR(v0, "  "); DUMP_GPR(v1, "\n");
  DUMP_GPR(a0, "  "); DUMP_GPR(a1, "\n");
  DUMP_GPR(a2, "  "); DUMP_GPR(a3, "\n");
  DUMP_GPR(t0, "  "); DUMP_GPR(t1, "\n");
  DUMP_GPR(t2, "  "); DUMP_GPR(t3, "\n");
  DUMP_GPR(t4, "  "); DUMP_GPR(t5, "\n");
  DUMP_GPR(t6, "  "); DUMP_GPR(t7, "\n");
  DUMP_GPR(s0, "  "); DUMP_GPR(s1, "\n");
  DUMP_GPR(s2, "  "); DUMP_GPR(s3, "\n");
  DUMP_GPR(s4, "  "); DUMP_GPR(s5, "\n");
  DUMP_GPR(s6, "  "); DUMP_GPR(s7, "\n");
  DUMP_GPR(t8, "  "); DUMP_GPR(t9, "\n");
  
  DUMP_GPR(k0, "  "); DUMP_GPR(k1, "\n");
  DUMP_GPR(gp, "  "); DUMP_GPR(sp, "\n");
  DUMP_GPR(fp, "  "); DUMP_GPR(ra, "\n");

  DBGOUT_CPU("pc:%08x sa:%08x\n", m_pc, m_sa);
  
  DUMP_GPR(hi, "  "); DUMP_GPR(lo, "\n");

  #undef DUMP_GPR

  for (int i = 0; i < 31; i++) {
    DBGOUT_CPU("f%02d:%08x %f\n", i, m_fpu.fpr[i], *((u32*)&m_fpu.fpr[i]));
  }
}

R5900::R5900() :
    m_recompiling_opcode(0x00000000),
    m_recompiled_code_table(NULL) {
  
}

R5900::~R5900() {
  clearRecompiledCode();
  if (m_recompiled_code_table) {
    delete[] m_recompiled_code_table;
    m_recompiled_code_table = NULL;
  }
}

void R5900::init() {
  R5900REGS::init();

  // 初期値を設定
  //m_cop0.random = 47;
  //m_cop0.wired = 0;
  //m_cop0.status = 0;
  //m_cop0.prid = 0x2e00;
  //m_cop0.config = 0x00000440; // DC=001,IC=010
}

void R5900::create_recompile_cache() {
  // 既にあれば削除
  if (m_recompiled_code_table) delete[] m_recompiled_code_table;

  m_recompiled_code_table = new u8*[(g_code_size >> 2) + 1];
  clearRecompiledCode();

#if defined _DEBUGGER
  init_run_count();
  load_recompile_hint();
#endif // defined _DEBUGGER
}

void R5900::clear_recompile_cache() {
  clearRecompiledCode();
}

void R5900::clearRecompiledCode() {
  memset(m_recompiled_code_table, 0, sizeof(u8*) * (g_code_size >> 2));

  for (vector<CodeBlock*>::iterator itr = m_recompiled_code_ary.begin();
      itr != m_recompiled_code_ary.end();
      ++itr) {
    delete *itr;
  }
  m_recompiled_code_ary.clear();
}

u8* R5900::recompile(u32 p_va, u32 p_count, bool p_enable_macro_recompile) {
  const u32 FIRST_BUFSIZE = 0x4000; // 16k
  const u32 STEP_BUFSIZE  = 0x1000; // 4k

  // 既にコードがあるなら終了
  int begin_idx = VA2IDX(p_va);
  if (m_recompiled_code_table[begin_idx]) {
    DBGOUT_REC("既に0x%08xのコードは生成済なのでなにもしません。\n", p_va);
    return m_recompiled_code_table[begin_idx];
  }

  // 直前に生成済みコードがあればブロックを新規生成せずにそれを拡張する
  CodeBlock* codeblock = NULL;
  for (u32 i = 0; i < m_recompiled_code_ary.size(); i++) {
    if (begin_idx == m_recompiled_code_ary[i]->idx +
                     m_recompiled_code_ary[i]->ee_inst_count) {
      DBGOUT_REC("直前のアドレスに生成済コードを発見。"\
                 "既存のバッファに追記します。\n");
      codeblock = m_recompiled_code_ary[i];

      // 書き込み位置を設定（直前のコードの末尾に置かれているret命令は潰す）
      m_recompiling_code_ptr = codeblock->buf + codeblock->size - 1;
      break;
    }
  }

  if (codeblock == NULL) {
    DBGOUT_REC("直前に生成済コードがありません。バッファを新規作成します。\n");
    // 新規ブロック
    codeblock = new CodeBlock;
    codeblock->idx = begin_idx;
    codeblock->buf = (u8*)malloc(FIRST_BUFSIZE);
    codeblock->capacity = FIRST_BUFSIZE;
    codeblock->size = 0;
    codeblock->ee_inst_count = 0;
    m_recompiled_code_ary.push_back(codeblock);

    // 実行権限を付与
    DWORD oldProtect;
    VirtualProtect(codeblock->buf, codeblock->capacity,
                   PAGE_EXECUTE_READWRITE, &oldProtect);

    m_recompiling_code_ptr = codeblock->buf;
  }

  m_recompiling_ee_va = p_va;
  u32 cur_idx = begin_idx;
  while (1) {
    assert(m_recompiled_code_table[cur_idx] == 0x00000000);
    m_recompiled_code_table[cur_idx] = m_recompiling_code_ptr;
    m_recompiling_opcode = *((u32*)VA2PA(m_recompiling_ee_va));

    // 各命令のリコンパイラのラベルと衝突しないように100からとする
    enum {
      jmp_addr = 100,
      jmp_addr_ee,
      addr_ctrl_end,
      x86_code_exist,
      end_of_inst_code,
      dont_jmp_addr_x86,
      skip_event_handle,
      membrk_miss
    };
#if defined _DEBUGGER
    // メモリブレークチェック
    mov32_reg_imm32(_eax, (u32)MemoryBreakPoint::checkBreak);
    call_reg(_eax);
    tst32_reg_reg(_eax, _eax);
    jz(membrk_miss);
    ret();
LABEL_DEF(membrk_miss);
#endif

    // PCを更新する
    // $esi = 0xffffffff
    xor32_reg_reg(_esi, _esi);
    dec32(_esi);
    mov32_reg_imm32(_edx, (u32)&m_pc);
    mov32_reg_imm32(_edi, (u32)&m_x86_jump_addr);
    mov32_reg_imm32(_ebx, (u32)&m_ee_jump_addr);
    // $eax = m_ee_jump_addr;
    mov32_reg_rmem(_eax, _ebx);
    
    // if (m_ee_jump_addr == 0xffffffff) {
    cmp32_reg_reg(_eax, _esi);
    jnz(jmp_addr);
    // m_pc += 4;
    add32_rmem_imm8(_edx, 4);
    // m_x86_jump_addr = 0x00000000;
    mov32_rmem_imm32(_edi, 0x00000000);
    jmp(addr_ctrl_end);
    // } else if (m_ee_jump_addr & 0x80000000) {
    // 最上位ビットが立っていたらx86のアドレスとして解釈する
    // 主にコールバックルーチンから戻るときに使用する
LABEL_DEF(jmp_addr);
    tst32_reg_reg(_eax, _eax);
    jns(jmp_addr_ee);
    // m_x86_jump_addr = m_ee_jump_addr & 0x7fffffff
    and32_eax_imm32(0x7fffffff);
    mov32_rmem_reg(_edi, _eax);
    jmp(x86_code_exist);
    // } else {
LABEL_DEF(jmp_addr_ee);
    // m_x86_jump_addr = m_recompiled_code_table[VA2IDX(m_pc)]
    sub32_reg_imm32(_eax, g_code_addr);
    mov32_reg_rmem_ofs32(_eax, _eax, (u32)m_recompiled_code_table);
    mov32_rmem_reg(_edi, _eax);
    // if (m_x86_jump_addr == 0) {
    tst32_reg_reg(_eax, _eax);
    jnz(x86_code_exist);
    // return;
    ret();
    // } else {
LABEL_DEF(x86_code_exist);
    // m_pc = m_ee_jump_addr;
    mov32_reg_rmem(_eax, _ebx);
    mov32_rmem_reg(_edx, _eax);
    // m_ee_jump_addr = 0xffffffff;
    mov32_rmem_reg(_ebx, _esi);
    // }
LABEL_DEF(addr_ctrl_end);

    // コード生成
    // beql命令等は分岐しない場合に遅延スロットの命令を無効にする
    tst8_mem_imm8((u32)&m_null_current_inst, 0xff);
    mov8_mem_imm8((u32)&m_null_current_inst, 0);
    jnz32(end_of_inst_code);

    // 必要かつ可能なら最適化を行う
    extern u32 (*g_r5900_macro_ary[])(u32 p_va);
    bool macro_recompile = false;
    if (p_enable_macro_recompile) {
      for (int i = 0; g_r5900_macro_ary[i]; i++) {
        u32  save_va = m_recompiling_ee_va;
        u32 ee_inst_count = g_r5900_macro_ary[i](m_recompiling_ee_va);
        if (ee_inst_count > 0) {
          codeblock->ee_inst_count += ee_inst_count;
          macro_recompile = true;
#if defined _DEBUGGER
          g_dbg->macro_rec_ary_.push_back(new MacroRecInfo(save_va, i));
#endif // defined _DEBUGGER
          break;
        }
      }
    }

    if (macro_recompile == false) {
      void invalid();
      void special(); void regimm(); void j();     void jal();
      void beq();     void bne();    void blez();  void bgtz();
      void addi();    void addiu();  void slti();  void sltiu();
      void andi();    void ori();    void xori();  void lui();
      void cop0();    void cop1();
      void beql();    void bnel();   void blezl(); void bgtzl();
      void daddi();   void daddiu(); void ldl();   void ldr();
      void mmi();                    void lq();    void sq();
      void lb();      void lh();     void lwl();   void lw();
      void lbu();     void lhu();    void lwr();   void lwu();
      void sb();      void sh();     void swl();   void sw();
      void sdl();     void sdr();    void swr();   void cache();
                      void lwc1();                 void pref();
                                     void lqc2();  void ld();
                      void swc1();
                                     void sqc2();  void sd();
      static void (*TABLE[0x40])() = {
        special, regimm, j,       jal,     beq,     bne,     blez,  bgtz,
        addi,    addiu,  slti,    sltiu,   andi,    ori,     xori,  lui,
        cop0,    cop1,   invalid, invalid, beql,    bnel,    blezl, bgtzl,
        daddi,   daddiu, ldl,     ldr,     mmi,     invalid, lq,    sq,
        lb,      lh,     lwl,     lw,      lbu,     lhu,     lwr,   lwu,
        sb,      sh,     swl,     sw,      sdl,     sdr,     swr,   cache,
        invalid, lwc1,   invalid, pref,    invalid, invalid, lqc2,  ld,
        invalid, swc1,   invalid, invalid, invalid, invalid, sqc2,  sd,
      };
      // 最適化無しなら普通にコード生成
      TABLE[g_cpu->m_recompiling_opcode >> 26]();
#if defined _DEBUGGER
      // リコンパイル最適化対象を探す
      mov32_reg_imm32(_edx, (u32)&m_run_count_ary[0] +
                            m_recompiling_ee_va - g_code_addr);
      inc32_edxmem();
#endif // defined _DEBUGGER
      codeblock->ee_inst_count++;
    }

LABEL_DEF(end_of_inst_code);
    // if (m_x86_jump_addr != 0x00000000) {
    mov32_reg_imm32(_edi, (u32)&m_x86_jump_addr);
    mov32_reg_rmem(_eax, _edi);
    tst32_reg_reg(_eax, _eax);
    jz(dont_jmp_addr_x86); // このジャンプいるのか？
    // } else {
    // m_x86_jump_addr = 0x00000000;
    xor32_reg_reg(_ecx, _ecx);
    mov32_rmem_reg(_edi, _ecx);
    // $eip = m_x86_jump_addr;
    jmp_reg(_eax);
    // }
LABEL_DEF(dont_jmp_addr_x86);

#if defined _DEBUGGER
    // サイクル数をカウント、一定間隔でメインスレッドに戻る
    cmp32_mem_imm32((u32)&m_cycle_count, 640000);  // 64万命令に1度 (様子見)
    jnz(skip_event_handle);
    mov32_reg_imm32(_eax, (u32)SwitchToFiber);
    push32_imm((u32)g_app.main_fiber());
    call_reg(_eax);
    mov32_mem_imm32((u32)&m_cycle_count, 0);
LABEL_DEF(skip_event_handle);
    inc32_mem((u32)&m_cycle_count);
#endif // defined _DEBUGGER

    // ジャンプ命令のアドレスバインド処理
    while (g_labelRefCount > 0) {
      g_labelRefCount--;
      LABELREF* p = &g_labelRef[g_labelRefCount];

      s32 reladdr = (s32)g_labelDef[p->idx] - (s32)p->ofs;
      if (p->ofs - p->addr == 1) {
        // 1バイトオフセット
        assert(reladdr < 128 && reladdr >= -128);
        *p->addr = (u8)(reladdr & 0xff);
      } else {
        // 4バイトオフセット
        *((u32*)p->addr) = reladdr;
      }
    }
    
    // 書き込み済みサイズを更新
    codeblock->size = (u32)(m_recompiling_code_ptr - codeblock->buf);

    // コンパイル位置を進める
    m_recompiling_ee_va += 4;

    // コードテーブルインデックスを更新
    cur_idx = VA2IDX(m_recompiling_ee_va);

    // 次のアドレスに既存コードがあれば連結して中断
    if (cur_idx < (g_code_size >> 2) && m_recompiled_code_table[cur_idx]) {
      DBGOUT_REC("0x%08xで生成済のコードを発見しました。\n",
                 m_recompiling_ee_va);
      bool join_succeed = false;

      //ds_util::DBGOUTA("ブロック数=%d\n", m_recompiled_code_ary.size());

      for (u32 j = 0; j < m_recompiled_code_ary.size(); j++) {
        if (m_recompiled_code_ary[j]->buf != m_recompiled_code_table[cur_idx]) continue;
        join_succeed = true;

        DBGOUT_REC("バッファを連結します。0x%08x~0x%08x + 0x%08x~0x%08x\n",
          g_code_addr + (codeblock->idx << 2),
          g_code_addr + ((codeblock->idx + codeblock->ee_inst_count) << 2),
          g_code_addr + (m_recompiled_code_ary[j]->idx << 2),
          g_code_addr + ((m_recompiled_code_ary[j]->idx + m_recompiled_code_ary[j]->ee_inst_count) << 2));
        u8* newbuf = codeblock->buf;
        
        // 拡張が必要なら行う(少なくとも１命令の生成が可能な余裕を持たせる必要がある)
        u32 newsize = codeblock->size + m_recompiled_code_ary[j]->size + STEP_BUFSIZE;
        if (newsize > codeblock->capacity) {
          newbuf = (u8*)realloc(codeblock->buf, newsize);
          DBGOUT_REC("バッファ[0x%x]が不足していたので"
                     "再割り当てしました(0x%x > 0x%x)\n",
                     codeblock->idx, codeblock->size, newsize);

          // サイズ拡張によりメモリが再配置されていたらテーブルを修正
          if (newbuf != codeblock->buf) {
            DBGOUT_REC("バッファ[0x%x]の再割り当てによりアドレスが"
                       "変化したので修正します(0x%x > 0x%x)\n",
                       codeblock->idx, codeblock->buf, newbuf);
            s32 offset = newbuf - codeblock->buf;
            for (u32 k = 0; k < codeblock->ee_inst_count; k++) {
              int idx = codeblock->idx + k;
              if (m_recompiled_code_table[idx] >= codeblock->buf &&
                  m_recompiled_code_table[idx] <  codeblock->buf + codeblock->size) {
                m_recompiled_code_table[idx] += offset;
              }
            }
            // 全EEスレッドの戻り先アドレス(x86→ee)があれば修正
            thread_mgr_->updateX86Retaddrs(codeblock->buf, codeblock->size, newbuf);
          }
          // 実行権限を付与
          DWORD oldProtect;
          VirtualProtect(newbuf, newsize, PAGE_EXECUTE_READWRITE, &oldProtect);

          codeblock->capacity = newsize;
        }

        // 被連結バッファのコピー
        u8* buf2_head_new = newbuf + codeblock->size;
        memcpy(buf2_head_new,
               m_recompiled_code_ary[j]->buf,
               m_recompiled_code_ary[j]->size);

        // 連結したバッファのマッピングテーブルを訂正
        assert(m_recompiled_code_ary[j]->idx == cur_idx);
        DBGOUT_REC("連結したバッファ[0x%x]のアドレスを修正します(0x%x > 0x%x)\n",
                   m_recompiled_code_ary[j]->idx,
                   m_recompiled_code_ary[j]->buf,
                   buf2_head_new);
        s32 offset = buf2_head_new - m_recompiled_code_ary[j]->buf;
        for (u32 k = 0; k < m_recompiled_code_ary[j]->ee_inst_count; k++) {
          int idx = cur_idx + k;
          if (m_recompiled_code_table[idx] >= m_recompiled_code_ary[j]->buf &&
              m_recompiled_code_table[idx] <  m_recompiled_code_ary[j]->buf +
                                              m_recompiled_code_ary[j]->size) {
            m_recompiled_code_table[idx] += offset;
          }
        }
        // 全EEスレッドの戻り先アドレス(x86→ee)があれば修正
        thread_mgr_->updateX86Retaddrs(m_recompiled_code_ary[j]->buf,
                                       m_recompiled_code_ary[j]->size,
                                       buf2_head_new);

        codeblock->buf = newbuf;
        codeblock->size += m_recompiled_code_ary[j]->size;
        codeblock->ee_inst_count += m_recompiled_code_ary[j]->ee_inst_count;

        // 結合済みコードブロックは削除
        delete m_recompiled_code_ary[j];
        m_recompiled_code_ary.erase(&m_recompiled_code_ary[j]);
        break;
      }
      // 連結可否に関わらずリコンパイルはここまで
      if (join_succeed == false) {
        // 連結できなかったら次のコードをマッピングテーブルから
        // 取得する必要があるのでret命令を仕込む
        ret();
      }
      break;
    }
    // 規定命令数までコンパイルしたら中断

    // テストコードの場合、コード領域外へ出たら終了とする(連続生成＆実行はしない)
#if defined _UNIT_TEST
    if (IS_OUT_OF_TEST_ELF_CODE_SECTION(m_recompiling_ee_va)) {
      cur_idx = p_count + begin_idx;
    }
#endif // defined _UNIT_TEST

    if (cur_idx - begin_idx >= p_count) {
      DBGOUT_REC("規定命令数のリコンパイルを終えました。"
                 "一旦リコンパイルループを抜けます\n");
      ret();
      codeblock->size++;
      break;
    }

    // バッファ残りが規定数超えたら拡張
    if (codeblock->size + STEP_BUFSIZE > codeblock->capacity) {
      DBGOUT_REC("バッファ[0x%x]が不足しています。(0x%x / 0x%x)"
                 "バッファを拡張します。(0x%x => 0x%x)\n",
                 codeblock->idx,
                 codeblock->size,
                 codeblock->capacity,
                 codeblock->capacity,
                 codeblock->capacity + STEP_BUFSIZE);
      // サイズ拡張して再割り当て
      codeblock->capacity += STEP_BUFSIZE;
      u8* newbuf = (u8*)realloc(codeblock->buf, codeblock->capacity);
      
      // 実行権限を付与
      DWORD oldProtect;
      VirtualProtect(newbuf, codeblock->capacity,
                     PAGE_EXECUTE_READWRITE, &oldProtect);

      // 書き込みポインタ、マッピングテーブルを訂正
      m_recompiling_code_ptr += newbuf - codeblock->buf;
      for (u32 j = 0; j < codeblock->ee_inst_count; j++) {
        int idx = codeblock->idx + j;
        if (m_recompiled_code_table[idx] >= codeblock->buf
        &&  m_recompiled_code_table[idx] <  codeblock->buf + codeblock->size) {
          m_recompiled_code_table[idx] += (u32)(newbuf - codeblock->buf);
        }
      }
      codeblock->buf = newbuf;
    }
  }
  return m_recompiled_code_table[begin_idx];
}

void R5900::execute(bool p_enable_macro_recompile, bool p_continue) {

#if defined _DEBUGGER
  g_dbg->hit_mem_break_ = false;
#endif // defined _DEBUGGER

#if defined _UNIT_TEST
  const int RECOMPILE_AMOUNT = 999;
#else // defined _UNIT_TEST
  const int RECOMPILE_AMOUNT = 32;
#endif // defined _UNIT_TEST

  u8* addr = NULL;
  while (true) {
    // コード領域外へ出たらテストコードの終了とする(連続生成＆実行はしない)
#if defined _UNIT_TEST
    if (IS_OUT_OF_TEST_ELF_CODE_SECTION(m_pc)) break;
#endif // defined _UNIT_TEST
    int idx  = VA2IDX(m_pc);
    addr = m_recompiled_code_table[idx];

    // PCの命令がNULLならコードを生成するためにループを継続する
    // PCの命令自体がNULLでなければジャンプ先がNULL、
    // 又はブレークポイントで停止している可能性がある。
    // ブレークポイントで停止している場合に限り、executeメソッドを終了する
    // ちなみにブレークポイントはPC上の先頭に
    // ret命令(0xc3)が置かれているかどうかで判別可能
#if defined _DEBUGGER
    if (g_dbg->hit_mem_break_) break;
#endif // defined _DEBUGGER
    if (addr && *addr == 0xc3) break;

    if (m_ee_jump_addr < 0x80000000) {
      DBGOUT_REC("0x%08xでジャンプ命令が実行されました。"
                 "ジャンプ先(0x%08x)を事前にリコンパイルします\n",
                 m_pc, m_ee_jump_addr);
      
      // ジャンプ先が決定している(=PCがディレイスロットである)場合
      // ジャンプ先のコードをリコンパイルしておく
      recompile(m_ee_jump_addr, RECOMPILE_AMOUNT, p_enable_macro_recompile);

      // PCの命令が無ければ、ディレイスロット分（1命令）だけ生成
      // 上のrecompileによってコードブロックの再配置が行われているかも
      // 知れないのでm_recompiled_code_tableを参照して取り直す
      addr = m_recompiled_code_table[idx];
      if (addr == NULL) {
        DBGOUT_REC("ディレイスロット0x%08xはリコンパイルされていません。"
                   "ジャンプするかもしれないので１行のみリコンパイルします\n",
                   m_pc);
        addr = recompile(m_pc, 1, p_enable_macro_recompile);
      }
    } else if (m_ee_jump_addr == 0xffffffff) {
      if (addr == NULL) {
        DBGOUT_REC("0x%08xはリコンパイルされていません。"
                   "数行のリコンパイルを行います\n", m_pc, m_ee_jump_addr);
        // 分岐命令か生成済みコードが見つかるまでコード生成
        addr = recompile(m_pc, RECOMPILE_AMOUNT, p_enable_macro_recompile);
      }
    }

    thread_mgr_->cur_thread()->set_recompiled_code(addr);
    SwitchToFiber(thread_mgr_->cur_thread()->fiber());

    if (p_continue == false) break;
  }

#if defined _DEBUGGER
  if (addr == NULL) {
    DBGOUT_REC("0x%08xで処理を中断します。"
               "次の実行先がリコンパイルされていません。\n", m_pc);
  } else {
    DBGOUT_REC("0x%08xで処理を中断します。"
               "ブレークポイントが設定されています。\n", m_pc);
  }
#endif // defined _DEBUGGER
}

#if defined _DEBUGGER

void R5900::enable_break(u32 p_va, u8* p_savebyte) {
  // リコンパイルされていない位置にブレーク張られたらその命令だけリコンパイル
  u8* ptr = recompile(p_va, 1);

  // 元のコードを退避し、ret命令に置き換え
  *p_savebyte = *ptr;
  *ptr = 0xc3; // ret
}

void R5900::disable_break(u32 p_va, u8 p_savebyte) {
  // 元のコードを復元
  *m_recompiled_code_table[VA2IDX(p_va)] = p_savebyte;
}

void R5900::init_run_count() {
  m_run_count_ary.resize(g_code_size);
  for (u32 i = 0; i < g_code_size; i++) {
    m_run_count_ary[i] = 0;
  }
}

void R5900::create_recompiler_hint() {
  for (u32 i = 0; i < g_code_size; i++) {
    u32 value = m_run_count_ary[i] >> 4;
    if (value > 255) value = 255;
    if (value > m_recompile_hint_ary[i]) {
      if (m_recompile_hint_ary[i] == 0) {
        // 新規追加
        u32 va = g_code_addr + (i << 2);
        BottleNeck* bottle_neck = new BottleNeck(va, &m_recompile_hint_ary[i]);

        s8 str[1024];
        disasm(va, *((u32*)VA2PA(va)), str);
        bottle_neck->set_text(str);

        g_dbg->bottle_neck_ary_.push_back(bottle_neck);
        g_dbg->reference_window()->add_entry(bottle_neck);
      }
      m_recompile_hint_ary[i] = (u8)value;
    }
    m_run_count_ary[i] = 0;
  }
}

void R5900::load_recompile_hint() {

  m_recompile_hint_ary.resize(g_code_size);

  s8 rhi_path[1024];
  sprintf(rhi_path, "%s/data/%s/%s.rhi", g_base_dir, g_elf_name, g_elf_name);

  FILE* fp = fopen(rhi_path, "rb");
  if (fp) {
    char* fbuf = NULL;
    u32 fsize = ds_util::fsize32(fp);
    fbuf = new char[fsize];
    fread(fbuf, 1, fsize, fp);
    fclose(fp);

    for (u32 i = 0; i < g_code_size; i++) {
      m_recompile_hint_ary[i] = i < fsize ? fbuf[i] : 0;
    }
    delete[] fbuf;
  } else {
    // ファイルがない場合
    for (u32 i = 0; i < g_code_size; i++) {
      m_recompile_hint_ary[i] = 0;
    }
  }
}

void R5900::save_recompile_hint() {
  s8 rhi_path[1024];
  sprintf(rhi_path, "%s/data/%s/%s.rhi", g_base_dir, g_elf_name, g_elf_name);

  FILE* fp = fopen(rhi_path, "wb");
  if (fp) {
    for (u32 i = 0; i < g_code_size; i++) {
      fwrite(&m_recompile_hint_ary[i], 1, 1, fp);
    }
    fclose(fp);
  }
}

#endif // defined _DEBUGGER
