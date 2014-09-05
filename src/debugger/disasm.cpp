//-----------------------------------------------------------------------------
// include
#include "main.h"
#include "memory/memory.h"
#include "debugger/disasm.h"
#include "debugger/debugger.h"
#include "debugger/analyze_info.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

//-----------------------------------------------------------------------------
// global
char _gpr[32][12] = {
  "@dzero@w", "@Gat@w", "@Gv0@w", "@Gv1@w",
  "@Ga0@w",   "@Ga1@w", "@Ga2@w", "@Ga3@w",
  "@Gt0@w",   "@Gt1@w", "@Gt2@w", "@Gt3@w",
  "@Gt4@w",   "@Gt5@w", "@Gt6@w", "@Gt7@w",
  "@Gs0@w",   "@Gs1@w", "@Gs2@w", "@Gs3@w",
  "@Gs4@w",   "@Gs5@w", "@Gs6@w", "@Gs7@w",
  "@Gt8@w",   "@Gt9@w", "@Gk0@w", "@Gk1@w",
  "@Ggp@w",   "@Gsp@w", "@Gfp@w", "@Rra@w",
};

char _gprp[32][12] = {
  "zero", "at", "v0", "v1", "a0", "a1", "a2", "a3",
  "t0",   "t1", "t2", "t3", "t4", "t5", "t6", "t7",
  "s0",   "s1", "s2", "s3", "s4", "s5", "s6", "s7",
  "t8",   "t9", "k0", "k1", "gp", "sp", "fp", "ra",
};

char _cpr[32][10] = {
  "index",    "random",   "entrylo0", "entrylo1",
  "context",  "pagemask", "wired",    "(n/a)",
  "badvaddr", "count",    "entryhi",  "compare",
  "status",   "cause",    "epc",      "prid",
  "config",   "(n/a)",    "(n/a)",    "(n/a)",
  "(n/a)",    "(n/a)",    "(n/a)",    "badpaddr",
  "(debug)",  "(perf)",   "(n/a)",    "(n/a)",
  "taglo",    "taghi",    "errorepc", "(n/a)"
};

char _syscallTable[256][32] = {
// 0x00-
  "RFU000_FullReset",            "ResetEE",
  "SetGsCrt",                    "RFU003",
  "Exit",                        "RFU005",
  "LoadPS2Exe",                  "ExecPS2",
  "RFU008",                      "RFU009",
  "AddSbusIntcHandler",          "RemoveSbusIntcHandler",
  "Interrupt2Iop",               "SetVTLBRefillHandler",
  "SetVCommonHandler",           "SetVInterruptHandler",
// 0x10-
  "AddIntcHandler",              "RemoveIntcHandler",
  "AddDmacHandler",              "RemoveDmacHandler",
  "_EnableIntc",                 "_DisableIntc",
  "_EnableDmac",                 "_DisableDmac",
  "_SetAlarm",                   "_ReleaseAlarm",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
// 0x20-
  "CreateThread",                "DeleteThread",
  "StartThread",                 "ExitThread",
  "ExitDeleteThread",            "TerminateThread",
  "(null)",                      "DisableDispatchThread",
  "EnableDispatchThread",        "ChangeThreadPriority",
  "null",                        "RotateThreadReadyQueue",
  "(null)",                      "ReleaseWaitThread",
  "(null)",                      "GetThreadId",
// 0x30-
  "ReferThreadStatus",           "(null)",
  "SleepThread",                 "WakeupThread",
  "(null)",                      "CancelWakeupThread",
  "(null)",                      "SuspendThread",
  "(null)",                      "ResumeThread",
  "(null)",                      "JoinThread",
  "RFU060",                      "RFU061",
  "EndOfHeap",                   "RFU063",
// 0x40-
  "CreateSema",                  "DeleteSema",
  "SignalSema",                  "(null)",
  "WaitSema",                    "PollSema",
  "(null)",                      "ReferSemaStatus",
  "(null)",                      "RFU073",
  "SetOsdConfigParam",           "GetOsdConfigParam",
  "GetGsHParam",                 "GetGsVParam",
  "SetGsHParam",                 "SetGsVParam",
// 0x50-
  "RFU080_CreateEventFlag",      "RFU081_DeleteEventFlag",
  "RFU082_SetEventFlag",         "(null)",
  "RFU084_ClearEventFlag",       "(null)",
  "RFU086_WaitEventFlag",        "RFU087_PollEventFlag",
  "(null)",                      "RFU089_ReferEventFlagStatus",
  "(null)",                      "RFU091",
  "EnableIntcHandler",           "DisableIntcHandler",
  "EnableDmacHandler",           "DisableDmacHandler",
// 0x60-
  "KSeg0",                       "EnableCache",
  "DisableCache",                "GetCop0",
  "FlushCache",                  "(null)",
  "CpuConfig",                   "(null)",            
  "(null)",                      "(null)",
  "(null)",                      "sceSifStopDma",        
  "SetCPUTimerHandler",          "SetCPUTimer",
  "ForceRead",                   "ForceWrite",
// 0x70-
  "GsGetIMR",                    "GsPutIMR",
  "SetPgifHandler",              "SetVSyncFlag",          
  "RFU116",                      "print",
  "sceSifDmaStat",               "sceSifSetDma",
  "sceSifSetDChain",             "sceSifSetReg",
  "sceSifGetReg",                "ExecOSD",
  "Deci2Call",                   "PSMode",
  "MachineType",                 "GetMemorySize",
// 0x80-
  "(null)",                      "(null)",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
  "isceSifSetDChain",            "isceSifSetDma",
  "isceSifDmaStat",              "(null)",
  "(null)",                      "(null)",
  "(null)",                      "iGsPutIMR",
// 0x90-
  "iGsGetIMR",                   "(null)",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
  "iCpuConfig",                  "(null)",
  "iFlushCache",                 "iGetCop0",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
  "(null)",                      "(null)",
// 0xa0-
  "(null)",                      "iDisableDmacHandler",
  "iEnableDmacHandler",          "iDisableIntcHandler",
  "iEnableIntcHandler",           "(null)",
  "RFU090_iReferEventFlagStatus", "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
// 0xb0-
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "iReferSemaStatus",             "(null)",
  "iPollSema",                    "(null)",
  "(null)",                       "iSignalSema",
  "(null)",                       "(null)",
// 0xc0-
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "iResumeThread",                "(null)",
  "iSuspendThread",               "(null)",
  "iCancelWakeupThread",          "(null)",
  "iWakeupThread",                "(null)",
  "(null)",                       "iReferThreadStatus",
// 0xd0-
  "(null)",                       "(null)",
  "iReleaseWaitThread",           "(null)",
  "iRotateThreadReadyQueue",      "(null)",
  "iChangeThreadPriority",        "(null)",
  "(null)",                       "(null)",
  "iTerminateThread",             "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
// 0xe0-
  "(null)",                       "_iReleaseAlarm",
  "_iSetAlarm",                   "_iDisableDmac",
  "_iEnableDmac",                 "_iDisableIntc",
  "_iEnableIntc",                 "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
// 0xf0-
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
  "(null)",                       "(null)",
};

//-----------------------------------------------------------------------------
// function
void invalid(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "invalid");
}
void _invalid(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "invalid");
}

//----------------------------------------------- special
#define _rs    ((u8)((p_opcode >> 21) & 0x1f))
#define _rt    ((u8)((p_opcode >> 16) & 0x1f))
#define _rd    ((u8)((p_opcode >> 11) & 0x1f))
#define _sa    ((u8)((p_opcode >>  6) & 0x1f))

#define _code  ((p_opcode >> 6) & 0xfffff)
#define _stype ((u8)((p_opcode >> 6) & 0x1f))

void sll(u32 p_addr, u32 p_opcode, char* p_str)   {
  if (p_opcode == 0x00000000) {
    sprintf(p_str, "@dnop");
  } else {
    sprintf(p_str, "sll %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
  }
}
void srl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "srl %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void sra(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sra %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void sllv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sllv %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_rs]);
}
void srlv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "srlv %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_rs]);
}
void srav(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "srav %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_rs]);
}
void _sll(u32 p_addr, u32 p_opcode, char* p_str) {
  if (p_opcode == 0x00000000) {
    sprintf(p_str, "nop");
  } else {
    sprintf(p_str, "sll %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
  }
}
void _srl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "srl %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _sra(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sra %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _sllv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sllv %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_rs]);
}
void _srlv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "srlv %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_rs]);
}
void _srav(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "srav %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_rs]);
}

void jr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@rjr@w %s", _gpr[_rs]);
}
void jalr(u32 p_addr, u32 p_opcode, char* p_str) {
  if (_rd == 31) {
    sprintf(p_str, "@rjalr@w %s", _gpr[_rs]);
  } else {
    sprintf(p_str, "@rjalr@w %s, %s", _gpr[_rd], _gpr[_rs]);
  }
}
void movz(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "movz %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void movn(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "movn %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _jr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "jr %s", _gprp[_rs]);
}
void _jalr(u32 p_addr, u32 p_opcode, char* p_str) {
  if (_rd == 31) {
    sprintf(p_str, "jalr %s", _gprp[_rs]);
  } else {
    sprintf(p_str, "jalr %s, %s", _gprp[_rd], _gprp[_rs]);
  }
}
void _movz(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "movz %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _movn(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "movn %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void syscall(u32 p_addr, u32 p_opcode, char* p_str) {
  char* scname = NULL;
  u32 sccode = *((u32*)VA2PA(p_addr - 4));
  if ((sccode & 0xffff0000) == 0x24030000)
    scname = _syscallTable[sccode & 0xff];
  sprintf(p_str, "@rsyscall@w 0x%05x [%s]", _code, scname ? scname : "???");
}
void _syscall(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "syscall 0x%05x", _code);
}
void brk(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@rbreak@w 0x%05x", _code);
}
void sync(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@rsync%s", (_stype & 0x10) ? ".p" : "");
}
void _brk(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "break 0x%05x", _code);
}
void _sync(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sync%s", (_stype & 0x10) ? ".p" : "");
}
void mfhi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mfhi %s", _gpr[_rd]);
}
void mthi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mthi %s", _gpr[_rs]);
}
void mflo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mflo %s", _gpr[_rd]);
}
void mtlo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtlo %s", _gpr[_rs]);
}
void dsllv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsllv %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_rs]);
}
void dsrlv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrlv %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_rs]);
}
void dsrav(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrav %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_rs]);
}
void _mfhi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mfhi %s", _gprp[_rd]);
}
void _mthi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mthi %s", _gprp[_rs]);
}
void _mflo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mflo %s", _gprp[_rd]);
}
void _mtlo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtlo %s", _gprp[_rs]);
}
void _dsllv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsllv %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_rs]);
}
void _dsrlv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrlv %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_rs]);
}
void _dsrav(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrav %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_rs]);
}
void mult(u32 p_addr, u32 p_opcode, char* p_str)  {
  if (_rd == 0) {
    sprintf(p_str, "mult %s, %s", _gpr[_rs], _gpr[_rt]);
  } else {
    sprintf(p_str, "mult %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
  }
}
void multu(u32 p_addr, u32 p_opcode, char* p_str)  {
  if (_rd == 0) {
    sprintf(p_str, "multu %s, %s", _gpr[_rs], _gpr[_rt]);
  } else {
    sprintf(p_str, "multu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
  }
}
void div(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "div %s, %s", _gpr[_rs], _gpr[_rt]);
}
void divu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "divu %s, %s", _gpr[_rs], _gpr[_rt]);
}
void _mult(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mult %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _multu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "multu %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _div(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "div %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _divu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "divu %s, %s", _gprp[_rs], _gprp[_rt]);
}
void add(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "add %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void addu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "addu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void sub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sub %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void subu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "subu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void and(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "and %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void or(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "or %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void xor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "xor %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void nor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "nor %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _add(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "add %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _addu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "addu %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _sub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sub %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _subu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "subu %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _and(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "and %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _or(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "or %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _xor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "xor %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _nor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "nor %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void mfsa(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mfsa %s", _gpr[_rd]);
}
void mtsa(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtsa %s", _gpr[_rs]);
}
void slt(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "slt %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void sltu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sltu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void dadd(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dadd %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void daddu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "daddu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void dsub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsub %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void dsubu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsubu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _mfsa(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mfsa %s", _gprp[_rd]);
}
void _mtsa(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtsa %s", _gprp[_rs]);
}
void _slt(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "slt %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _sltu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sltu %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _dadd(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dadd %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _daddu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "daddu %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _dsub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsub %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _dsubu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsubu %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void tge(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tge %s, %s", _gpr[_rs], _gpr[_rt]);
}
void tgeu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tgeu %s, %s", _gpr[_rs], _gpr[_rt]);
}
void tlt(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tlt %s, %s", _gpr[_rs], _gpr[_rt]);
}
void tltu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tltu %s, %s", _gpr[_rs], _gpr[_rt]);
}
void teq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "teq %s, %s", _gpr[_rs], _gpr[_rt]);
}
void tne(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tne %s, %s", _gpr[_rs], _gpr[_rt]);
}
void _tge(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tge %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _tgeu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tgeu %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _tlt(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tlt %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _tltu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tltu %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _teq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "teq %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _tne(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tne %s, %s", _gprp[_rs], _gprp[_rt]);
}
void dsll(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsll %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void dsrl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrl %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void dsra(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsra %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void dsll32(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsll32 %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void dsrl32(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrl32 %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void dsra32(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsra32 %s, %s, %d", _gpr[_rd], _gpr[_rt], _sa);
}
void _dsll(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsll %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _dsrl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrl %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _dsra(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsra %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _dsll32(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsll32 %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _dsrl32(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsrl32 %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}
void _dsra32(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "dsra32 %s, %s, %d", _gprp[_rd], _gprp[_rt], _sa);
}

#undef _rs
#undef _rt
#undef _rd
#undef _sa

#undef _code
#undef _stype

//----------------------------------------------- regimm
#define _rs    ((u8)((p_opcode >> 21) & 0x1f))
#define _ofs16 ((s16)(p_opcode & 0xffff))
#define _imm   ((u16)(p_opcode & 0xffff))

void bltz(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbltz@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bgez(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbgez@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bltzl(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbltzl@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bgezl(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbgezl@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void _bltz(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bltz %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bgez(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bgez %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bltzl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bltzl %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bgezl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bgezl %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void tgei(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tgei %s, 0x%04x", _gpr[_rs], _imm);
}
void tgeiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tgeiu %s, 0x%04x", _gpr[_rs], _imm);
}
void tlti(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tlti %s, 0x%04x", _gpr[_rs], _imm);
}
void tltiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tltiu %s, 0x%04x", _gpr[_rs], _imm);
}
void teqi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "teqi %s, 0x%04x", _gpr[_rs], _imm);
}
void tnei(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tnei %s, 0x%04x", _gpr[_rs], _imm);
}
void _tgei(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tgei %s, 0x%04x", _gprp[_rs], _imm);
}
void _tgeiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tgeiu %s, 0x%04x", _gprp[_rs], _imm);
}
void _tlti(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tlti %s, 0x%04x", _gprp[_rs], _imm);
}
void _tltiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tltiu %s, 0x%04x", _gprp[_rs], _imm);
}
void _teqi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "teqi %s, 0x%04x", _gprp[_rs], _imm);
}
void _tnei(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "tnei %s, 0x%04x", _gprp[_rs], _imm);
}
void bltzal(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@rbltzal@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bgezal(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@rbgezal@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bltzall(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@rbltzall@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bgezall(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@rbgezall@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void _bltzal(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bltzal %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bgezal(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bgezal %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bltzall(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bltzall %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bgezall(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bgezall %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void mtsab(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtsab %s, 0x%04x", _gpr[_rs], _imm);
}
void mtsah(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtsah %s, 0x%04x", _gpr[_rs], _imm);
}
void _mtsab(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtsab %s, 0x%04x", _gprp[_rs], _imm);
}
void _mtsah(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtsah %s, 0x%04x", _gprp[_rs], _imm);
}

#undef _rs
#undef _ofs16
#undef _imm

//----------------------------------------------- cop0
#define _rs    ((u8)((p_opcode >> 21) & 0x1f))
#define _rt    ((u8)((p_opcode >> 16) & 0x1f))
#define _rd    ((u8)((p_opcode >> 11) & 0x1f))
#define _ofs16 ((s16)(p_opcode & 0xffff))
#define _func  ((u8)(p_opcode & 0x3f))

void cop0(u32 p_addr, u32 p_opcode, char* p_str) {
  switch (_rs) {
    case 0x00: // mf0
      switch (_rd) {
        case 0x18: { // debug register
          static const char table[8][16] = {
            "mfbpc", "(null)", "mfiab", "mfiabm",
            "mfdab", "mfdabm", "mfdvb", "mfdvbm"
          };
          sprintf(p_str, "%s %s", table[p_opcode & 0x7], _gpr[_rt]);
        } break;
        case 0x19: // performance counter register
          if (p_opcode & 1) {
            sprintf(p_str, "mfpc %s, %d", _gpr[_rt], (p_opcode >> 1) & 1);
          } else {
            sprintf(p_str, "mfps %s, 0", _gpr[_rt]);
          }
          break;
        default:
          sprintf(p_str, "mfc0 %s, %s", _gpr[_rt], _cpr[_rd]);
          break;
      }
      break;
    case 0x04: // mt0
      switch (_rd) {
        case 0x18: { // debug register
          char table[8][16] = {
            "mtbpc", "(null)", "mtiab", "mtiabm",
            "mtdab", "mtdabm", "mtdvb", "mtdvbm"
          };
          sprintf(p_str, "%s %s", table[p_opcode & 0x7], _gpr[_rt]);
        } break;
        case 0x19: // performance counter register
          if (p_opcode & 1) {
            sprintf(p_str, "mtpc %s, %d", _gpr[_rt], (p_opcode >> 1) & 1);
          } else {
            sprintf(p_str, "mtps %s, 0", _gpr[_rt]);
          }
          break;
        default:
          sprintf(p_str, "mtc0 %s, %s", _gpr[_rt], _cpr[_rd]);
          break;
      }
      break;
    case 0x08: { // bc0
      s16 ofs = _ofs16 + 1;
      switch (_rt) {
        case 0x00:
          sprintf(p_str, "@gbc0f@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        case 0x01:
          sprintf(p_str, "@gbc0t@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        case 0x02:
          sprintf(p_str, "@gbc0fl@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        case 0x03:
          sprintf(p_str, "@gbc0tl@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        default: invalid(p_addr, p_opcode, p_str); break;
      }
    } break;
    case 0x10: // c0
      switch (_func) {
        case 0x01: sprintf(p_str, "@rtlbr"); break;
        case 0x02: sprintf(p_str, "@rtlbwi"); break;
        case 0x06: sprintf(p_str, "@rtlbwr"); break;
        case 0x08: sprintf(p_str, "@rtlbp"); break;
        case 0x18: sprintf(p_str, "@reret"); break;
        case 0x38: sprintf(p_str, "@rei"); break;
        case 0x39: sprintf(p_str, "@rdi"); break;
        default:   invalid(p_addr, p_opcode, p_str); break;
      }
      break;
  }
}

void _cop0(u32 p_addr, u32 p_opcode, char* p_str) {
  switch (_rs) {
  case 0x00: // mf0
    switch (_rd) {
      case 0x18: { // debug register
        char table[8][16] = {
          "mfbpc", "(null)", "mfiab", "mfiabm",
          "mfdab", "mfdabm", "mfdvb", "mfdvbm"
        };
        sprintf(p_str, "%s %s", table[p_opcode & 0x7], _gprp[_rt]);
      } break;
      case 0x19:  // performance counter register
        if (p_opcode & 1) {
          sprintf(p_str, "mfpc %s, %d", _gprp[_rt], (p_opcode >> 1) & 1);
        } else {
          sprintf(p_str, "mfps %s, 0", _gprp[_rt]);
        }
        break;
      default:
        sprintf(p_str, "mfc0 %s, %s", _gprp[_rt], _cpr[_rd]);
        break;
    }
    break;
  case 0x04: // mt0
    switch (_rd) {
      case 0x18: {// debug register
        char table[8][16] = {
          "mtbpc", "(null)", "mtiab", "mtiabm",
          "mtdab", "mtdabm", "mtdvb", "mtdvbm"
        };
        sprintf(p_str, "%s %s", table[p_opcode & 0x7], _gprp[_rt]);
      } break;
      case 0x19:  // performance counter register
        if (p_opcode & 1) {
          sprintf(p_str, "mtpc %s, %d", _gprp[_rt], (p_opcode >> 1) & 1);
        } else {
          sprintf(p_str, "mtps %s, 0", _gprp[_rt]);
        }
        break;
      default:
        sprintf(p_str, "mtc0 %s, %s", _gprp[_rt], _cpr[_rd]);
        break;
    }
    break;
  case 0x08:  // bc0
    switch (_rt) {
      case 0x00:
        sprintf(p_str, "bc0f 0x%08x", p_addr + 4 + _ofs16 * 4);
        break;
      case 0x01:
        sprintf(p_str, "bc0t 0x%08x", p_addr + 4 + _ofs16 * 4);
        break;
      case 0x02:
        sprintf(p_str, "bc0fl 0x%08x", p_addr + 4 + _ofs16 * 4);
        break;
      case 0x03:
        sprintf(p_str, "bc0tl 0x%08x", p_addr + 4 + _ofs16 * 4);
        break;
      default:
        _invalid(p_addr, p_opcode, p_str);
        break;
    }
    break;
  case 0x10:  // c0
    switch (_func) {
      case 0x01: sprintf(p_str, "tlbr"); break;
      case 0x02: sprintf(p_str, "tlbwi"); break;
      case 0x06: sprintf(p_str, "tlbwr"); break;
      case 0x08: sprintf(p_str, "tlbp"); break;
      case 0x18: sprintf(p_str, "eret"); break;
      case 0x38: sprintf(p_str, "ei"); break;
      case 0x39: sprintf(p_str, "di"); break;
      default:   _invalid(p_addr, p_opcode, p_str); break;
    }
    break;
  }
}

#undef _rs
#undef _rt
#undef _rd
#undef _ofs16
#undef _func

//----------------------------------------------- cop1
#define _rs    ((u8)((p_opcode >> 21) & 0x1f))
#define _rt    ((u8)((p_opcode >> 16) & 0x1f))
#define _ft    ((u8)((p_opcode >> 16) & 0x1f))
#define _fs    ((u8)((p_opcode >> 11) & 0x1f))
#define _fd    ((u8)((p_opcode >>  6) & 0x1f))
#define _ofs16 ((s16)(p_opcode & 0xffff))
#define _func  ((u8)(p_opcode & 0x3f))

void cop1(u32 p_addr, u32 p_opcode, char* p_str) {
  switch (_rs) {
    case 0x00:
      sprintf(p_str, "mfc1 %s, fpr%d", _gpr[_rt], _fs);
      break;
    case 0x02:
      sprintf(p_str, "cfc1 %s, fpr%d", _gpr[_rt], _fs);
      break;
    case 0x04:
      sprintf(p_str, "mtc1 %s, fpr%d", _gpr[_rt], _fs);
      break;
    case 0x06:
      sprintf(p_str, "ctc1 %s, fpr%d", _gpr[_rt], _fs);
      break;
    case 0x08: { // bc1
      s16 ofs = _ofs16 + 1;
      switch (_rt) {
        case 0x00:
          sprintf(p_str, "@gbc1f@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        case 0x01:
          sprintf(p_str, "@gbc1t@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        case 0x02:
          sprintf(p_str, "@gbc1fl@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        case 0x03:
          sprintf(p_str, "@gbc1tl@w 0x%08x(@c%s%d@w)",
            p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
          break;
        default:
          invalid(p_addr, p_opcode, p_str);
          break;
      }
      break; }
    case 0x10:  // s
      switch (_func) {
        case 0x00:
          sprintf(p_str, "add.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x01:
          sprintf(p_str, "sub.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x02:
          sprintf(p_str, "mul.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x03:
          sprintf(p_str, "div.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x04:
          sprintf(p_str, "sqrt.s fpr%d, fpr%d", _fd, _ft);
          break;
        case 0x05:
          sprintf(p_str, "abs.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x06:
          sprintf(p_str, "mov.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x07:
          sprintf(p_str, "neg.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x16:
          sprintf(p_str, "rsqrt.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x18:
          sprintf(p_str, "adda.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x19:
          sprintf(p_str, "suba.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x1a:
          sprintf(p_str, "mula.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x1c:
          sprintf(p_str, "madd.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x1d:
          sprintf(p_str, "msub.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x1e:
          sprintf(p_str, "madda.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x1f:
          sprintf(p_str, "msuba.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x24:
          sprintf(p_str, "cvt.w.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x28:
          sprintf(p_str, "max.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x29:
          sprintf(p_str, "min.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x30:
          sprintf(p_str, "c.f.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x32:
          sprintf(p_str, "c.eq.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x34:
          sprintf(p_str, "c.lt.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x36:
          sprintf(p_str, "c.le.s fpr%d, fpr%d", _fs, _ft);
          break;
        default:
          invalid(p_addr, p_opcode, p_str);
          break;
      }
      break;
    case 0x14: // w
      if (_func == 0x20) {
        sprintf(p_str, "cvt.s.w fpr%d, fpr%d", _fd, _fs);
      } else {
        invalid(p_addr, p_opcode, p_str);
      }
      break;
  }
}

void _cop1(u32 p_addr, u32 p_opcode, char* p_str) {
  switch (_rs) {
    case 0x00:
      sprintf(p_str, "mfc1 %s, fpr%d", _gprp[_rt], _fs);
      break;
    case 0x02:
      sprintf(p_str, "cfc1 %s, fpr%d", _gprp[_rt], _fs);
      break;
    case 0x04:
      sprintf(p_str, "mtc1 %s, fpr%d", _gprp[_rt], _fs);
      break;
    case 0x06:
      sprintf(p_str, "ctc1 %s, fpr%d", _gprp[_rt], _fs);
      break;
    case 0x08: // bc1
      switch (_rt) {
        case 0x00: sprintf(p_str, "bc1f 0x%08x", p_addr + 4 + _ofs16 * 4);
          break;
        case 0x01: sprintf(p_str, "bc1t 0x%08x", p_addr + 4 + _ofs16 * 4);
          break;
        case 0x02: sprintf(p_str, "bc1fl 0x%08x", p_addr + 4 + _ofs16 * 4);
          break;
        case 0x03: sprintf(p_str, "bc1tl 0x%08x", p_addr + 4 + _ofs16 * 4);
          break;
        default:
          _invalid(p_addr, p_opcode, p_str);
          break;
      }
      break;
    case 0x10: // s
      switch (_func) {
        case 0x00:
          sprintf(p_str, "add.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x01:
          sprintf(p_str, "sub.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x02:
          sprintf(p_str, "mul.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x03:
          sprintf(p_str, "div.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x04:
          sprintf(p_str, "sqrt.s fpr%d, fpr%d", _fd, _ft);
          break;
        case 0x05:
          sprintf(p_str, "abs.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x06:
          sprintf(p_str, "mov.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x07:
          sprintf(p_str, "neg.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x16:
          sprintf(p_str, "rsqrt.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x18:
          sprintf(p_str, "adda.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x19:
          sprintf(p_str, "suba.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x1a:
          sprintf(p_str, "mula.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x1c:
          sprintf(p_str, "madd.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x1d:
          sprintf(p_str, "msub.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x1e:
          sprintf(p_str, "madda.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x1f:
          sprintf(p_str, "msuba.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x24:
          sprintf(p_str, "cvt.w.s fpr%d, fpr%d", _fd, _fs);
          break;
        case 0x28:
          sprintf(p_str, "max.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x29:
          sprintf(p_str, "min.s fpr%d, fpr%d, fpr%d", _fd, _fs, _ft);
          break;
        case 0x30:
          sprintf(p_str, "c.f.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x32:
          sprintf(p_str, "c.eq.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x34:
          sprintf(p_str, "c.lt.s fpr%d, fpr%d", _fs, _ft);
          break;
        case 0x36:
          sprintf(p_str, "c.le.s fpr%d, fpr%d", _fs, _ft);
          break;
        default:
          _invalid(p_addr, p_opcode, p_str);
          break;
      }
      break;
    case 0x14: // w
      if (_func == 0x20) {
        sprintf(p_str, "cvt.s.w fpr%d, fpr%d", _fd, _fs);
      } else {
        _invalid(p_addr, p_opcode, p_str);
      }
      break;
  }
}

#undef _rs
#undef _rt
#undef _ft
#undef _fs
#undef _fd
#undef _ofs16
#undef _func

//----------------------------------------------- cop2
void cop2(u32 p_addr, u32 p_opcode, char* p_str)  { sprintf(p_str, "cop2"); }
void _cop2(u32 p_addr, u32 p_opcode, char* p_str) { sprintf(p_str, "cop2"); }

//----------------------------------------------- mmi
#define _rs ((u8)((p_opcode >> 21) & 0x1f))
#define _rt ((u8)((p_opcode >> 16) & 0x1f))
#define _rd ((u8)((p_opcode >> 11) & 0x1f))
#define _sa ((u8)((p_opcode >>  6) & 0x1f))

void madd(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "madd %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void maddu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "maddu %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void plzcw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "plzcw %s, %s", _gpr[_rd], _gpr[_rs]);
}
void _madd(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "madd %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _maddu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "maddu %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _plzcw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "plzcw %s, %s", _gprp[_rd], _gprp[_rs]);
}
void mfhi1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mfhi1 %s", _gpr[_rd]);
}
void mthi1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mthi1 %s", _gpr[_rs]);
}
void mflo1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mflo1 %s", _gpr[_rd]);
}
void mtlo1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtlo1 %s", _gpr[_rs]);
}
void _mfhi1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mfhi1 %s", _gprp[_rd]);
}
void _mthi1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mthi1 %s", _gprp[_rs]);
}
void _mflo1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mflo1 %s", _gprp[_rd]);
}
void _mtlo1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mtlo1 %s", _gprp[_rs]);
}
void mult1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mult1 %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void multu1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "multu1 %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void div1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "div1 %s, %s", _gpr[_rs], _gpr[_rt]);
}
void divu1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "divu1 %s, %s", _gpr[_rs], _gpr[_rt]);
}
void _mult1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "mult1 %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _multu1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "multu1 %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _div1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "div1 %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _divu1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "divu1 %s, %s", _gprp[_rs], _gprp[_rt]);
}
void madd1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "madd1 %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void maddu1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "maddu1 %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _madd1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "madd1 %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _maddu1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "maddu1 %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmfhl(u32 p_addr, u32 p_opcode, char* p_str) {
  switch ((p_opcode >> 6) & 0x1f) {
    case 0: sprintf(p_str, "pmfhl.lw %s", _gpr[_rd]); break;
    case 1: sprintf(p_str, "pmfhl.uw %s", _gpr[_rd]); break;
    case 2: sprintf(p_str, "pmfhl.slw %s", _gpr[_rd]); break;
    case 3: sprintf(p_str, "pmfhl.lh %s", _gpr[_rd]); break;
    case 4: sprintf(p_str, "pmfhl.sh %s", _gpr[_rd]); break;
    default: invalid(p_addr, p_opcode, p_str); break;
  }
}
void _pmfhl(u32 p_addr, u32 p_opcode, char* p_str) {
  switch ((p_opcode >> 6) & 0x1f) {
    case 0: sprintf(p_str, "pmfhl.lw %s", _gprp[_rd]); break;
    case 1: sprintf(p_str, "pmfhl.uw %s", _gprp[_rd]); break;
    case 2: sprintf(p_str, "pmfhl.slw %s", _gprp[_rd]); break;
    case 3: sprintf(p_str, "pmfhl.lh %s", _gprp[_rd]); break;
    case 4: sprintf(p_str, "pmfhl.sh %s", _gprp[_rd]); break;
    default: _invalid(p_addr, p_opcode, p_str); break;
  }
}
void pmthl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmthl.lw %s", _gpr[_rs]); }
void psllh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psllh %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_sa]);
}
void psrlh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psrlh %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_sa]);
}
void psrah(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psrah %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_sa]);
}
void _pmthl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmthl.lw %s", _gprp[_rs]);
}
void _psllh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psllh %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_sa]);
}
void _psrlh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psrlh %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_sa]);
}
void _psrah(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psrah %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_sa]);
}
void psllw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psllw %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_sa]);
}
void psrlw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psrlw %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_sa]);
}
void psraw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psraw %s, %s, %s", _gpr[_rd], _gpr[_rt], _gpr[_sa]);
}
void _psllw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psllw %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_sa]);
}
void _psrlw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psrlw %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_sa]);
}
void _psraw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psraw %s, %s, %s", _gprp[_rd], _gprp[_rt], _gprp[_sa]);
}

//----------------------------------------------- mmi0
void paddw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pcgtw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcgtw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pmaxw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaxw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pcgtw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcgtw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pmaxw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaxw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void paddh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pcgth(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcgth %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pmaxh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaxh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pcgth(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcgth %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pmaxh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaxh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void paddb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddb %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubb %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pcgtb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcgtb %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddb %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubb %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pcgtb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcgtb %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void paddsw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddsw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubsw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubsw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pextlw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextlw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void ppacw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppacw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddsw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddsw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubsw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubsw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pextlw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextlw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _ppacw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppacw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void paddsh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddsh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubsh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubsh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pextlh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextlh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void ppach(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppach %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddsh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddsh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubsh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubsh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pextlh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextlh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _ppach(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppach %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void paddsb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddsh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubsb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubsh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pextlb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextlh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void ppacb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppach %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddsb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddsh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubsb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubsh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pextlb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextlh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _ppacb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppach %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pext5(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pext5 %s, %s", _gpr[_rd], _gpr[_rt]);
}
void ppac5(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppac5 %s, %s", _gpr[_rd], _gpr[_rt]);
}
void _pext5(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pext5 %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _ppac5(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ppac5 %s, %s", _gprp[_rd], _gprp[_rt]);
}

void mmi0(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    paddw,   psubw,   pcgtw,   pmaxw,
    paddh,   psubh,   pcgth,   pmaxh,
    paddb,   psubb,   pcgtb,   invalid,
    invalid, invalid, invalid, invalid,
    paddsw,  psubsw,  pextlw,  ppacw,
    paddsh,  psubsh,  pextlh,  ppach,
    paddsb,  psubsb,  pextlb,  ppacb,
    invalid, invalid, pext5,   ppac5,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}
void _mmi0(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    _paddw,   _psubw,   _pcgtw,   _pmaxw,
    _paddh,   _psubh,   _pcgth,   _pmaxh,
    _paddb,   _psubb,   _pcgtb,   _invalid,
    _invalid, _invalid, _invalid, _invalid,
    _paddsw,  _psubsw,  _pextlw,  _ppacw,
    _paddsh,  _psubsh,  _pextlh,  _ppach,
    _paddsb,  _psubsb,  _pextlb,  _ppacb,
    _invalid, _invalid, _pext5,   _ppac5,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}

//----------------------------------------------- mmi1
void pabsw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pabsw %s, %s", _gpr[_rd], _gpr[_rt]);
}
void pceqw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pceqw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pminw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pminw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pabsw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pabsw %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _pceqw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pceqw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pminw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pminw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void padsbh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "padsbh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pabsh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pabsh %s, %s", _gpr[_rd], _gpr[_rt]);
}
void pceqh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pceqh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pminh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pminh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _padsbh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "padsbh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pabsh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pabsh %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _pceqh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pceqh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pminh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pminh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pceqb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pceqb %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pceqb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pceqb %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void padduw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "padduw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubuw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pextuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextuw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _padduw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "padduw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubuw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pextuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextuw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void padduh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "padduh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubuh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubuh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pextuh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextuh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _padduh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "padduh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubuh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubuh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pextuh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextuh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void paddub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddub %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psubub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubub %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pextub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextub %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _paddub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "paddub %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psubub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psubub %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pextub(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pextub %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void qfsrv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "qfsrv %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _qfsrv(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "qfsrv %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void mmi1(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    invalid, pabsw,   pceqw,   pminw,
    padsbh,  pabsh,   pceqh,   pminh,
    invalid, invalid, pceqb,   invalid,
    invalid, invalid, invalid, invalid,
    padduw,  psubuw,  pextuw,  invalid,
    padduh,  psubuh,  pextuh,  invalid,
    paddub,  psubub,  pextub,  qfsrv,
    invalid, invalid, invalid, invalid,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}
void _mmi1(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    _invalid, _pabsw,   _pceqw,   _pminw,
    _padsbh,  _pabsh,   _pceqh,   _pminh,
    _invalid, _invalid, _pceqb,   _invalid,
    _invalid, _invalid, _invalid, _invalid,
    _padduw,  _psubuw,  _pextuw,  _invalid,
    _padduh,  _psubuh,  _pextuh,  _invalid,
    _paddub,  _psubub,  _pextub,  _qfsrv,
    _invalid, _invalid, _invalid, _invalid,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}

//----------------------------------------------- mmi2
void pmaddw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psllvw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psrlvw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmaddw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psllvw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psrlvw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmsubw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmsubw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmfhi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmfhi %s", _gpr[_rd]);
}
void pmflo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmflo %s", _gpr[_rd]);
}
void pinth(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pinth %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmfhi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmfhi %s", _gprp[_rd]);
}
void _pmflo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmflo %s", _gprp[_rd]);
}
void _pinth(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pinth %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmultw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmultw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pdivw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pdivw %s, %s", _gpr[_rs], _gpr[_rt]);
}
void pcpyld(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcpyld %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmultw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmultw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pdivw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pdivw %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _pcpyld(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcpyld %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmaddh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void phmadh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "phmadh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pand(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pand %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pxor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pxor %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmaddh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmaddh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _phmadh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "phmadh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pand(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pand %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pxor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pxor %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmsubh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmsubh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void phmsbh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "phmsbh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmsubh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmsubh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _phmsbh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "phmsbh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pexeh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexeh %s, %s", _gpr[_rd], _gpr[_rt]);
}
void prevh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "prevh %s, %s", _gpr[_rd], _gpr[_rt]);
}
void _pexeh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexeh %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _prevh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "prevh %s, %s", _gprp[_rd], _gprp[_rt]);
}
void pmulth(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmulth %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pdivbw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pdivbw %s, %s", _gpr[_rs], _gpr[_rt]);
}
void pexew(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexew %s, %s", _gpr[_rd], _gpr[_rt]);
}
void prot3w(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "prot3w %s, %s", _gpr[_rd], _gpr[_rt]);
}
void _pmulth(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmulth %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pdivbw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pdivbw %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _pexew(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexew %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _prot3w(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "prot3w %s, %s", _gprp[_rd], _gprp[_rt]);
}
void mmi2(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    pmaddw,  invalid, psllvw,  psrlvw,
    pmsubw,  invalid, invalid, invalid,
    pmfhi,   pmflo,   pinth,   invalid,
    pmultw,  pdivw,   pcpyld,  invalid,
    pmaddh,  phmadh,  pand,    pxor,
    pmsubh,  phmsbh,  invalid, invalid,
    invalid, invalid, pexeh,   prevh,
    pmulth,  pdivbw,  pexew,   prot3w,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}
void _mmi2(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    _pmaddw,  _invalid, _psllvw,  _psrlvw,
    _pmsubw,  _invalid, _invalid, _invalid,
    _pmfhi,   _pmflo,   _pinth,   _invalid,
    _pmultw,  _pdivw,   _pcpyld,  _invalid,
    _pmaddh,  _phmadh,  _pand,    _pxor,
    _pmsubh,  _phmsbh,  _invalid, _invalid,
    _invalid, _invalid, _pexeh,   _prevh,
    _pmulth,  _pdivbw,  _pexew,   _prot3w,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}

//----------------------------------------------- mmi3
void pmadduw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmadduw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void psravw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psravw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmadduw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmadduw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _psravw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "psravw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmthi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmthi %s", _gpr[_rs]);
}
void pmtlo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmtlo %s", _gpr[_rs]);
}
void pinteh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pinteh %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmthi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmthi %s", _gprp[_rs]);
}
void _pmtlo(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmtlo %s", _gprp[_rs]);
}
void _pinteh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pinteh %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pmultuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmultuw %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]); }
void pdivuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pdivuw %s, %s", _gpr[_rs], _gpr[_rt]); }
void pcpyud(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcpyud %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _pmultuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pmultuw %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pdivuw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pdivuw %s, %s", _gprp[_rs], _gprp[_rt]);
}
void _pcpyud(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcpyud %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void por(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "por %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void pnor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pnor %s, %s, %s", _gpr[_rd], _gpr[_rs], _gpr[_rt]);
}
void _por(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "por %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void _pnor(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pnor %s, %s, %s", _gprp[_rd], _gprp[_rs], _gprp[_rt]);
}
void pexch(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexch %s, %s", _gpr[_rd], _gpr[_rt]);
}
void pcpyh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcpyh %s, %s", _gpr[_rd], _gpr[_rt]);
}
void pexcw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexcw %s, %s", _gpr[_rd], _gpr[_rt]);
}
void _pexch(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexch %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _pcpyh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pcpyh %s, %s", _gprp[_rd], _gprp[_rt]);
}
void _pexcw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pexcw %s, %s", _gprp[_rd], _gprp[_rt]);
}
void mmi3(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    pmadduw,  invalid,  invalid,  psravw,
    invalid,  invalid,  invalid,  invalid,
    pmthi,    pmtlo,    pinteh,    invalid,
    pmultuw,  pdivuw,    pcpyud,    invalid,
    invalid,  invalid,  por,    pnor,
    invalid,  invalid,  invalid,  invalid,
    invalid,  invalid,  pexch,    pcpyh,
    invalid,  invalid,  pexcw,    invalid,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}
void _mmi3(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    _pmadduw,  _invalid,  _invalid,  _psravw,
    _invalid,  _invalid,  _invalid,  _invalid,
    _pmthi,    _pmtlo,    _pinteh,  _invalid,
    _pmultuw,  _pdivuw,  _pcpyud,  _invalid,
    _invalid,  _invalid,  _por,    _pnor,
    _invalid,  _invalid,  _invalid,  _invalid,
    _invalid,  _invalid,  _pexch,    _pcpyh,
    _invalid,  _invalid,  _pexcw,    _invalid,
  };
  TABLE[(p_opcode >> 6) & 0x1f](p_addr, p_opcode, p_str);
}

#undef _rs
#undef _rt
#undef _rd
#undef _sa

//----------------------------------------------- misc
#define _rs    ((u8)((p_opcode >> 21) & 0x1f))
#define _rt    ((u8)((p_opcode >> 16) & 0x1f))
#define _rd    ((u8)((p_opcode >> 11) & 0x1f))
#define _imm   ((u16)(p_opcode & 0xffff))
#define _ofs16 ((s16)(p_opcode & 0xffff))
#define _ofs26 ((u32)(p_opcode & 0x03ffffff))
#define _base  ((u8)((p_opcode >> 21) & 0x1f))
#define _ft    ((u8)((p_opcode >> 16) & 0x1f))
#define _hint  ((u8)((p_opcode >> 16) & 0x1f))

void special(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x40])(u32, u32, char*) = {
    sll,  invalid, srl,  sra,  sllv,    invalid, srlv,    srav,
    jr,   jalr,    movz, movn, syscall, brk,     invalid, sync,
    mfhi, mthi,    mflo, mtlo, dsllv,   invalid, dsrlv,   dsrav,
    mult, multu,   div,  divu, invalid, invalid, invalid, invalid,
    add,  addu,    sub,  subu, and,     or,      xor,     nor,
    mfsa, mtsa,    slt,  sltu, dadd,    daddu,   dsub,    dsubu,
    tge,  tgeu,    tlt,  tltu, teq,     invalid, tne,     invalid,
    dsll, invalid, dsrl, dsra, dsll32,  invalid, dsrl32,  dsra32,
  };
  TABLE[p_opcode & 0x3f](p_addr, p_opcode, p_str);
}
void _special(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x40])(u32, u32, char*) = {
    _sll,  _invalid, _srl,  _sra,  _sllv,    _invalid, _srlv,    _srav,
    _jr,   _jalr,    _movz, _movn, _syscall, _brk,     _invalid, _sync,
    _mfhi, _mthi,    _mflo, _mtlo, _dsllv,   _invalid, _dsrlv,   _dsrav,
    _mult, _multu,   _div,  _divu, _invalid, _invalid, _invalid, _invalid,
    _add,  _addu,    _sub,  _subu, _and,     _or,      _xor,     _nor,
    _mfsa, _mtsa,    _slt,  _sltu, _dadd,    _daddu,   _dsub,    _dsubu,
    _tge,  _tgeu,    _tlt,  _tltu, _teq,     _invalid, _tne,     _invalid,
    _dsll, _invalid, _dsrl, _dsra, _dsll32,  _invalid, _dsrl32,  _dsra32,
  };
  TABLE[p_opcode & 0x3f](p_addr, p_opcode, p_str);
}
void regimm(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    bltz,   bgez,   bltzl,   bgezl,   invalid, invalid, invalid, invalid,
    tgei,   tgeiu,  tlti,    tltiu,   teqi,    invalid, tnei,    invalid,
    bltzal, bgezal, bltzall, bgezall, invalid, invalid, invalid, invalid,
    mtsab,  mtsah,  invalid, invalid, invalid, invalid, invalid, invalid,
  };
  TABLE[(p_opcode >> 16) & 0x1f](p_addr, p_opcode, p_str);
}
void _regimm(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x20])(u32, u32, char*) = {
    _bltz,    _bgez,    _bltzl,   _bgezl,
    _invalid, _invalid, _invalid, _invalid,
    _tgei,    _tgeiu,   _tlti,    _tltiu,
    _teqi,    _invalid, _tnei,    _invalid,
    _bltzal,  _bgezal,  _bltzall, _bgezall,
    _invalid, _invalid, _invalid, _invalid,
    _mtsab,   _mtsah,   _invalid, _invalid,
    _invalid, _invalid, _invalid, _invalid,
  };
  TABLE[(p_opcode >> 16) & 0x1f](p_addr, p_opcode, p_str);
}
void j(u32 p_addr, u32 p_opcode, char* p_str) {
  u32  jaddr = (p_addr & 0xf0000000) | _ofs26 << 2;
  char sign;
  u32  absdelta;
  
  if (jaddr < p_addr) {
    sign = '-';
    absdelta = p_addr - jaddr;
  } else {
    sign = '+';
    absdelta = jaddr - p_addr;
  }
  sprintf(p_str, "@gj@w 0x%08x(@c%c%d@w)", jaddr, sign, absdelta >> 2);
}
void jal(u32 p_addr, u32 p_opcode, char* p_str) {
  u32  jaddr = (p_addr & 0xf0000000) | _ofs26 << 2;
  char sign;
  u32  absdelta;
  
  if (jaddr < p_addr) {
    sign = '-';
    absdelta = p_addr - jaddr;
  } else {
    sign = '+';
    absdelta = jaddr - p_addr;
  }
  sprintf(p_str, "@rjal@w 0x%08x(@c%c%d@w)", jaddr, sign, absdelta >> 2);
}
void _j(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "j 0x%08x", (p_addr & 0xf0000000) | _ofs26 << 2);
}
void _jal(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "jal 0x%08x", (p_addr & 0xf0000000) | _ofs26 << 2);
}
void beq(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbeq@w %s, %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], _gpr[_rt], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bne(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbne@w %s, %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], _gpr[_rt], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void blez(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gblez@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bgtz(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16 + 1;
  sprintf(p_str, "@gbgtz@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void _beq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "beq %s, %s, 0x%08x",
    _gprp[_rs], _gprp[_rt], p_addr + 4 + _ofs16 * 4);
}
void _bne(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bne %s, %s, 0x%08x",
    _gprp[_rs], _gprp[_rt], p_addr + 4 + _ofs16 * 4);
}
void _blez(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "blez %s, 0x%08x",
    _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bgtz(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bgtz %s, 0x%08x",
    _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void addi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "addi %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void addiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "addiu %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void slti(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "slti %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void sltiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sltiu %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void andi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "andi %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void ori(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ori %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void xori(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "xori %s, %s 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void lui(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lui %s, 0x%04x", _gpr[_rt], _imm); }
void _addi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "addi %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _addiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "addiu %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _slti(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "slti %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _sltiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sltiu %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _andi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "andi %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _ori(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ori %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _xori(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "xori %s, %s 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _lui(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lui %s, 0x%04x", _gprp[_rt], _imm);
}
void beql(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16;
  sprintf(p_str, "@gbeql@w %s, %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], _gpr[_rt], p_addr + 4 + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bnel(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16;
  sprintf(p_str, "@gbnel@w %s, %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], _gpr[_rt], p_addr + 4 + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void blezl(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16;
  sprintf(p_str, "@gblezl@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + 4 + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void bgtzl(u32 p_addr, u32 p_opcode, char* p_str) {
  s16 ofs = _ofs16;
  sprintf(p_str, "@gbgtzl@w %s, 0x%08x(@c%s%d@w)",
    _gpr[_rs], p_addr + 4 + ofs * 4, ofs > 0 ? "+" : "", ofs);
}
void _beql(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "beql %s, %s, 0x%08x",
    _gprp[_rs], _gprp[_rt], p_addr + 4 + _ofs16 * 4);
}
void _bnel(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bnel %s, %s, 0x%08x",
    _gprp[_rs], _gprp[_rt], p_addr + 4 + _ofs16 * 4);
}
void _blezl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "blezl %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void _bgtzl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "bgtzl %s, 0x%08x", _gprp[_rs], p_addr + 4 + _ofs16 * 4);
}
void daddi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "daddi %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void daddiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "daddiu %s, %s, 0x%04x", _gpr[_rt], _gpr[_rs], _imm);
}
void ldl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@yldl@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void ldr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@yldr@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void _daddi(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "daddi %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _daddiu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "daddiu %s, %s, 0x%04x", _gprp[_rt], _gprp[_rs], _imm);
}
void _ldl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ldl %s, 0x%04x(%s)", _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _ldr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ldr %s, 0x%04x(%s)", _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void mmi(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x40])(u32, u32, char*) = {
    madd,    maddu,   invalid, invalid,
    plzcw,   invalid, invalid, invalid,
    mmi0,    mmi2,    invalid, invalid,
    invalid, invalid, invalid, invalid,
    mfhi1,   mthi1,   mflo1,   mtlo1,
    invalid, invalid, invalid, invalid,
    mult1,   multu1,  div1,    divu1,
    invalid, invalid, invalid, invalid,
    madd1,   maddu1,  invalid, invalid,
    invalid, invalid, invalid, invalid,
    mmi1,    mmi3,    invalid, invalid,
    invalid, invalid, invalid, invalid,
    pmfhl,   pmthl,   invalid, invalid,
    psllh,   invalid, psrlh,   psrah,
    invalid, invalid, invalid, invalid,
    psllw,   invalid, psrlw,   psraw,
  };
  TABLE[p_opcode & 0x3f](p_addr, p_opcode, p_str);
}
void _mmi(u32 p_addr, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x40])(u32, u32, char*) = {
    _madd,    _maddu,   _invalid, _invalid,
    _plzcw,   _invalid, _invalid, _invalid,
    _mmi0,    _mmi2,    _invalid, _invalid,
    _invalid, _invalid, _invalid, _invalid,
    _mfhi1,   _mthi1,   _mflo1,   _mtlo1,
    _invalid, _invalid, _invalid, _invalid,
    _mult1,   _multu1,  _div1,    _divu1,
    _invalid, _invalid, _invalid, _invalid,
    _madd1,   _maddu1,  _invalid, _invalid,
    _invalid, _invalid, _invalid, _invalid,
    _mmi1,    _mmi3,    _invalid, _invalid,
    _invalid, _invalid, _invalid, _invalid,
    _pmfhl,   _pmthl,   _invalid, _invalid,
    _psllh,   _invalid, _psrlh,   _psrah,
    _invalid, _invalid, _invalid, _invalid,
    _psllw,   _invalid, _psrlw,   _psraw,
  };
  TABLE[p_opcode & 0x3f](p_addr, p_opcode, p_str);
}
void lq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylq@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void sq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysq@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void _lq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lq %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _sq(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sq %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void lb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylb@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylh@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lwl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylwl@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylw@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lbu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylbu@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lhu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylhu@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lwr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylwr@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void lwu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylwu@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void _lb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lb %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lh %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lwl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lwl %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lw %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lbu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lbu %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lhu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lhu %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lwr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lwr %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _lwu(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lwu %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void sb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysb@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void sh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysh@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void swl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@yswl@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void sw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysw@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void sdl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysdl@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void sdr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysdr@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void swr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@yswr@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void cache(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@rcache");
}
void _sb(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sb %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _sh(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sh %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _swl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "swl %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _sw(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sw %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _sdl(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sdl %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _sdr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sdr %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _swr(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "swr %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}
void _cache(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "cache");
}
void lwc1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylwc1@w fpr%d, 0x%04x(%s)",
    _ft, (u16)_ofs16, _gpr[_base]);
}
void pref(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pref %02x, 0x%04x(%s)",
    _hint, (u16)_ofs16, _gpr[_base]);
}
void lqc2(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ylqc2");
}
void ld(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@yld@w %s, 0x%04x(%s)",
    _gpr[_rt], _imm, _gpr[_rs]);
}
void _lwc1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lwc1 fpr%d, 0x%04x(%s)",
    _ft, (u16)_ofs16, _gprp[_base]);
}
void _pref(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "pref %02x, 0x%04x(%s)",
    _hint, (u16)_ofs16, _gprp[_base]);
}
void _lqc2(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "lqc2");
}
void _ld(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "ld %s, 0x%04x(%s)",
    _gprp[_rt], _imm, _gprp[_rs]);
}
void swc1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@yswc1@w fpr%d, 0x%04x(%s)",
    _ft, (u16)_ofs16, _gpr[_base]);
}
void sqc2(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysqc2");
}
void sd(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "@ysd@w %s, 0x%04x(%s)",
    _gpr[_rt], (u16)_ofs16, _gpr[_base]);
}
void _swc1(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "swc1 fpr%d, 0x%04x(%s)",
    _ft, (u16)_ofs16, _gprp[_base]);
}
void _sqc2(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sqc2");
}
void _sd(u32 p_addr, u32 p_opcode, char* p_str) {
  sprintf(p_str, "sd %s, 0x%04x(%s)",
    _gprp[_rt], (u16)_ofs16, _gprp[_base]);
}

#undef _rs
#undef _rt
#undef _imm
#undef _ofs16
#undef _ofs26
#undef _base
#undef _ft
#undef _hint

//-----------------------------------------------
void disasm(u32 p_va, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x40])(u32, u32, char*) = {
    special, regimm, j,       jal,     beq,     bne,     blez,  bgtz,
    addi,    addiu,  slti,    sltiu,   andi,    ori,     xori,  lui,
    cop0,    cop1,   cop2,    invalid, beql,    bnel,    blezl, bgtzl,
    daddi,   daddiu, ldl,     ldr,     mmi,     invalid, lq,    sq,
    lb,      lh,     lwl,     lw,      lbu,     lhu,     lwr,   lwu,
    sb,      sh,     swl,     sw,      sdl,     sdr,     swr,   cache,
    invalid, lwc1,   invalid, pref,    invalid, invalid, lqc2,  ld,
    invalid, swc1,   invalid, invalid, invalid, invalid, sqc2,  sd,
  };
  TABLE[p_opcode >> 26](p_va, p_opcode, p_str);

  if (g_dbg->isInModuleCodeSection(p_va) == false) {
    return;
  }

  AnalyzeInfoMapItr itr = g_dbg->analyze_map_.find(p_va);
  if (itr == g_dbg->analyze_map_.end() ||
      itr->second.valid_estimate() == false) {
    return;
  }

  // 静的解析データがあれば情報をさらに付加する
  bool find = false;
  // 静的解析で得た予測値にラベル(コード)があればそれを表示
  Label* label = g_dbg->find_codelabel(itr->second.estimate_value());
  if (label) {
    char tmp[1024];
    sprintf(tmp, " @m%s@w", label->text());
    strcat(p_str, tmp);
    find = true;
  }
  // 静的解析で得た予測値にラベル(メモリ)があればそれを表示
  for (int k = 0; k < g_dbg->getMemLabelCount(); k++) {
    MemLabel* mem_label = g_dbg->getMemLabel(k);
    char tmp[1024];
    if (mem_label->GetMemoryLabelFromVA(itr->second.estimate_value(), tmp)) {
      strcat(p_str, tmp);
      find = true;
      break;
    }
  }
  if (find == false) {
    // 静的解析で得た予測値にシンボル情報があればそれを表示
    char *sym = g_dbg->findSymbolTableName(itr->second.estimate_value());
    if (sym) {
      char tmp[1024];
      sprintf(tmp, " @m%s@w", sym);
      strcat(p_str, tmp);
      find = true;
    }
  }
  if (find == false) {
    // 静的解析で得た予測値に文字列があればそれを表示
    StringMapItr string = g_dbg->string_map_.find(itr->second.estimate_value());
    if (string != g_dbg->string_map_.end()) {
      char tmp[1024];
      sprintf(tmp, " @o\"%s\"@w", string->second->text());
      strcat(p_str, tmp);
      find = true;
    }
  }
}

void disasm_plain(u32 p_va, u32 p_opcode, char* p_str) {
  static void (*TABLE[0x40])(u32, u32, char*) = {
    _special, _regimm, _j,       _jal,     _beq,     _bne,     _blez,  _bgtz,
    _addi,    _addiu,  _slti,    _sltiu,   _andi,    _ori,     _xori,  _lui,
    _cop0,    _cop1,   _cop2,    _invalid, _beql,    _bnel,    _blezl, _bgtzl,
    _daddi,   _daddiu, _ldl,     _ldr,     _mmi,     _invalid, _lq,    _sq,
    _lb,      _lh,     _lwl,     _lw,      _lbu,     _lhu,     _lwr,   _lwu,
    _sb,      _sh,     _swl,     _sw,      _sdl,     _sdr,     _swr,   _cache,
    _invalid, _lwc1,   _invalid, _pref,    _invalid, _invalid, _lqc2,  _ld,
    _invalid, _swc1,   _invalid, _invalid, _invalid, _invalid, _sqc2,  _sd,
  };
  TABLE[p_opcode >> 26](p_va, p_opcode, p_str);
}
