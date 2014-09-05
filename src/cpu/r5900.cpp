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

  // �����l��ݒ�
  //m_cop0.random = 47;
  //m_cop0.wired = 0;
  //m_cop0.status = 0;
  //m_cop0.prid = 0x2e00;
  //m_cop0.config = 0x00000440; // DC=001,IC=010
}

void R5900::create_recompile_cache() {
  // ���ɂ���΍폜
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

  // ���ɃR�[�h������Ȃ�I��
  int begin_idx = VA2IDX(p_va);
  if (m_recompiled_code_table[begin_idx]) {
    DBGOUT_REC("����0x%08x�̃R�[�h�͐����ςȂ̂łȂɂ����܂���B\n", p_va);
    return m_recompiled_code_table[begin_idx];
  }

  // ���O�ɐ����ς݃R�[�h������΃u���b�N��V�K���������ɂ�����g������
  CodeBlock* codeblock = NULL;
  for (u32 i = 0; i < m_recompiled_code_ary.size(); i++) {
    if (begin_idx == m_recompiled_code_ary[i]->idx +
                     m_recompiled_code_ary[i]->ee_inst_count) {
      DBGOUT_REC("���O�̃A�h���X�ɐ����σR�[�h�𔭌��B"\
                 "�����̃o�b�t�@�ɒǋL���܂��B\n");
      codeblock = m_recompiled_code_ary[i];

      // �������݈ʒu��ݒ�i���O�̃R�[�h�̖����ɒu����Ă���ret���߂ׂ͒��j
      m_recompiling_code_ptr = codeblock->buf + codeblock->size - 1;
      break;
    }
  }

  if (codeblock == NULL) {
    DBGOUT_REC("���O�ɐ����σR�[�h������܂���B�o�b�t�@��V�K�쐬���܂��B\n");
    // �V�K�u���b�N
    codeblock = new CodeBlock;
    codeblock->idx = begin_idx;
    codeblock->buf = (u8*)malloc(FIRST_BUFSIZE);
    codeblock->capacity = FIRST_BUFSIZE;
    codeblock->size = 0;
    codeblock->ee_inst_count = 0;
    m_recompiled_code_ary.push_back(codeblock);

    // ���s������t�^
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

    // �e���߂̃��R���p�C���̃��x���ƏՓ˂��Ȃ��悤��100����Ƃ���
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
    // �������u���[�N�`�F�b�N
    mov32_reg_imm32(_eax, (u32)MemoryBreakPoint::checkBreak);
    call_reg(_eax);
    tst32_reg_reg(_eax, _eax);
    jz(membrk_miss);
    ret();
LABEL_DEF(membrk_miss);
#endif

    // PC���X�V����
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
    // �ŏ�ʃr�b�g�������Ă�����x86�̃A�h���X�Ƃ��ĉ��߂���
    // ��ɃR�[���o�b�N���[�`������߂�Ƃ��Ɏg�p����
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

    // �R�[�h����
    // beql���ߓ��͕��򂵂Ȃ��ꍇ�ɒx���X���b�g�̖��߂𖳌��ɂ���
    tst8_mem_imm8((u32)&m_null_current_inst, 0xff);
    mov8_mem_imm8((u32)&m_null_current_inst, 0);
    jnz32(end_of_inst_code);

    // �K�v���\�Ȃ�œK�����s��
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
      // �œK�������Ȃ畁�ʂɃR�[�h����
      TABLE[g_cpu->m_recompiling_opcode >> 26]();
#if defined _DEBUGGER
      // ���R���p�C���œK���Ώۂ�T��
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
    jz(dont_jmp_addr_x86); // ���̃W�����v����̂��H
    // } else {
    // m_x86_jump_addr = 0x00000000;
    xor32_reg_reg(_ecx, _ecx);
    mov32_rmem_reg(_edi, _ecx);
    // $eip = m_x86_jump_addr;
    jmp_reg(_eax);
    // }
LABEL_DEF(dont_jmp_addr_x86);

#if defined _DEBUGGER
    // �T�C�N�������J�E���g�A���Ԋu�Ń��C���X���b�h�ɖ߂�
    cmp32_mem_imm32((u32)&m_cycle_count, 640000);  // 64�����߂�1�x (�l�q��)
    jnz(skip_event_handle);
    mov32_reg_imm32(_eax, (u32)SwitchToFiber);
    push32_imm((u32)g_app.main_fiber());
    call_reg(_eax);
    mov32_mem_imm32((u32)&m_cycle_count, 0);
LABEL_DEF(skip_event_handle);
    inc32_mem((u32)&m_cycle_count);
#endif // defined _DEBUGGER

    // �W�����v���߂̃A�h���X�o�C���h����
    while (g_labelRefCount > 0) {
      g_labelRefCount--;
      LABELREF* p = &g_labelRef[g_labelRefCount];

      s32 reladdr = (s32)g_labelDef[p->idx] - (s32)p->ofs;
      if (p->ofs - p->addr == 1) {
        // 1�o�C�g�I�t�Z�b�g
        assert(reladdr < 128 && reladdr >= -128);
        *p->addr = (u8)(reladdr & 0xff);
      } else {
        // 4�o�C�g�I�t�Z�b�g
        *((u32*)p->addr) = reladdr;
      }
    }
    
    // �������ݍς݃T�C�Y���X�V
    codeblock->size = (u32)(m_recompiling_code_ptr - codeblock->buf);

    // �R���p�C���ʒu��i�߂�
    m_recompiling_ee_va += 4;

    // �R�[�h�e�[�u���C���f�b�N�X���X�V
    cur_idx = VA2IDX(m_recompiling_ee_va);

    // ���̃A�h���X�Ɋ����R�[�h������ΘA�����Ē��f
    if (cur_idx < (g_code_size >> 2) && m_recompiled_code_table[cur_idx]) {
      DBGOUT_REC("0x%08x�Ő����ς̃R�[�h�𔭌����܂����B\n",
                 m_recompiling_ee_va);
      bool join_succeed = false;

      //ds_util::DBGOUTA("�u���b�N��=%d\n", m_recompiled_code_ary.size());

      for (u32 j = 0; j < m_recompiled_code_ary.size(); j++) {
        if (m_recompiled_code_ary[j]->buf != m_recompiled_code_table[cur_idx]) continue;
        join_succeed = true;

        DBGOUT_REC("�o�b�t�@��A�����܂��B0x%08x~0x%08x + 0x%08x~0x%08x\n",
          g_code_addr + (codeblock->idx << 2),
          g_code_addr + ((codeblock->idx + codeblock->ee_inst_count) << 2),
          g_code_addr + (m_recompiled_code_ary[j]->idx << 2),
          g_code_addr + ((m_recompiled_code_ary[j]->idx + m_recompiled_code_ary[j]->ee_inst_count) << 2));
        u8* newbuf = codeblock->buf;
        
        // �g�����K�v�Ȃ�s��(���Ȃ��Ƃ��P���߂̐������\�ȗ]�T����������K�v������)
        u32 newsize = codeblock->size + m_recompiled_code_ary[j]->size + STEP_BUFSIZE;
        if (newsize > codeblock->capacity) {
          newbuf = (u8*)realloc(codeblock->buf, newsize);
          DBGOUT_REC("�o�b�t�@[0x%x]���s�����Ă����̂�"
                     "�Ċ��蓖�Ă��܂���(0x%x > 0x%x)\n",
                     codeblock->idx, codeblock->size, newsize);

          // �T�C�Y�g���ɂ�胁�������Ĕz�u����Ă�����e�[�u�����C��
          if (newbuf != codeblock->buf) {
            DBGOUT_REC("�o�b�t�@[0x%x]�̍Ċ��蓖�Ăɂ��A�h���X��"
                       "�ω������̂ŏC�����܂�(0x%x > 0x%x)\n",
                       codeblock->idx, codeblock->buf, newbuf);
            s32 offset = newbuf - codeblock->buf;
            for (u32 k = 0; k < codeblock->ee_inst_count; k++) {
              int idx = codeblock->idx + k;
              if (m_recompiled_code_table[idx] >= codeblock->buf &&
                  m_recompiled_code_table[idx] <  codeblock->buf + codeblock->size) {
                m_recompiled_code_table[idx] += offset;
              }
            }
            // �SEE�X���b�h�̖߂��A�h���X(x86��ee)������ΏC��
            thread_mgr_->updateX86Retaddrs(codeblock->buf, codeblock->size, newbuf);
          }
          // ���s������t�^
          DWORD oldProtect;
          VirtualProtect(newbuf, newsize, PAGE_EXECUTE_READWRITE, &oldProtect);

          codeblock->capacity = newsize;
        }

        // ��A���o�b�t�@�̃R�s�[
        u8* buf2_head_new = newbuf + codeblock->size;
        memcpy(buf2_head_new,
               m_recompiled_code_ary[j]->buf,
               m_recompiled_code_ary[j]->size);

        // �A�������o�b�t�@�̃}�b�s���O�e�[�u�������
        assert(m_recompiled_code_ary[j]->idx == cur_idx);
        DBGOUT_REC("�A�������o�b�t�@[0x%x]�̃A�h���X���C�����܂�(0x%x > 0x%x)\n",
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
        // �SEE�X���b�h�̖߂��A�h���X(x86��ee)������ΏC��
        thread_mgr_->updateX86Retaddrs(m_recompiled_code_ary[j]->buf,
                                       m_recompiled_code_ary[j]->size,
                                       buf2_head_new);

        codeblock->buf = newbuf;
        codeblock->size += m_recompiled_code_ary[j]->size;
        codeblock->ee_inst_count += m_recompiled_code_ary[j]->ee_inst_count;

        // �����ς݃R�[�h�u���b�N�͍폜
        delete m_recompiled_code_ary[j];
        m_recompiled_code_ary.erase(&m_recompiled_code_ary[j]);
        break;
      }
      // �A���ۂɊւ�炸���R���p�C���͂����܂�
      if (join_succeed == false) {
        // �A���ł��Ȃ������玟�̃R�[�h���}�b�s���O�e�[�u������
        // �擾����K�v������̂�ret���߂��d����
        ret();
      }
      break;
    }
    // �K�薽�ߐ��܂ŃR���p�C�������璆�f

    // �e�X�g�R�[�h�̏ꍇ�A�R�[�h�̈�O�֏o����I���Ƃ���(�A�����������s�͂��Ȃ�)
#if defined _UNIT_TEST
    if (IS_OUT_OF_TEST_ELF_CODE_SECTION(m_recompiling_ee_va)) {
      cur_idx = p_count + begin_idx;
    }
#endif // defined _UNIT_TEST

    if (cur_idx - begin_idx >= p_count) {
      DBGOUT_REC("�K�薽�ߐ��̃��R���p�C�����I���܂����B"
                 "��U���R���p�C�����[�v�𔲂��܂�\n");
      ret();
      codeblock->size++;
      break;
    }

    // �o�b�t�@�c�肪�K�萔��������g��
    if (codeblock->size + STEP_BUFSIZE > codeblock->capacity) {
      DBGOUT_REC("�o�b�t�@[0x%x]���s�����Ă��܂��B(0x%x / 0x%x)"
                 "�o�b�t�@���g�����܂��B(0x%x => 0x%x)\n",
                 codeblock->idx,
                 codeblock->size,
                 codeblock->capacity,
                 codeblock->capacity,
                 codeblock->capacity + STEP_BUFSIZE);
      // �T�C�Y�g�����čĊ��蓖��
      codeblock->capacity += STEP_BUFSIZE;
      u8* newbuf = (u8*)realloc(codeblock->buf, codeblock->capacity);
      
      // ���s������t�^
      DWORD oldProtect;
      VirtualProtect(newbuf, codeblock->capacity,
                     PAGE_EXECUTE_READWRITE, &oldProtect);

      // �������݃|�C���^�A�}�b�s���O�e�[�u�������
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
    // �R�[�h�̈�O�֏o����e�X�g�R�[�h�̏I���Ƃ���(�A�����������s�͂��Ȃ�)
#if defined _UNIT_TEST
    if (IS_OUT_OF_TEST_ELF_CODE_SECTION(m_pc)) break;
#endif // defined _UNIT_TEST
    int idx  = VA2IDX(m_pc);
    addr = m_recompiled_code_table[idx];

    // PC�̖��߂�NULL�Ȃ�R�[�h�𐶐����邽�߂Ƀ��[�v���p������
    // PC�̖��ߎ��̂�NULL�łȂ���΃W�����v�悪NULL�A
    // ���̓u���[�N�|�C���g�Œ�~���Ă���\��������B
    // �u���[�N�|�C���g�Œ�~���Ă���ꍇ�Ɍ���Aexecute���\�b�h���I������
    // ���Ȃ݂Ƀu���[�N�|�C���g��PC��̐擪��
    // ret����(0xc3)���u����Ă��邩�ǂ����Ŕ��ʉ\
#if defined _DEBUGGER
    if (g_dbg->hit_mem_break_) break;
#endif // defined _DEBUGGER
    if (addr && *addr == 0xc3) break;

    if (m_ee_jump_addr < 0x80000000) {
      DBGOUT_REC("0x%08x�ŃW�����v���߂����s����܂����B"
                 "�W�����v��(0x%08x)�����O�Ƀ��R���p�C�����܂�\n",
                 m_pc, m_ee_jump_addr);
      
      // �W�����v�悪���肵�Ă���(=PC���f�B���C�X���b�g�ł���)�ꍇ
      // �W�����v��̃R�[�h�����R���p�C�����Ă���
      recompile(m_ee_jump_addr, RECOMPILE_AMOUNT, p_enable_macro_recompile);

      // PC�̖��߂�������΁A�f�B���C�X���b�g���i1���߁j��������
      // ���recompile�ɂ���ăR�[�h�u���b�N�̍Ĕz�u���s���Ă��邩��
      // �m��Ȃ��̂�m_recompiled_code_table���Q�Ƃ��Ď�蒼��
      addr = m_recompiled_code_table[idx];
      if (addr == NULL) {
        DBGOUT_REC("�f�B���C�X���b�g0x%08x�̓��R���p�C������Ă��܂���B"
                   "�W�����v���邩������Ȃ��̂łP�s�̂݃��R���p�C�����܂�\n",
                   m_pc);
        addr = recompile(m_pc, 1, p_enable_macro_recompile);
      }
    } else if (m_ee_jump_addr == 0xffffffff) {
      if (addr == NULL) {
        DBGOUT_REC("0x%08x�̓��R���p�C������Ă��܂���B"
                   "���s�̃��R���p�C�����s���܂�\n", m_pc, m_ee_jump_addr);
        // ���򖽗߂������ς݃R�[�h��������܂ŃR�[�h����
        addr = recompile(m_pc, RECOMPILE_AMOUNT, p_enable_macro_recompile);
      }
    }

    thread_mgr_->cur_thread()->set_recompiled_code(addr);
    SwitchToFiber(thread_mgr_->cur_thread()->fiber());

    if (p_continue == false) break;
  }

#if defined _DEBUGGER
  if (addr == NULL) {
    DBGOUT_REC("0x%08x�ŏ����𒆒f���܂��B"
               "���̎��s�悪���R���p�C������Ă��܂���B\n", m_pc);
  } else {
    DBGOUT_REC("0x%08x�ŏ����𒆒f���܂��B"
               "�u���[�N�|�C���g���ݒ肳��Ă��܂��B\n", m_pc);
  }
#endif // defined _DEBUGGER
}

#if defined _DEBUGGER

void R5900::enable_break(u32 p_va, u8* p_savebyte) {
  // ���R���p�C������Ă��Ȃ��ʒu�Ƀu���[�N����ꂽ�炻�̖��߂������R���p�C��
  u8* ptr = recompile(p_va, 1);

  // ���̃R�[�h��ޔ����Aret���߂ɒu������
  *p_savebyte = *ptr;
  *ptr = 0xc3; // ret
}

void R5900::disable_break(u32 p_va, u8 p_savebyte) {
  // ���̃R�[�h�𕜌�
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
        // �V�K�ǉ�
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
    // �t�@�C�����Ȃ��ꍇ
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
