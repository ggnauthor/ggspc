#ifndef _memory_memory
#define _memory_memory

/*---------*/
/* include */
/*---------*/
#include "main.h"

/*--------*/
/* define */
/*--------*/
#define VA2PA(_va)  g_memory->va2pa(_va)

/*--------*/
/* struct */
/*--------*/
class Memory
{
// constant definition
public:
  enum {
    kMainMemoryBase  = 0,
    kMainMemorySize  = 32 * 1024 * 1024,  // 32mb
    kBootRomBase  = kMainMemoryBase + kMainMemorySize,
    kBootRomSize  = 4 * 1024 * 1024,  // 4mb
    kEERegisterBase  = kBootRomBase + kBootRomSize,
    kEERegisterSize  = 1024 * 1024,    // 1mb レジスタマップは64kbで十分だがアドレス変換の１ページが1mbなので（emu側の仕様なのでTLBを真面目にサポートする時に変わるかも）
    kVURegisterBase  = kEERegisterBase + kEERegisterSize,
    kVURegisterSize  = 1024 * 1024,    // 1mb
    kGSRegisterBase  = kVURegisterBase + kVURegisterSize,
    kGSRegisterSize  = 1024 * 1024,    // 1mb
    kScratchPadBase  = kGSRegisterBase + kGSRegisterSize,
    kScratchPadSize  = 1024 * 1024,    // 1mb スクラッチパッドは16kbで十分だがアドレス変換の１ページが1mbなので（emu側の仕様なのでTLBを真面目にサポートする時に変わるかも）
    kKernelMemBase  = kMainMemoryBase,
    kKernelMemSize  = 1024 * 1024,    // 1mb
  };

// constructor/destructor
public:
  Memory();
  ~Memory();

// member function
public:
  inline u32 va2pa(u32 p_va) {
    u32 pa = (u32)att_[p_va >> 20 & 0xfff];
    return pa ? pa + (p_va & 0xfffff) : NULL;
  }

  void clear();

// accessor
public:
  // 正直あまりatt_ポインタを公開したくないのだが、
  // リコンパイルしたコード内でattを取得する為に関数なんか呼びたくないのでやむを得ず
  inline u8** att() { return att_; }

// member variable
private:
  u8* att_[0x1000];
  u8* mem_;
};

/*--------*/
/* extern */
/*--------*/
extern Memory*  g_memory;

#endif // _memory_memory
