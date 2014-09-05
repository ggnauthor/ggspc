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
    kEERegisterSize  = 1024 * 1024,    // 1mb ���W�X�^�}�b�v��64kb�ŏ\�������A�h���X�ϊ��̂P�y�[�W��1mb�Ȃ̂Łiemu���̎d�l�Ȃ̂�TLB��^�ʖڂɃT�|�[�g���鎞�ɕς�邩���j
    kVURegisterBase  = kEERegisterBase + kEERegisterSize,
    kVURegisterSize  = 1024 * 1024,    // 1mb
    kGSRegisterBase  = kVURegisterBase + kVURegisterSize,
    kGSRegisterSize  = 1024 * 1024,    // 1mb
    kScratchPadBase  = kGSRegisterBase + kGSRegisterSize,
    kScratchPadSize  = 1024 * 1024,    // 1mb �X�N���b�`�p�b�h��16kb�ŏ\�������A�h���X�ϊ��̂P�y�[�W��1mb�Ȃ̂Łiemu���̎d�l�Ȃ̂�TLB��^�ʖڂɃT�|�[�g���鎞�ɕς�邩���j
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
  // �������܂�att_�|�C���^�����J�������Ȃ��̂����A
  // ���R���p�C�������R�[�h����att���擾����ׂɊ֐��Ȃ񂩌Ăт����Ȃ��̂ł�ނ𓾂�
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
