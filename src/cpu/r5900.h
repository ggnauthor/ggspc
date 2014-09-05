#ifndef _cpu_r5900
#define _cpu_r5900

#include "main.h"
#include "stlp_wrapper.h"

namespace COP0 {

enum STATUS {
  STATUS_IE  = 0x00000001,
  STATUS_EXL = 0x00000002,
  STATUS_ERL = 0x00000004,
  STATUS_KSU = 0x00000018,
  STATUS_IM  = 0x00000c00,
  STATUS_BEM = 0x00001000,
  STATUS_IM7 = 0x00008000,
  STATUS_EIE = 0x00010000,
  STATUS_EDI = 0x00020000,
  STATUS_CH  = 0x00040000,
  STATUS_BEV = 0x00400000,
  STATUS_DEV = 0x00800000,
  STATUS_CU  = 0xf0000000,
};

enum CONFIG {
  CONFIG_K0  = 0x00000007,
  CONFIG_DC  = 0x000001c0,
  CONFIG_IC  = 0x00000e00,
  CONFIG_BPE = 0x00001000,
  CONFIG_NBE = 0x00002000,
  CONFIG_DCE = 0x00010000,
  CONFIG_ICE = 0x00020000,
  CONFIG_DIE = 0x00040000,
  CONFIG_EC  = 0x70000000,
};

enum CAUSE {
  CAUSE_EXC  = 0x0000007c,
  CAUSE_IP2  = 0x00000400,
  CAUSE_IP3  = 0x00000800,
  CAUSE_IP7  = 0x00008000,
  CAUSE_EXC2 = 0x00070000,
  CAUSE_CE   = 0x30000000,
  CAUSE_BD2  = 0x40020000,
  CAUSE_BD   = 0x80000000,
};

}; // namespace COP0

namespace COP1 {

enum FCR31 {
  FCR31_C  = 0x00800000,
  FCR31_I  = 0x00020000,
  FCR31_D  = 0x00010000,
  FCR31_O  = 0x00008000,
  FCR31_U  = 0x00004000,
  FCR31_SI = 0x00000040,
  FCR31_SD = 0x00000020,
  FCR31_SO = 0x00000010,
  FCR31_SU = 0x00000008,
};

const unsigned int FLT_SIGN_MASK = 0x80000000;
const unsigned int FLT_EXP_MASK  = 0x7f800000;
const unsigned int FLT_INF_P     = 0x7f800000; // 仮数部が非0の時NaNとなる
                                               // (cop1では未サポート)
const unsigned int FLT_INF_N     = 0xff800000; // 仮数部が非0の時NaNとなる
                                               // (cop1では未サポート)
const unsigned int FLT_MAX_P     = 0x7f7fffff;
const unsigned int FLT_MAX_N     = 0xff7fffff;
const unsigned int FLT_MIN_P     = 0x00800000;
const unsigned int FLT_MIN_N     = 0x80800000;
const unsigned int FLT_ZERO_P    = 0x00000000; // 仮数部が非0の時Denとなる
                                               // (cop1では未サポート)
const unsigned int FLT_ZERO_N    = 0x80000000; // 仮数部が非0の時Denとなる
                                               // (cop1では未サポート)

};  // namespace COP1

#define CASTINT(_v)   *((int*)&(_v))
#define CASTUINT(_v)  *((unsigned int*)&(_v))
#define CASTFLOAT(_v) *((float*)&(_v))

// 仮想アドレスから実メモリアドレスへ変換しeaxに格納される
//（内部でeax,ecx,edxを使用するので注意！）
// MIPSアセンブラの仕様的でアラインされたメモリにしか読み書きできないので
// ページ境界を跨ぐ場合については考慮する必要は無い
#define _va2pa_mask(_mask, _base, _offset)          \
  mov32_reg_mem(_edx, _GPR_D(_base, 0));            \
  if (_offset != 0) add32_reg_imm32(_edx, _offset); \
  mov32_reg_reg(_eax, _edx);                        \
  mov32_reg_imm32(_ecx, (u32)g_memory->att());      \
  /* _eax = (_base.0-31 + _offset) >> 0x20 */       \
  shr32_reg_imm8(_edx, 20);                         \
  /* _eax = &g_att[_eax] */                         \
  lea_reg_reg_p_reg_x_4(_edx, _ecx, _edx);          \
  /* _edx = (_base.0-31 + _offset) & 0xffffff */    \
  and32_reg_imm32(_eax, _mask);                     \
  add32_reg_rmem(_eax, _edx);

//-----------------------------------------------------------------------------
// class
class GPR {
public:
  union {
    u128 x;
    u64  q[2];
    u32  d[4];
    u16  w[8];
    u8   b[16];
  };
};

class HILO {
public:
  union {
    u128 x;
    u64  q[2];
    u32  d[4];
  };
};

class R5900REGS {
public:
  R5900REGS() {}
  ~R5900REGS() {}

  void init() {
    // cpu
    for (int i = 0; i < 32; i++) {
      m_gpr[i].q[0] = m_gpr[i].q[1] = 0;
    }
    m_hi.q[0] = m_hi.q[1] = 0;
    m_lo.q[0] = m_lo.q[1] = 0;
    m_sa = 0;
    m_pc = 0x00000000;

    // fpu
    for (int i = 0; i < 32; i++) {
      m_fpu.fpr[i] = 0.0f;
    }
    m_fpu.acc       = 0.0f;
    m_fpu.fcr0.zero = 0x0000;
    m_fpu.fcr0.imp  = 0x00; // 0x2e
    m_fpu.fcr0.rev  = 0x00;
    m_fpu.fcr31     = 0;

    // cop0
    for (int i = 0; i < 32; i++) {
      m_cop0.cpr[i] = 0;
    }
    m_cop0.status   = 0x00038c01; // 全割り込み許可

    m_ee_jump_addr  = 0xffffffff;
    m_x86_jump_addr = 0x00000000;
    m_null_current_inst = false;

#if defined _DEBUGGER
    m_cycle_count = 0;
#endif // defined _DEBUGGER
  }

  virtual void dump();

  // general purpose registers
  __declspec(align(16)) union {
    struct {
      GPR m_ze, m_at, m_v0, m_v1, m_a0, m_a1, m_a2, m_a3,
          m_t0, m_t1, m_t2, m_t3, m_t4, m_t5, m_t6, m_t7,
          m_s0, m_s1, m_s2, m_s3, m_s4, m_s5, m_s6, m_s7,
          m_t8, m_t9, m_k0, m_k1, m_gp, m_sp, m_fp, m_ra;
    };
    GPR m_gpr[32];
  };
  __declspec(align(16)) HILO  m_hi;
  __declspec(align(16)) HILO  m_lo;
  u32 m_sa; // shift amount
  u32 m_pc; // program counter
  
  // cop1(fpu) registers
  struct {
    f32 fpr[32];
    f32 acc;
    union {
      struct {
        u16 zero;
        u8  imp;
        u8  rev;
      };
      u32 d;
    } fcr0;
    u32 fcr31;

    u32 v_tmp; // 架空のレジスタ（実装の都合で作成）
  } m_fpu;

  // cop0 registers
  struct {
    union {
      struct {
        u32 index;
        u32 random;
        u32 entrylo0;
        u32 entrylo1;
        u32 context;
        u32 pagemask;
        u32 wired;
        u32 _reserved7;
        u32 badvaddr;
        u32 count;
        u32 entryhi;
        u32 compare;
        union {
          struct {
            u32 status_ie:1;
            u32 status_exl:1;
            u32 status_erl:1;
            u32 status_ksu:2;
            u32 status_rsv05_09:5;
            u32 status_im:2;
            u32 status_bem:1;
            u32 status_rsv13_14:2;
            u32 status_im7:1;
            u32 status_eie:1;
            u32 status_edi:1;
            u32 status_ch:1;
            u32 status_rsv19_21:3;
            u32 status_bev:1;
            u32 status_dev:1;
            u32 status_rsv24_27:4;
            u32 status_cu:4;
          };
          u32 status;
        };
        u32 cause;
        u32 epc;
        u32 prid;
        u32 config;
        u32 _reserved17;
        u32 _reserved18;
        u32 _reserved19;
        u32 _reserved20;
        u32 _reserved21;
        u32 _reserved22;
        u32 badpaddr;
        u32 bpc;
        u32 pccr;
        u32 _reserved26;
        u32 _reserved27;
        u32 taglo;
        u32 taghi;
        u32 errorepc;
        u32 _reserved31;
      };
      u32 cpr[32];
    };
    // id24 重複
    u32  iab;
    u32  iabm;
    u32  dab;
    u32  dabm;
    u32  dvb;
    u32  dvbm;
    // id25 重複
    u32  pcr0;
    u32 pcr1;
  } m_cop0;

  u32  m_ee_jump_addr;
  u32  m_x86_jump_addr;
  bool m_null_current_inst;

#if defined _DEBUGGER
  u32  m_cycle_count;
#endif //defined _DEBUGGER
};

class CodeBlock {
public:
  inline CodeBlock() :
  idx(0xffffffff),
  buf(0x00000000),
  capacity(0),
  size(0),
  ee_inst_count(0) {
  }

  inline ~CodeBlock() {
    if (buf) {
      free(buf);
      buf = NULL;
    }
  }

public:
  u32 idx;           // マッピングテーブルの先頭インデックス
  u8* buf;           // 生成コードバッファ
  u32 capacity;      // バッファサイズ
  u32 size;          // 書き込み済みコードサイズ
  u32 ee_inst_count; // バッファの持つEE命令数
};

class R5900 : public R5900REGS {
// constructor/destructor
public:
  R5900();
  ~R5900();

#if defined _DEBUG
  void* operator new(size_t p_sz, int p_block,
                     const char * p_file, int p_line) {
    void* ptr = _aligned_malloc_dbg(p_sz, 16, p_file, p_line);
    return ptr;
  }
  void operator delete(void* p_ptr) {
    _aligned_free_dbg(p_ptr);
  }
#else
  void* operator new(size_t p_sz) {
    void* ptr = _aligned_malloc(p_sz, 16);
    return ptr;
  }
  void operator delete(void* p_ptr) {
    _aligned_free(p_ptr);
  }
#endif

// member function
public:
  void init();
  void clearRecompiledCode();
  void create_recompile_cache();
  void clear_recompile_cache();
  u8*  recompile(u32 p_va, u32 p_count, bool p_enable_macro_recompile = true);
  void execute(bool p_enable_macro_recompile = true, bool p_continue = true);

#if defined _DEBUGGER
  void enable_break(u32 p_pa, u8* p_savebyte);
  void disable_break(u32 p_pa, u8 p_savebyte);

  void init_run_count();
  void create_recompiler_hint();
  void load_recompile_hint();
  void save_recompile_hint();
#endif // defined _DEBUGGER

// member variable
public:
  // リコンパイル済み(x86)コード配列
  vector<CodeBlock*> m_recompiled_code_ary;

  // リコンパイル済み(x86)コードマッピングテーブル
  u8** m_recompiled_code_table;
  
  // リコンパイル中のopcode
  u32  m_recompiling_opcode;

  // リコンパイル中のEE仮想アドレス
  u32  m_recompiling_ee_va;

  // リコンパイル中の生成コード書き込みポインタ
  u8*  m_recompiling_code_ptr;

#if defined _DEBUGGER
  vector<u8>  m_recompile_hint_ary; // 最適化ヒント情報
  vector<u32> m_run_count_ary;      // 実行カウント
#endif // defined _DEBUGGER
};

//-----------------------------------------------------------------------------
// extern
extern R5900* g_cpu;

#endif // _cpu_r5900
