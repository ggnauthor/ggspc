#ifndef _debugger_disasm
#define _debugger_disasm

/*---------*/
/* include */
/*---------*/
#include "main.h"

/*--------*/
/* define */
/*--------*/

/*-------*/
/* class */
/*-------*/

/*----------*/
/* function */
/*----------*/
void disasm(u32 p_va, u32 p_opcode, char* p_str);
void disasm_plain(u32 p_va, u32 p_opcode, char* p_str);

#endif // _debugger_disasm
