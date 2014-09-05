#ifndef _elf
#define _elf

#include "main.h"

class Elf {
public:
  Elf() {}
  ~Elf() {}

  static u32 load(char* p_fname,
    u32* p_pgmhead, u32* p_pgmsize, u8* p_buf, u32 p_bufSize);

private:
  static int decSLEB128(u8* p_data, int* p_bc);
  static int decULEB128(u8* p_data, int* p_bc);
  static int sortSourceLocationEntryByAddr(
               const void* p_a, const void* p_b);
  static int sortSourceLocationEntryByNameLine(
               const void* p_a, const void* p_b);
};

#endif // _elf
