#include "memory/memory.h"

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

Memory*  g_memory = NULL;

Memory::Memory() {
  // 物理アドレスは繋がっていたほうが都合がいい
  // 少なくともメインメモリの直後にBIOSROMが必要(x86マッピングテーブルの都合)
  int size = kMainMemorySize + kBootRomSize + kEERegisterSize +
    kVURegisterSize + kGSRegisterSize + kScratchPadSize;
  mem_ = (u8*)malloc(size);
  memset(mem_, 0, size);

  u8* ptr = mem_;

  ptr += kMainMemorySize;
  u8* bootrom = ptr;

  ptr += kBootRomSize;
  u8* eereg = ptr;

  ptr += kEERegisterSize;
  u8* vureg = ptr;

  ptr += kVURegisterSize;
  u8* gsreg = ptr;

  ptr += kGSRegisterSize;
  u8* scratchpad = ptr;

  for (int i = 0x0000; i < 0x1000; i++) att_[i] = NULL;
  // メインメモリ    0x00000000-0x02000000
  for (int i = 0x000; i < 0x020; i++) att_[i] = mem_ + i * 0x100000;
  for (int i = 0x200; i < 0x220; i++) att_[i] = mem_ + (i - 0x200) * 0x100000;
  for (int i = 0x300; i < 0x320; i++) att_[i] = mem_ + (i - 0x300) * 0x100000;
  // EEレジスタマップ  0x10000000-0x11000000
  att_[0x100] = eereg;
  // VUレジスタマップ  0x11000000-0x12000000
  att_[0x110] = vureg;
  // GSレジスタマップ  0x12000000-0x13000000
  att_[0x120] = gsreg;
  // Boot ROM
  for (int i = 0x1fc; i < 0x200; i++) att_[i] = bootrom + (i - 0x1fc) * 0x100000;
  for (int i = 0x9fc; i < 0xa00; i++) att_[i] = bootrom + (i - 0x9fc) * 0x100000;
  for (int i = 0xbfc; i < 0xc00; i++) att_[i] = bootrom + (i - 0xbfc) * 0x100000;
  att_[0xfff] = bootrom + 0x300000;
  // スクラッチパッド
  att_[0x700] = scratchpad;
  // KSEG0
  for (int i = 0x800; i < 0x820; i++) att_[i] = mem_ + (i - 0x800) * 0x100000;
  att_[0x900] = eereg;
  att_[0x910] = vureg;
  att_[0x920] = gsreg;
  // KSEG1
  for (int i = 0xa00; i < 0xa20; i++) att_[i] = mem_ + (i - 0xa00) * 0x100000;
  att_[0xb00] = eereg;
  att_[0xb10] = vureg;
  att_[0xb20] = gsreg;
}

Memory::~Memory() {
  if (mem_) free(mem_);
}

void Memory::clear() {
  if (mem_) {
    int size = kMainMemorySize + kBootRomSize + kEERegisterSize +
               kVURegisterSize + kGSRegisterSize + kScratchPadSize;
    memset(mem_, 0, size);
  }
}
