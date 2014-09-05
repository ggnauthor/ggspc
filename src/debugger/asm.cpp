#include "main.h"
#include "memory/memory.h"
#include "debugger/asm.h"
#include "debugger/debugger.h"

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {
  char _cpr[32][10] = {
    "index",   "random",   "entrylo0", "entrylo1",
    "context", "pagemask", "wired",    "(n/a)",
    "badvaddr", "count",   "entryhi",  "compare",
    "status",  "cause",    "epc",      "prid",
    "config",  "(n/a)",    "(n/a)",    "(n/a)",
    "(n/a)",   "(n/a)",    "(n/a)",    "badpaddr",
    "(debug)", "(perf)",   "(n/a)",    "(n/a)",
    "taglo",   "taghi",    "errorepc", "(n/a)"
  };
}

typedef struct INSTENTRY {
  char label[16];
  u32  (*func)(struct INSTENTRY*, u32, char*, char*, char*, char*);
  u32  inst1;
  u32  inst2;
  u32  inst3;
  int  argc;
}INSTENTRY;

const char ERR_EMPTY_ARGUMENT[]          = "Assemble Error : Found Empty Argument.";
const char ERR_TOO_MANY_OPERANDS[]       = "Assemble Error : Too many operands.";
const char ERR_TOO_FEW_OPERANDS[]        = "Assemble Error : Too few operands.";
const char ERR_UNKNOWN_INSTRACTION[]     = "Assemble Error : Unknown instruction.";
const char ERR_UNKNOWN_GPR[]             = "Assemble Error : Unknown GPR.";
const char ERR_UNKNOWN_FPR[]             = "Assemble Error : Unknown FPR.";
const char ERR_BAD_CHARACTER[]           = "Assemble Error : Found bad character in numeric operand.";
const char ERR_OUT_OF_RANGE_S5BITS[]     = "Assemble Error : Out of range [-16~15].";
const char ERR_OUT_OF_RANGE_U5BITS[]     = "Assemble Error : Out of range [0~31].";
const char ERR_OUT_OF_RANGE_U20BITS[]    = "Assemble Error : Out of range [0x00000~0xfffff].";
const char ERR_OUT_OF_RANGE_OFS16[]      = "Assemble Error : Out of range [PC-0x1fffc~PC+0x20000].";
const char ERR_ADDR_ALIGNMENT[]          = "Assemble Error : Address must be 4 bytes align.";
const char ERR_OUT_OF_RANGE_U16BITS[]    = "Assemble Error : Out of range [0x0000~0xffff].";
const char ERR_OUT_OF_RANGE_ADDR26[]     = "Assemble Error : Out of memory region.";
const char ERR_WRONG_FORMAT_OFFSETBASE[] = "Assemble Error : Wrong format [ex. 0x0004(v0)].";

#define TO_GPR(_nam)                                    \
  int _nam = gprID(p_##_nam);                           \
  if (_nam == -1) {                                     \
    sprintf(p_err, "%s %s", ERR_UNKNOWN_GPR, p_##_nam); \
    return 0;                                           \
  }

#define TO_FPR(_nam)                                    \
  int _nam = fprID(p_##_nam);                           \
  if (_nam == -1) {                                     \
    sprintf(p_err, "%s %s", ERR_UNKNOWN_FPR, p_##_nam); \
    return 0;                                           \
  }

#define TO_INT(_num, _min, _max, _err)  \
  int _num;                             \
  if (!toNum(p_##_num, &_num)) {        \
    strcpy(p_err, ERR_BAD_CHARACTER);   \
    return 0;                           \
  }                                     \
  if ((_num) < _min || (_num) > _max) { \
    strcpy(p_err, _err);                \
    return 0;                           \
  }

#define TO_S5BITS(_num)  TO_INT(_num, -16, 15, ERR_OUT_OF_RANGE_S5BITS)
#define TO_U5BITS(_num)  TO_INT(_num, 0, 31, ERR_OUT_OF_RANGE_U5BITS)
#define TO_U20BITS(_num) TO_INT(_num, 0, 0xfffff, ERR_OUT_OF_RANGE_U20BITS)
#define TO_S16BITS(_num) TO_INT(_num, -0x8000, 0x7fff, ERR_OUT_OF_RANGE_S16BITS)
#define TO_U16BITS(_num) TO_INT(_num, 0x0000, 0xffff, ERR_OUT_OF_RANGE_U16BITS)

int gprID(char* p_str) {
  static char _gpr[32][12] = {
    "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
    "t0",   "t1", "t2", "t3", "t4", "t5", "t6", "t7",
    "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7",
    "t8",   "t9", "k0", "k1", "gp", "sp", "fp", "ra",
  };
  for (int i = 0; i < 32; i++) {
    if (strcmp(_gpr[i], p_str) == 0) return i;
  }
  return -1;
}

int fprID(char* p_str) {
  if (strncmp(p_str, "fpr", 3) != 0) return -1;
  if (p_str[3] == '\0') return -1;
  
  int num = p_str[3] - '0';
  if (p_str[4] == '\0') {
    return num;
  } else if (p_str[5] == '\0') {
    num = num * 10 + (p_str[4] - '0');
    return num > 31 ? -1 : num;
  } else {
    return -1;
  }
}

bool toNum(char* p_str, int* p_num) {
  char badchars[1024] = "";
  char *bcptr = NULL;
  // 第3引数を0にするとCの定数表記に従う。0x=16進, 0=8進, それ以外10進
  *p_num = strtoul(p_str, &bcptr, 0);
  if (bcptr[0] != '\0') return false;
  return true;
}

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(5)  |  0(6)  |
// |  inst1 |    0   |   rt   |   rd   |   sa   |  inst2 |
// +--------+-----------------+--------+--------+--------+
u32 asm_rd_rt_sa(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_rd, char* p_rt, char* p_sa) {
  TO_GPR(rd);
  TO_GPR(rt);
  TO_U5BITS(sa);
  return p_ie->inst1 << 26 | rt << 16 | rd << 11 | sa << 6 | p_ie->inst2;
}

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(5)  |  0(6)  |
// |  inst1 |   rs   |   rt   |   rd   |   0    |  inst2 |
// +--------+-----------------+--------+--------+--------+
u32 asm_rd_rt_rs(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_rd, char* p_rt, char* p_rs) {
  TO_GPR(rs);
  TO_GPR(rt);
  TO_GPR(rd);
  return p_ie->inst1 << 26 | rs << 21 | rt << 16 |
         rd << 11 | p_ie->inst3 << 6 | p_ie->inst2;
}
u32 asm_rd_rs_rt(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_rd, char* p_rs, char* p_rt) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, p_rd, p_rt, p_rs);
}
u32 asm_rd_rs(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char* p_rd, char* p_rs, char*) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, p_rd, "zero", p_rs);
}
u32 asm_rs_rt(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char* p_rs, char* p_rt, char*) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, "zero", p_rt, p_rs);
}
u32 asm_rd(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
           char* p_rd, char*, char*) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, p_rd, "zero", "zero");
}
u32 asm_rs(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
           char* p_rs, char*, char*) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, "zero", "zero", p_rs);
}
u32 asm_ra_rs(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char* p_rs, char*, char*) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, "ra", "zero", p_rs);
}
u32 asm_noarg(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char*, char*, char*) {
  return asm_rd_rt_rs(p_ie, p_va, p_err, "zero", "zero", "zero");
}

// +--------+-----------------------------------+--------+
// |  26(6) |                6(20)              |  0(6)  |
// |  inst1 |                code               |  inst2 |
// +--------+-----------------------------------+--------+
u32 asm_code(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
             char* p_code, char*, char*) {
  TO_U20BITS(code);
  return p_ie->inst1 << 26 | code << 6 | p_ie->inst2;
}

// +--------+-----------------+--------------------------+
// |  26(6) |  21(5) |  16(5) |          0(16)           |
// |  inst1 |   rs   |  inst2 |          offset          |
// +--------+-----------------+--------------------------+
u32 asm_rs_ofs16(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_rs, char* p_ofs, char*) {
  TO_GPR(rs);
  int ofs;              
  if (!toNum(p_ofs, &ofs)) {      
    strcpy(p_err, ERR_BAD_CHARACTER);      
    return 0;                  
  }
  if (ofs % 4 > 0) {
    strcpy(p_err, ERR_ADDR_ALIGNMENT);  
    return 0;  
  }
  ofs = (s32)(ofs - p_va - 4);
  if (ofs < -0x20000 || ofs > 0x0001fffc) {  
    strcpy(p_err, ERR_OUT_OF_RANGE_OFS16);  
    return 0;                  
  }
  return p_ie->inst1 << 26 | rs << 21 |
         p_ie->inst2 << 16 | ((ofs >> 2) & 0xffff);
}

// +--------+-----------------+--------------------------+
// |  26(6) |  21(5) |  16(5) |          0(16)           |
// |  inst1 |   rs   |  inst2 |        immediate         |
// +--------+-----------------+--------------------------+
u32 asm_rs_imm16(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_rs, char* p_imm, char*) {
  TO_GPR(rs);
  TO_U16BITS(imm);
  return p_ie->inst1 << 26 | rs << 21 |
         p_ie->inst2 << 16 | (imm & 0xffff);
}

// +--------+--------------------------------------------+
// |  26(6) |                   0(26)                    |
// |  inst1 |                   offset                   |
// +--------+--------------------------------------------+
u32 asm_addr26(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
               char* p_addr, char*, char*) {
  int addr;              
  if (!toNum(p_addr, &addr)) {      
    strcpy(p_err, ERR_BAD_CHARACTER);      
    return 0;                  
  }
  if (addr % 4 > 0) {
    strcpy(p_err, ERR_ADDR_ALIGNMENT);  
    return 0;  
  }
  if ((addr & 0xf0000000) != (p_va & 0xf0000000)) {  
    strcpy(p_err, ERR_OUT_OF_RANGE_ADDR26);  
    return 0;                  
  }
  return p_ie->inst1 << 26 | ((p_va & 0xf0000000) + addr) >> 2;
}

// +--------+-----------------+--------------------------+
// |  26(6) |  21(5) |  16(5) |          0(16)           |
// |  inst1 |   rs   |   rt   |          offset          |
// +--------+-----------------+--------------------------+
u32 asm_rs_rt_ofs16(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                    char* p_rs, char* p_rt, char* p_ofs) {
  TO_GPR(rs);
  TO_GPR(rt);
  int ofs;              
  if (!toNum(p_ofs, &ofs)) {      
    strcpy(p_err, ERR_BAD_CHARACTER);      
    return 0;                  
  }
  if (ofs % 4 > 0) {
    strcpy(p_err, ERR_ADDR_ALIGNMENT);  
    return 0;  
  }
  ofs = (s32)(ofs - p_va - 4);
  if (ofs < -0x20000 || ofs > 0x0001fffc) {  
    strcpy(p_err, ERR_OUT_OF_RANGE_OFS16);  
    return 0;                  
  }
  return p_ie->inst1 << 26 | rs << 21 | rt << 16 | ((ofs >> 2) & 0xffff);
}

// +--------+-----------------+--------------------------+
// |  26(6) |  21(5) |  16(5) |          0(16)           |
// |  inst1 |   rs   |   rt   |          offset          |
// +--------+-----------------+--------------------------+
u32 asm_rt_rs_imm16(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                    char* p_rt, char* p_rs, char* p_imm) {
  TO_GPR(rs);
  TO_GPR(rt);
  TO_U16BITS(imm);
  return p_ie->inst1 << 26 | rs << 21 | rt << 16 | (imm & 0xffff);
}

u32 asm_rt_imm16(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_rt, char* p_imm, char*) {
  TO_GPR(rt);
  TO_U16BITS(imm);
  return p_ie->inst1 << 26 | rt << 16 | (imm & 0xffff);
}

// +--------+-----------------+--------------------------+
// |  26(6) |  21(5) |  16(5) |          0(16)           |
// |  inst1 |  base  |   rt   |          offset          |
// +--------+-----------------+--------------------------+
u32 asm_rt_ofsbase(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                   char* p_rt, char* p_ofsbase, char*) {
  TO_GPR(rt);
  char* p_ofs = p_ofsbase;
  char* p_base = strchr(p_ofsbase, '(');
  char* end = strchr(p_ofsbase, ')');
  if (p_base) *p_base++ = '\0';
  if (end) *end = '\0';
  if (p_ofs == NULL || p_base == NULL) {
    strcpy(p_err, ERR_WRONG_FORMAT_OFFSETBASE);
    return 0;                  
  }
  TO_U16BITS(ofs);
  TO_GPR(base);
  return p_ie->inst1 << 26 | base << 21 | rt << 16 | (ofs & 0xffff);
}

u32 asm_ft_ofsbase(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                   char* p_ft, char* p_ofsbase, char*) {
  TO_FPR(ft);
  char* p_ofs = p_ofsbase;
  char* p_base = strchr(p_ofsbase, '(');
  char* end = strchr(p_ofsbase, ')');
  if (p_base) *p_base++ = '\0';
  if (end) *end = '\0';
  if (p_ofs == NULL || p_base == NULL) {
    strcpy(p_err, ERR_WRONG_FORMAT_OFFSETBASE);  
    return 0;                  
  }
  TO_U16BITS(ofs);
  TO_GPR(base);
  return p_ie->inst1 << 26 | base << 21 | ft << 16 | (ofs & 0xffff);
}

u32 asm_imm_ofsbase(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                    char* p_imm, char* p_ofsbase, char*) {
  TO_U5BITS(imm);
  char* p_ofs = p_ofsbase;
  char* p_base = strchr(p_ofsbase, '(');
  char* end = strchr(p_ofsbase, ')');
  if (p_base) *p_base++ = '\0';
  if (end) *end = '\0';
  if (p_ofs == NULL || p_base == NULL) {
    strcpy(p_err, ERR_WRONG_FORMAT_OFFSETBASE);  
    return 0;                  
  }
  TO_U16BITS(ofs);
  TO_GPR(base);
  return p_ie->inst1 << 26 | base << 21 | imm << 16 | (ofs & 0xffff);
}

// +--------+-----------------+--------+--------+--------+
// |  26(6) |  21(5) |  16(5) |  11(5) |  6(5)  |  0(6)  |
// |  inst1 |   rs   |   rt   |   rd   |   0    |  inst2 |
// +--------+-----------------+--------+--------+--------+
u32 asm_fd_fs_ft(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                 char* p_fd, char* p_fs, char* p_ft) {
  TO_FPR(fd);
  TO_FPR(fs);
  TO_FPR(ft);
  return p_ie->inst1 << 26 | p_ie->inst2 << 21 |
         ft << 16 | fs << 11 | fd << 6 | p_ie->inst3;
}
u32 asm_fd_fs(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char* p_fd, char* p_fs, char*) {
  TO_FPR(fd);
  TO_FPR(fs);
  return p_ie->inst1 << 26 | p_ie->inst2 << 21 |
         fs << 11 | fd << 6 | p_ie->inst3;
}
u32 asm_fd_ft(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char* p_fd, char* p_ft, char*) {
  TO_FPR(fd);
  TO_FPR(ft);
  return p_ie->inst1 << 26 | p_ie->inst2 << 21 |
         ft << 16 | fd << 6 | p_ie->inst3;
}
u32 asm_fs_ft(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
              char* p_fs, char* p_ft, char*) {
  TO_FPR(fs);
  TO_FPR(ft);
  return p_ie->inst1 << 26 | p_ie->inst2 << 21 |
         ft << 16 | fs << 11 | p_ie->inst3;
}
u32 asm_cop1_ofs16(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                   char* p_ofs, char*, char*) {
  int ofs;              
  if (!toNum(p_ofs, &ofs)) {      
    strcpy(p_err, ERR_BAD_CHARACTER);      
    return 0;                  
  }
  if (ofs % 4 > 0) {
    strcpy(p_err, ERR_ADDR_ALIGNMENT);  
    return 0;  
  }
  ofs = (s32)(ofs - p_va - 4);
  if (ofs < -0x20000 || ofs > 0x0001fffc) {  
    strcpy(p_err, ERR_OUT_OF_RANGE_OFS16);  
    return 0;                  
  }
  return p_ie->inst1 << 26 | p_ie->inst2 << 21 |
         p_ie->inst3 << 16 | ((ofs >> 2) & 0xffff);
}
u32 asm_cop1_rt_fs(struct INSTENTRY* p_ie, u32 p_va, char* p_err,
                   char* p_rt, char* p_fs, char*) {
  TO_GPR(rt); TO_FPR(fs);
  return p_ie->inst1 << 26 | p_ie->inst2 << 21 | rt << 16 | fs << 11;
}

u32 asmR5900(char* p_str, u32 p_va, char* p_err) {
//                                 label func             instructions    argc
#define SPECIAL(_lbl, _i)          _lbl, asm_noarg,       0x00, _i, 0x00, 0,
#define SPECIAL_RD_RT_SA(_lbl, _i) _lbl, asm_rd_rt_sa,    0x00, _i, 0x00, 3,
#define SPECIAL_RD(_lbl, _i)       _lbl, asm_rd,          0x00, _i, 0x00, 1,
#define SPECIAL_RS(_lbl, _i)       _lbl, asm_rs,          0x00, _i, 0x00, 1,
#define SPECIAL_RA_RS(_lbl, _i)    _lbl, asm_ra_rs,       0x00, _i, 0x00, 1,
#define SPECIAL_RD_RS(_lbl, _i)    _lbl, asm_rd_rs,       0x00, _i, 0x00, 2,
#define SPECIAL_RS_RT(_lbl, _i)    _lbl, asm_rs_rt,       0x00, _i, 0x00, 2,
#define SPECIAL_RD_RS_RT(_lbl, _i) _lbl, asm_rd_rs_rt,    0x00, _i, 0x00, 3,
#define SPECIAL_RD_RT_RS(_lbl, _i) _lbl, asm_rd_rt_rs,    0x00, _i, 0x00, 3,
#define SPECIAL_CODE(_lbl, _i)     _lbl, asm_code,        0x00, _i, 0x00, 1,
#define SPECIAL_SYNC(_lbl, _i, _t) _lbl, asm_noarg,       0x00, _i, _t,   0,

#define REGIMM_RS_OFS16(_lbl, _i)  _lbl, asm_rs_ofs16,    0x01, _i, 0x00, 2,
#define REGIMM_RS_IMM16(_lbl, _i)  _lbl, asm_rs_imm16,    0x01, _i, 0x00, 2,

#define MISC_ADDR26(_lbl, _i)      _lbl, asm_addr26,      _i, 0x00, 0x00, 1,
#define MISC_RS_RT_OFS16(_lbl, _i) _lbl, asm_rs_rt_ofs16, _i, 0x00, 0x00, 3,
#define MISC_RS_OFS16(_lbl, _i)    _lbl, asm_rs_ofs16,    _i, 0x00, 0x00, 2,
#define MISC_RT_RS_IMM16(_lbl, _i) _lbl, asm_rt_rs_imm16, _i, 0x00, 0x00, 3,
#define MISC_RT_IMM16(_lbl, _i)    _lbl, asm_rt_imm16,    _i, 0x00, 0x00, 2,
#define MISC_RT_OFSBASE(_lbl, _i)  _lbl, asm_rt_ofsbase,  _i, 0x00, 0x00, 2,
#define MISC_FT_OFSBASE(_lbl, _i)  _lbl, asm_ft_ofsbase,  _i, 0x00, 0x00, 2,
#define MISC_IMM_OFSBASE(_lbl, _i) _lbl, asm_imm_ofsbase, _i, 0x00, 0x00, 2,

#define COP1_S_FD_FS_FT(_lbl, _i)  _lbl, asm_fd_fs_ft,    0x11, 0x10, _i, 3,
#define COP1_S_FD_FS(_lbl, _i)     _lbl, asm_fd_fs,       0x11, 0x10, _i, 2,
#define COP1_S_FD_FT(_lbl, _i)     _lbl, asm_fd_ft,       0x11, 0x10, _i, 2,
#define COP1_S_FS_FT(_lbl, _i)     _lbl, asm_fs_ft,       0x11, 0x10, _i, 2,
#define COP1_BC1_OFS(_lbl, _i)     _lbl, asm_cop1_ofs16,  0x11, 0x08, _i, 1,
#define COP1_W_FD_FS(_lbl, _i)     _lbl, asm_fd_fs,       0x11, 0x14, _i, 2,
#define COP1_RT_FS(_lbl, _i)       _lbl, asm_cop1_rt_fs,  0x11, _i, 0x00, 2,

  static INSTENTRY table[] = {
    // 同名の命令は隣接させること
    //               label      inst2
    SPECIAL(         "nop",     0x00)
    SPECIAL_RD_RT_SA("sll",     0x00)
    SPECIAL_RD_RT_SA("srl",     0x02)
    SPECIAL_RD_RT_SA("sra",     0x03)
    SPECIAL_RD_RT_SA("sllv",    0x04)
    SPECIAL_RD_RT_SA("srlv",    0x06)
    SPECIAL_RD_RT_SA("srav",    0x07)

    SPECIAL_RS(      "jr",      0x08)
    SPECIAL_RA_RS(   "jalr",    0x09)
    SPECIAL_RD_RS(   "jalr",    0x09)
    SPECIAL_RD_RS_RT("movz",    0x0a)
    SPECIAL_RD_RS_RT("movn",    0x0b)

    SPECIAL(         "syscall", 0x0c)
    SPECIAL_CODE(    "syscall", 0x0c)
    SPECIAL(         "break",   0x0d)
    SPECIAL_CODE(    "break",   0x0d)

    //           label      inst2  stype
    SPECIAL_SYNC("sync",    0x0f,  0x00)
    SPECIAL_SYNC("sync.l",  0x0f,  0x00)
    SPECIAL_SYNC("sync.p",  0x0f,  0x10)

    //               label     inst2
    SPECIAL_RD(      "mfhi",   0x10)
    SPECIAL_RS(      "mthi",   0x11)
    SPECIAL_RD(      "mflo",   0x12)
    SPECIAL_RS(      "mtlo",   0x13)
    SPECIAL_RD_RT_RS("dsllv",  0x14)
    SPECIAL_RD_RT_RS("dsrlv",  0x16)
    SPECIAL_RD_RT_RS("dsrav",  0x17)

    SPECIAL_RS_RT(   "dsrav",  0x18)
    SPECIAL_RS_RT(   "dsrav",  0x19)
    SPECIAL_RS_RT(   "dsrav",  0x1a)
    SPECIAL_RS_RT(   "dsrav",  0x1b)

    SPECIAL_RD_RS_RT("add",    0x20)
    SPECIAL_RD_RS_RT("addu",   0x21)
    SPECIAL_RD_RS_RT("sub",    0x22)
    SPECIAL_RD_RS_RT("subu",   0x23)
    SPECIAL_RD_RS_RT("and",    0x24)
    SPECIAL_RD_RS_RT("or",     0x25)
    SPECIAL_RD_RS_RT("xor",    0x26)
    SPECIAL_RD_RS_RT("nor",    0x27)

    SPECIAL_RD(      "mfsa",   0x28)
    SPECIAL_RS(      "mtsa",   0x29)
    SPECIAL_RD_RS_RT("slt",    0x2a)
    SPECIAL_RD_RS_RT("sltu",   0x2b)
    SPECIAL_RD_RS_RT("dadd",   0x2c)
    SPECIAL_RD_RS_RT("daddu",  0x2d)
    SPECIAL_RD_RS_RT("dsub",   0x2e)
    SPECIAL_RD_RS_RT("dsubu",  0x2f)

    SPECIAL_RS_RT(   "tge",    0x30)
    SPECIAL_RS_RT(   "tgeu",   0x31)
    SPECIAL_RS_RT(   "tlt",    0x32)
    SPECIAL_RS_RT(   "tltu",   0x33)
    SPECIAL_RS_RT(   "teq",    0x34)
    SPECIAL_RS_RT(   "tne",    0x36)

    SPECIAL_RD_RT_SA("dsll",   0x38)
    SPECIAL_RD_RT_SA("dsrl",   0x3a)
    SPECIAL_RD_RT_SA("dsra",   0x3b)
    SPECIAL_RD_RT_SA("dsll32", 0x3c)
    SPECIAL_RD_RT_SA("dsrl32", 0x3e)
    SPECIAL_RD_RT_SA("dsra32", 0x3f)

    //              label      inst2
    REGIMM_RS_OFS16("bltz",    0x00)
    REGIMM_RS_OFS16("bgez",    0x01)
    REGIMM_RS_OFS16("bltzl",   0x02)
    REGIMM_RS_OFS16("bgezl",   0x03)

    REGIMM_RS_OFS16("tgei",    0x08)
    REGIMM_RS_OFS16("tgeiu",   0x09)
    REGIMM_RS_OFS16("tlti",    0x0a)
    REGIMM_RS_OFS16("tltiu",   0x0b)
    REGIMM_RS_OFS16("teqi",    0x0c)
    REGIMM_RS_OFS16("tnei",    0x0e)

    REGIMM_RS_OFS16("bltzal",  0x10)
    REGIMM_RS_OFS16("bgezal",  0x11)
    REGIMM_RS_OFS16("bltzall", 0x12)
    REGIMM_RS_OFS16("bgezall", 0x13)
    REGIMM_RS_IMM16("mtsab",   0x18)
    REGIMM_RS_IMM16("mtsah",   0x19)

    //               label     inst1
    MISC_ADDR26(     "j",      0x02)
    MISC_ADDR26(     "jal",    0x03)
    MISC_RS_RT_OFS16("beq",    0x04)
    MISC_RS_RT_OFS16("bne",    0x05)
    MISC_RS_OFS16(   "blez",   0x06)
    MISC_RS_OFS16(   "bgtz",   0x07)

    MISC_RT_RS_IMM16("addi",   0x08)
    MISC_RT_RS_IMM16("addiu",  0x09)
    MISC_RT_RS_IMM16("slti",   0x0a)
    MISC_RT_RS_IMM16("sltiu",  0x0b)
    MISC_RT_RS_IMM16("andi",   0x0c)
    MISC_RT_RS_IMM16("ori",    0x0d)
    MISC_RT_RS_IMM16("xori",   0x0e)
    MISC_RT_IMM16(   "lui",    0x0f)

    MISC_RS_RT_OFS16("beql",   0x14)
    MISC_RS_RT_OFS16("bnel",   0x15)
    MISC_RS_OFS16(   "blezl",  0x16)
    MISC_RS_OFS16(   "bgtzl",  0x17)

    MISC_RT_RS_IMM16("daddi",  0x18)
    MISC_RT_RS_IMM16("daddiu", 0x19)
    MISC_RT_OFSBASE( "ldl",    0x1a)
    MISC_RT_OFSBASE( "ldr",    0x1b)
    MISC_RT_OFSBASE( "lq",     0x1e)
    MISC_RT_OFSBASE( "sq",     0x1f)
    
    MISC_RT_OFSBASE( "lb",     0x20)
    MISC_RT_OFSBASE( "lh",     0x21)
    MISC_RT_OFSBASE( "lwl",    0x22)
    MISC_RT_OFSBASE( "lw",     0x23)
    MISC_RT_OFSBASE( "lbu",    0x24)
    MISC_RT_OFSBASE( "lhu",    0x25)
    MISC_RT_OFSBASE( "lwr",    0x26)
    MISC_RT_OFSBASE( "lwu",    0x27)
    
    MISC_RT_OFSBASE( "sb",     0x28)
    MISC_RT_OFSBASE( "sh",     0x29)
    MISC_RT_OFSBASE( "swl",    0x2a)
    MISC_RT_OFSBASE( "sw",     0x2b)
    MISC_RT_OFSBASE( "sbl",    0x2c)
    MISC_RT_OFSBASE( "shr",    0x2d)
    MISC_RT_OFSBASE( "swr",    0x2e)
    MISC_RT_OFSBASE( "cache",  0x2f)

    MISC_FT_OFSBASE( "lwc1",   0x31)
    MISC_IMM_OFSBASE("pref",   0x33)
    MISC_RT_OFSBASE( "lqc2",   0x36)
    MISC_RT_OFSBASE( "ld",     0x37)

    MISC_FT_OFSBASE( "swc1",   0x39)
    MISC_RT_OFSBASE( "sqc2",   0x3e)
    MISC_RT_OFSBASE( "sd",     0x3f)

    //               label     inst3
    COP1_S_FD_FS_FT( "add.s",  0x00)
    COP1_S_FD_FS_FT( "sub.s",  0x01)
    COP1_S_FD_FS_FT( "mul.s",  0x02)
    COP1_S_FD_FS_FT( "div.s",  0x03)
    COP1_S_FD_FT(    "sqrt.s", 0x04)
    COP1_S_FD_FS(    "abs.s",  0x05)
    COP1_S_FD_FS(    "mov.s",  0x06)
    COP1_S_FD_FS(    "neg.s",  0x07)

    COP1_S_FD_FS_FT( "rsqrt.s", 0x16)

    COP1_S_FS_FT(    "adda.s",  0x18)
    COP1_S_FS_FT(    "suba.s",  0x19)
    COP1_S_FS_FT(    "mula.s",  0x1a)
    COP1_S_FD_FS_FT( "madd.s",  0x1c)
    COP1_S_FD_FS_FT( "msub.s",  0x1d)
    COP1_S_FS_FT(    "madda.s", 0x1e)
    COP1_S_FS_FT(    "msuba.s", 0x1f)

    COP1_S_FD_FS(    "cvt.w.s", 0x24)

    COP1_S_FD_FS_FT( "max.s",   0x28)
    COP1_S_FD_FS_FT( "min.s",   0x29)

    COP1_S_FS_FT(    "c.f.s",   0x30)
    COP1_S_FS_FT(    "c.eq.s",  0x32)
    COP1_S_FS_FT(    "c.lt.s",  0x34)
    COP1_S_FS_FT(    "c.le.s",  0x36)

    COP1_W_FD_FS(    "cvt.s.w", 0x20)

    COP1_BC1_OFS(    "bc1f",    0x00)
    COP1_BC1_OFS(    "bc1t",    0x01)
    COP1_BC1_OFS(    "bc1fl",   0x02)
    COP1_BC1_OFS(    "bc1tl",   0x03)

    COP1_RT_FS(      "mfc1",    0x00)
    COP1_RT_FS(      "cfc1",    0x02)
    COP1_RT_FS(      "mtc1",    0x04)
    COP1_RT_FS(      "ctc1",    0x06)

    "",  NULL, 0x00, 0x00, 0x00, 0, // end of array
  };

  ds_util::strtrim(p_str);

  int   wordcount = 0;
  char* words[6] = { 0, 0, 0, 0, 0, 0 };
  words[wordcount++] = p_str;
  if (words[wordcount - 1])
    words[wordcount++] = strchr(p_str, ' ');
  if (words[wordcount - 1])
    words[wordcount++] = strchr(words[wordcount - 1] + 1, ',');
  if (words[wordcount - 1])
    words[wordcount++] = strchr(words[wordcount - 1] + 1, ',');
  if (words[wordcount - 1])
    words[wordcount++] = strchr(words[wordcount - 1] + 1, ',');
  if (words[wordcount - 1])
    words[wordcount++] = strchr(words[wordcount - 1] + 1, ',');

  if (wordcount > 5) {
    strcpy(p_err, ERR_TOO_MANY_OPERANDS);
    return 0;
  }
  wordcount--;

  // セパレータを'\0'化除外
  for (int i = 1; i < wordcount; i++) {
    words[i][0] = '\0';
    words[i]++;
  }
  for (int i = 0; i < wordcount; i++) ds_util::strtrim(words[i]);

  for (int i = 1; i < wordcount; i++) {
    if (words[i][0] == '\0') {
      strcpy(p_err, ERR_EMPTY_ARGUMENT);
      return 0;
    }
  }

  for (int i = 0; table[i].func; i++) {
    if (strcmp(words[0], table[i].label) != 0) {
      // エラーメッセージがある場合は一度ラベルが一致しているが引数が不一致
      // 同名のラベルは隣接しているので
      // ラベルが一致しなくなった時点でエラーとする
      if (p_err[0] != '\0') return 0;
      else continue;
    }
    if (wordcount - 1 < table[i].argc) {
      strcpy(p_err, ERR_TOO_FEW_OPERANDS);
      continue;
    }
    if (wordcount - 1 > table[i].argc) {
      strcpy(p_err, ERR_TOO_MANY_OPERANDS);
      continue;
    }
    p_err[0] = '\0';
    return table[i].func(&table[i], p_va, p_err, words[1], words[2], words[3]);
  }
  if (p_err[0] == '\0') strcpy(p_err, ERR_UNKNOWN_INSTRACTION);
  return 0;
}
