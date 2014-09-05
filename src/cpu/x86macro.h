#ifndef _cpu_x86macro
#define _cpu_x86macro

//-----------------------------------------------------------------------------
// include

//-----------------------------------------------------------------------------
// define
#define wc1(_code) { *g_cpu->m_recompiling_code_ptr++ = (u8)(_code); }
#define wc2(_code) {                                \
  *((u16*)g_cpu->m_recompiling_code_ptr) = (_code); \
  g_cpu->m_recompiling_code_ptr += 2;               \
}
#define wc3(_code) { wc1(_code); wc1(_code >> 8); wc1(_code >> 16); }
#define wc4(_code) {                                \
  *((u32*)g_cpu->m_recompiling_code_ptr) = (_code); \
  g_cpu->m_recompiling_code_ptr += 4;               \
}

struct LABELREF {
  u8* addr; // 分岐命令のアドレス（飛び先との相対アドレスが必要なため）
  u8* ofs;  // バインド処理でアドレスを書き込む位置
  u8  idx;  // ラベル識別インデックス
};

#define LABEL_DEF_MAX  512
#define LABEL_REF_MAX  256

#define LABEL_DEF(_lbl)                             \
  assert(_lbl < LABEL_DEF_MAX);                     \
  g_labelDef[_lbl] = g_cpu->m_recompiling_code_ptr;

#define LABEL_REF(_lbl, _ofs) {                                            \
  assert(g_labelRefCount < LABEL_REF_MAX);                                 \
  g_labelRef[g_labelRefCount].ofs  = g_cpu->m_recompiling_code_ptr;        \
  g_labelRef[g_labelRefCount].addr = g_cpu->m_recompiling_code_ptr - _ofs; \
  g_labelRef[g_labelRefCount].idx  = _lbl;                                 \
  g_labelRefCount++;                                                       \
}

#define _eax 0
#define _ecx 1
#define _edx 2
#define _ebx 3
#define _esp 4
#define _ebp 5
#define _esi 6
#define _edi 7

#define _al 0
#define _cl 1
#define _dl 2
#define _bl 3
#define _ah 4
#define _ch 5
#define _dh 6
#define _bh 7

#define _ax 0
#define _cx 1
#define _dx 2
#define _bx 3
#define _sp 4
#define _bp 5
#define _si 6
#define _di 7

#define _xmm0 0
#define _xmm1 1
#define _xmm2 2
#define _xmm3 3
#define _xmm4 4
#define _xmm5 5
#define _xmm6 6
#define _xmm7 7

#define regbyte(_r0,_r1) ((_r0 << 3) | _r1)
//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 に 32bit レジスタ _r1 を処理する
// and, or, tst, adcの場合はオペランドのデコードが他と異なるので注意
#define mov32_reg_reg(_r0, _r1) { wc1(0x8b); wc1(0xc0 | regbyte(_r0, _r1)); }
#define add32_reg_reg(_r0, _r1) { wc1(0x03); wc1(0xc0 | regbyte(_r0, _r1)); }
#define sub32_reg_reg(_r0, _r1) { wc1(0x2b); wc1(0xc0 | regbyte(_r0, _r1)); }
#define xor32_reg_reg(_r0, _r1) { wc1(0x33); wc1(0xc0 | regbyte(_r0, _r1)); }
#define and32_reg_reg(_r0, _r1) { wc1(0x21); wc1(0xc0 | regbyte(_r1, _r0)); }
#define or32_reg_reg(_r0, _r1)  { wc1(0x09); wc1(0xc0 | regbyte(_r1, _r0)); }
#define cmp32_reg_reg(_r0, _r1) { wc1(0x3b); wc1(0xc0 | regbyte(_r0, _r1)); }
#define tst32_reg_reg(_r0, _r1) { wc1(0x85); wc1(0xc0 | regbyte(_r1, _r0)); }
#define adc32_reg_reg(_r0, _r1) { wc1(0x11); wc1(0xc0 | regbyte(_r1, _r0)); }
#define sbb32_reg_reg(_r0, _r1) { wc1(0x1b); wc1(0xc0 | regbyte(_r0, _r1)); }

//-----------------------------------------------------------------------------
// 16bit レジスタ _r0 に 16bit レジスタ_r1 を処理する
// and, or, tst, adcの場合はオペランドのデコードが他と異なるので注意
#define mov16_reg_reg(_r0, _r1) { wc2(0x8b66); wc1(0xc0 | regbyte(_r0, _r1)); }
#define add16_reg_reg(_r0, _r1) { wc2(0x0366); wc1(0xc0 | regbyte(_r0, _r1)); }
#define sub16_reg_reg(_r0, _r1) { wc2(0x2b66); wc1(0xc0 | regbyte(_r0, _r1)); }
#define xor16_reg_reg(_r0, _r1) { wc2(0x3366); wc1(0xc0 | regbyte(_r0, _r1)); }
#define and16_reg_reg(_r0, _r1) { wc2(0x2166); wc1(0xc0 | regbyte(_r1, _r0)); }
#define or16_reg_reg(_r0, _r1)  { wc2(0x0966); wc1(0xc0 | regbyte(_r1, _r0)); }
#define cmp16_reg_reg(_r0, _r1) { wc2(0x3b66); wc1(0xc0 | regbyte(_r0, _r1)); }
#define tst16_reg_reg(_r0, _r1) { wc2(0x8566); wc1(0xc0 | regbyte(_r1, _r0)); }
#define adc16_reg_reg(_r0, _r1) { wc2(0x1166); wc1(0xc0 | regbyte(_r1, _r0)); }
#define sbb16_reg_reg(_r0, _r1) { wc2(0x1b66); wc1(0xc0 | regbyte(_r0, _r1)); }

//-----------------------------------------------------------------------------
// 8bit レジスタ _r0 に 8bit レジスタ_r1 を処理する
// and, or, tstの場合はオペランドのデコードが他と異なるので注意
#define mov8_reg_reg(_r0, _r1) { wc1(0x8a); wc1(0xc0 | regbyte(_r0, _r1)); }
#define add8_reg_reg(_r0, _r1) { wc1(0x02); wc1(0xc0 | regbyte(_r0, _r1)); }
#define sub8_reg_reg(_r0, _r1) { wc1(0x2a); wc1(0xc0 | regbyte(_r0, _r1)); }
#define xor8_reg_reg(_r0, _r1) { wc1(0x32); wc1(0xc0 | regbyte(_r0, _r1)); }
#define and8_reg_reg(_r0, _r1) { wc1(0x20); wc1(0xc0 | regbyte(_r1, _r0)); }
#define or8_reg_reg(_r0, _r1)  { wc1(0x08); wc1(0xc0 | regbyte(_r1, _r0)); }
#define cmp8_reg_reg(_r0, _r1) { wc1(0x3a); wc1(0xc0 | regbyte(_r0, _r1)); }
#define tst8_reg_reg(_r0, _r1) { wc1(0x84); wc1(0xc0 | regbyte(_r1, _r0)); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 に _addr で指定したメモリを処理する
#define mov32_reg_mem(_r0, _addr) {              \
  wc1(0x8b); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define add32_reg_mem(_r0, _addr) {              \
  wc1(0x03); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define sub32_reg_mem(_r0, _addr) {              \
  wc1(0x2b); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define xor32_reg_mem(_r0, _addr) {              \
  wc1(0x33); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define and32_reg_mem(_r0, _addr) {              \
  wc1(0x23); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define or32_reg_mem(_r0, _addr)  {              \
  wc1(0x0b); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define cmp32_reg_mem(_r0, _addr) {              \
  wc1(0x3b); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define tst32_reg_mem(_r0, _addr) {              \
  wc1(0x85); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define adc32_reg_mem(_r0, _addr) {              \
  wc1(0x13); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define sbb32_reg_mem(_r0, _addr) {              \
  wc1(0x1b); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}

// 短縮命令
#define mov32_eax_mem(_addr) { wc1(0xa1); wc4(_addr); }

//-----------------------------------------------------------------------------
// 8bit レジスタ _r0 に _addr で指定したメモリを処理する
#define mov8_reg_mem(_r0, _addr) {               \
  wc1(0x8a); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}

// 短縮命令
#define mov8_al_mem(_addr) { wc1(0xa0); wc4(_addr); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 に 32bit レジスタ _r1 の指すメモリを処理する
#define mov32_reg_rmem(_r0, _r1) { wc1(0x8b); wc1(regbyte(_r0, _r1)); }
#define add32_reg_rmem(_r0, _r1) { wc1(0x03); wc1(regbyte(_r0, _r1)); }
#define sub32_reg_rmem(_r0, _r1) { wc1(0x2b); wc1(regbyte(_r0, _r1)); }
#define xor32_reg_rmem(_r0, _r1) { wc1(0x33); wc1(regbyte(_r0, _r1)); }
#define and32_reg_rmem(_r0, _r1) { wc1(0x23); wc1(regbyte(_r0, _r1)); }
#define or32_reg_rmem(_r0, _r1)  { wc1(0x0b); wc1(regbyte(_r0, _r1)); }
#define cmp32_reg_rmem(_r0, _r1) { wc1(0x3b); wc1(regbyte(_r0, _r1)); }
#define tst32_reg_rmem(_r0, _r1) { wc1(0x85); wc1(regbyte(_r0, _r1)); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 に 32bit レジスタ _r1 +
//                        8bit 即値 _ofs の指すメモリをコピーする
#define mov32_reg_rmem_ofs8(_r0, _r1, _ofs) {          \
  wc1(0x8b); wc1(0x40 | regbyte(_r0, _r1)); wc1(_ofs); \
}
// 32bit レジスタ _r0 に 32bit レジスタ _r1 +
//                       32bit 即値 _ofs の指すメモリをコピーする
#define mov32_reg_rmem_ofs32(_r0, _r1, _ofs) {         \
  wc1(0x8b); wc1(0x80 | regbyte(_r0, _r1)); wc4(_ofs); \
}
// 32bit レジスタ _r0 に 32bit レジスタ _r1 +
//                       32bit レジスタ _r2 * 4 の指すメモリをコピーする
//#define mov32_reg_rmem_rmem_x_4(_r0, _r1, _r2) { \
//  wc1(0x8b); wc1(0x04 | (_r0 << 3));             \
//  wc1(0x80 | regbyte(_r2, _r1));                 \
//}

// 32bit レジスタ _r0 に _addr +
//                       32bit レジスタ _r1 * 4 の指すメモリをコピーする
#define mov32_reg_mem_rofs_x_4(_r0, _addr, _r1) { \
  wc2(0x8b3e); wc1(0x04 | (_r0 << 3));            \
  wc1(0x85 | (_r1 << 3)); wc4(_addr);             \
}

// 8bit レジスタ _r0 に 32bit レジスタ _r1 +
//                       8bit 即値 _ofs の指すメモリをコピーする
#define mov8_reg_rmem_ofs8(_r0, _r1, _ofs) {           \
  wc1(0x8a); wc1(0x40 | regbyte(_r0, _r1)); wc1(_ofs); \
}

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 に 32bit 即値 _imm を処理する
#define mov32_reg_imm32(_r0, _imm) { wc1(0xb8 | _r0); wc4(_imm); }
#define add32_reg_imm32(_r0, _imm) { wc1(0x81); wc1(0xc0 | _r0); wc4(_imm); }
#define sub32_reg_imm32(_r0, _imm) { wc1(0x81); wc1(0xe8 | _r0); wc4(_imm); }
#define xor32_reg_imm32(_r0, _imm) { wc1(0x81); wc1(0xf0 | _r0); wc4(_imm); }
#define and32_reg_imm32(_r0, _imm) { wc1(0x81); wc1(0xe0 | _r0); wc4(_imm); }
#define or32_reg_imm32(_r0, _imm)  { wc1(0x81); wc1(0xc8 | _r0); wc4(_imm); }
#define cmp32_reg_imm32(_r0, _imm) { wc1(0x81); wc1(0xf8 | _r0); wc4(_imm); }
#define tst32_reg_imm32(_r0, _imm) { wc1(0xf7); wc1(0xf8 | _r0); wc4(_imm); }

#define shl32_reg_imm8(_r0, _imm)  { wc1(0xc1); wc1(0xe0 | _r0); wc1(_imm); }
#define shr32_reg_imm8(_r0, _imm)  { wc1(0xc1); wc1(0xe8 | _r0); wc1(_imm); }
#define sar32_reg_imm8(_r0, _imm)  { wc1(0xc1); wc1(0xf8 | _r0); wc1(_imm); }

#define shl32_reg_cl(_r0) { wc1(0xd3); wc1(0xe0 | _r0); }
#define shr32_reg_cl(_r0) { wc1(0xd3); wc1(0xe8 | _r0); }
#define sar32_reg_cl(_r0) { wc1(0xd3); wc1(0xf8 | _r0); }

// 短縮命令
#define add32_eax_imm32(_imm) { wc1(0x05); wc4(_imm); }
#define sub32_eax_imm32(_imm) { wc1(0x2d); wc4(_imm); }
#define xor32_eax_imm32(_imm) { wc1(0x35); wc4(_imm); }
#define and32_eax_imm32(_imm) { wc1(0x25); wc4(_imm); }
#define or32_eax_imm32(_imm)  { wc1(0x0d); wc4(_imm); }
#define cmp32_eax_imm32(_imm) { wc1(0x3d); wc4(_imm); }
#define tst32_eax_imm32(_imm) { wc1(0xa9); wc4(_imm); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 に 8bit 即値 _imm を処理する
#define add32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xc0 | _r0); wc1(_imm); }
#define sub32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xe8 | _r0); wc1(_imm); }
#define xor32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xf0 | _r0); wc1(_imm); }
#define and32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xe0 | _r0); wc1(_imm); }
#define or32_reg_imm8(_r0, _imm)  { wc1(0x83); wc1(0xc8 | _r0); wc1(_imm); }
#define cmp32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xf8 | _r0); wc1(_imm); }
#define adc32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xd0 | _r0); wc1(_imm); }
#define sbb32_reg_imm8(_r0, _imm) { wc1(0x83); wc1(0xd8 | _r0); wc1(_imm); }

//-----------------------------------------------------------------------------
// _addr で指定したメモリに 32bit レジスタ _r0 を 32bit 分処理する
#define mov32_mem_reg(_addr, _r0) {              \
  wc1(0x89); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define add32_mem_reg(_addr, _r0) {              \
  wc1(0x01); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define sub32_mem_reg(_addr, _r0) {              \
  wc1(0x29); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define xor32_mem_reg(_addr, _r0) {              \
  wc1(0x31); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define and32_mem_reg(_addr, _r0) {              \
  wc1(0x21); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define or32_mem_reg(_addr, _r0)  {              \
  wc1(0x09); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define cmp32_mem_reg(_addr, _r0) {              \
  wc1(0x39); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}
#define tst32_mem_reg(_addr, _r0) tst32_reg_mem(_r0, _addr)

// 短縮命令
#define mov32_mem_eax(_addr) { wc1(0xa3); wc4(_addr); }

//-----------------------------------------------------------------------------
// _addr で指定したメモリに 8bit レジスタ _r0 を 8bit 分処理する
#define mov8_mem_reg(_addr, _r0) {               \
  wc1(0x88); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}

// 短縮命令
#define mov8_mem_al(_addr) { wc1(0xa2); wc4(_addr); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すメモリに 32bit レジスタ _r1 を 32bit 分処理する
#define mov32_rmem_reg(_r0, _r1) { wc1(0x89); wc1(regbyte(_r1, _r0)); }
#define add32_rmem_reg(_r0, _r1) { wc1(0x01); wc1(regbyte(_r1, _r0)); }
#define sub32_rmem_reg(_r0, _r1) { wc1(0x29); wc1(regbyte(_r1, _r0)); }
#define xor32_rmem_reg(_r0, _r1) { wc1(0x31); wc1(regbyte(_r1, _r0)); }
#define and32_rmem_reg(_r0, _r1) { wc1(0x21); wc1(regbyte(_r1, _r0)); }
#define or32_rmem_reg(_r0, _r1)  { wc1(0x09); wc1(regbyte(_r1, _r0)); }
#define cmp32_rmem_reg(_r0, _r1) { wc1(0x39); wc1(regbyte(_r1, _r0)); }
#define tst32_rmem_reg(_r0, _r1) test_reg_rmem(_r0, _r1)

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すメモリに 16bit レジスタ _r1 を 16bit 分処理する
#define mov_rmem_reg_16(_r0, _r1) { wc2(0x8966); wc1(regbyte(_r1, _r0)); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すメモリに 8bit レジスタ _r1 を 8bit 分処理する
#define mov_rmem_reg_8(_r0, _r1) { wc1(0x88); wc1(regbyte(_r1, _r0)); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すメモリ + 8bit 即値 _ofs の指すメモリに
//                                  32bit レジスタ _r1 を 32bit 分処理する
#define mov32_rmem_ofs8_reg(_r0, _ofs, _r1) {          \
  wc1(0x89); wc1(0x40 | regbyte(_r1, _r0)); wc1(_ofs); \
}

//-----------------------------------------------------------------------------
// _addr で指定したメモリに 32bit 即値 _imm を 32bit 分処理する
#define mov32_mem_imm32(_addr, _imm) { wc2(0x05c7); wc4(_addr); wc4(_imm); }
#define add32_mem_imm32(_addr, _imm) { wc2(0x0581); wc4(_addr); wc4(_imm); }
#define sub32_mem_imm32(_addr, _imm) { wc2(0x2d81); wc4(_addr); wc4(_imm); }
#define xor32_mem_imm32(_addr, _imm) { wc2(0x3581); wc4(_addr); wc4(_imm); }
#define and32_mem_imm32(_addr, _imm) { wc2(0x2581); wc4(_addr); wc4(_imm); }
#define or32_mem_imm32(_addr, _imm)  { wc2(0x0d81); wc4(_addr); wc4(_imm); }
#define cmp32_mem_imm32(_addr, _imm) { wc2(0x3d81); wc4(_addr); wc4(_imm); }
#define tst32_mem_imm32(_addr, _imm) { wc2(0x05f7); wc4(_addr); wc4(_imm); }

//-----------------------------------------------------------------------------
// _addr で指定したメモリに 8bit 即値 _imm を 8bit 分処理する
#define mov8_reg_imm8(_r0, _imm) { wc1(0xb0 | _r0); wc1(_imm); }
#define add8_reg_imm8(_r0, _imm) { wc1(0x04 | _r0); wc1(_imm); }
#define sub8_reg_imm8(_r0, _imm) { wc1(0x2c | _r0); wc1(_imm); }
#define xor8_reg_imm8(_r0, _imm) { wc1(0x34 | _r0); wc1(_imm); }
#define and8_reg_imm8(_r0, _imm) { wc1(0x24 | _r0); wc1(_imm); }
#define or8_reg_imm8(_r0, _imm)  { wc1(0x0c | _r0); wc1(_imm); }
#define cmp8_reg_imm8(_r0, _imm) { wc1(0x80); wc1(0xf8 | _r0); wc1(_imm); }
#define tst8_reg_imm8(_r0, _imm) { wc1(0xf6); wc1(0xc0 | _r0); wc1(_imm); }

#define tst8_eax_imm8(_imm) { wc1(0xa8); wc1(_imm); }

//-----------------------------------------------------------------------------
// _addr で指定したメモリに 8bit 即値 _imm を 8bit 分処理する
#define mov8_mem_imm8(_addr, _imm) { wc2(0x05c6); wc4(_addr); wc1(_imm); }
#define add8_mem_imm8(_addr, _imm) { wc2(0x0583); wc4(_addr); wc1(_imm); }
#define sub8_mem_imm8(_addr, _imm) { wc2(0x2d83); wc4(_addr); wc1(_imm); }
#define xor8_mem_imm8(_addr, _imm) { wc2(0x3583); wc4(_addr); wc1(_imm); }
#define and8_mem_imm8(_addr, _imm) { wc2(0x2583); wc4(_addr); wc1(_imm); }
#define or8_mem_imm8(_addr, _imm)  { wc2(0x0d83); wc4(_addr); wc1(_imm); }
#define cmp8_mem_imm8(_addr, _imm) { wc2(0x3d83); wc4(_addr); wc1(_imm); }
#define tst8_mem_imm8(_addr, _imm) { wc2(0x05f6); wc4(_addr); wc1(_imm); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すメモリに 32bit 即値 _imm を 32bit 分処理する
#define mov32_rmem_imm32(_r0, _imm) { wc1(0xc7); wc1(_r0); wc4(_imm); }
#define add32_rmem_imm32(_r0, _imm) { wc1(0x81); wc1(_r0); wc4(_imm); }
#define sub32_rmem_imm32(_r0, _imm) { wc1(0x81); wc1(0x28 | _r0); wc4(_imm); }
#define xor32_rmem_imm32(_r0, _imm) { wc1(0x81); wc1(0x30 | _r0); wc4(_imm); }
#define and32_rmem_imm32(_r0, _imm) { wc1(0x81); wc1(0x20 | _r0); wc4(_imm); }
#define or32_rmem_imm32(_r0, _imm)  { wc1(0x81); wc1(0x08 | _r0); wc4(_imm); }
#define cmp32_rmem_imm32(_r0, _imm) { wc1(0x81); wc1(0x38 | _r0); wc4(_imm); }
#define tst32_rmem_imm32(_r0, _imm) { wc1(0xf7); wc1(_r0); wc4(_imm); }

#define add32_rmem_imm8(_r0, _imm) { wc1(0x83); wc1(_r0); wc1(_imm); }
#define sub32_rmem_imm8(_r0, _imm) { wc1(0x83); wc1(0x28 | _r0); wc1(_imm); }
#define xor32_rmem_imm8(_r0, _imm) { wc1(0x83); wc1(0x30 | _r0); wc1(_imm); }
#define and32_rmem_imm8(_r0, _imm) { wc1(0x83); wc1(0x20 | _r0); wc1(_imm); }
#define or32_rmem_imm8(_r0, _imm)  { wc1(0x83); wc1(0x08 | _r0); wc1(_imm); }
#define cmp32_rmem_imm8(_r0, _imm) { wc1(0x83); wc1(0x38 | _r0); wc1(_imm); }
#define tst32_rmem_imm8(_r0, _imm) { wc1(0xf6); wc1(_r0); wc1(_imm); }

// 32bit レジスタ _r0 + 8bit 即値 _ofs の指すメモリと
//                      8bit 即値 _imm のand結果によってフラグを設定する
#define tst8_rmem_ofs8_imm8(_r0, _ofs, _imm) {      \
  wc1(0xf6); wc1(0x40 | _r0); wc1(_ofs); wc1(_imm); \
}

//-----------------------------------------------------------------------------
// _lbl で指定されたラベルにジャンプする。
// 但しジャンプ先は +127 ～ -128 の範囲であること
#define jo(_lbl)    { wc1(0x70); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jno(_lbl)   { wc1(0x71); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jb(_lbl)    { wc1(0x72); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jae(_lbl)   { wc1(0x73); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jz(_lbl)    { wc1(0x74); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jnz(_lbl)   { wc1(0x75); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jbe(_lbl)   { wc1(0x76); wc1(0x00); LABEL_REF(_lbl, 1); }
#define ja(_lbl)    { wc1(0x77); wc1(0x00); LABEL_REF(_lbl, 1); }
#define js(_lbl)    { wc1(0x78); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jns(_lbl)   { wc1(0x79); wc1(0x00); LABEL_REF(_lbl, 1); }

#define jl(_lbl)    { wc1(0x7c); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jle(_lbl)   { wc1(0x7e); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jg(_lbl)    { wc1(0x7f); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jge(_lbl)   { wc1(0x7d); wc1(0x00); LABEL_REF(_lbl, 1); }

#define jecxz(_lbl) { wc1(0xe3); wc1(0x00); LABEL_REF(_lbl, 1); }
#define jmp(_lbl)   { wc1(0xeb); wc1(0x00); LABEL_REF(_lbl, 1); }

#define jc(_lbl)    jb(_lbl)
#define jnc(_lbl)   jae(_lbl)

#define jnz32(_lbl) { wc2(0x850f); wc4(0x00000000); LABEL_REF(_lbl, 4); }
#define jz32(_lbl)  { wc2(0x840f); wc4(0x00000000); LABEL_REF(_lbl, 4); }
#define jmp32(_lbl) { wc1(0xe9); wc4(0x00000000); LABEL_REF(_lbl, 4); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すアドレスにジャンプする
#define jmp_reg(_r0) { wc1(0xff); wc1(0xe0 | _r0); }

//-----------------------------------------------------------------------------
// 32bit レジスタ _r0 の指すメモリの指すアドレスにジャンプする
#define jmp_rmem(_r0) { wc1(0xff); wc1(0x20 | _r0); }

//-----------------------------------------------------------------------------
// eax に eax と 32bit レジスタ _r0 を符号無し乗算したものを格納する
#define mul_reg(_r0)   { wc1(0xf7); wc1(0xe0 | _r0); } // 符号無し
#define mul_mem(_addr) { wc2(0x25f7); wc4(_addr); }
#define mul_rmem(_r0)  { wc1(0xf7); wc1(0x20 | _r0); }

//-----------------------------------------------------------------------------
// eax に eax と 32bit レジスタ _r0 を符号付き乗算したものを格納する
#define imul_reg(_r0)   { wc1(0xf7); wc1(0xe8 | _r0); } // 符号付き
#define imul_mem(_addr) { wc2(0x2df7); wc4(_addr); }
#define imul_rmem(_r0)  { wc1(0xf7); wc1(0x28 | _r0); }

//-----------------------------------------------------------------------------
// eax に eax と 32bit レジスタ _r0 を符号無し除算したものを格納する
#define div_reg(_r0)   { wc1(0xf7); wc1(0xf0 | _r0); } // 符号無し
#define div_mem(_addr) { wc2(0x35f7); wc4(_addr); }
#define div_rmem(_r0)  { wc1(0xf7); wc1(0x30 | _r0); }

//-----------------------------------------------------------------------------
// eax に eax と 32bit レジスタ _r0 を符号付き除算したものを格納する
#define idiv_reg(_r0)   { wc1(0xf7); wc1(0xf8 | _r0); } // 符号付き
#define idiv_mem(_addr) { wc2(0x3df7); wc4(_addr); }
#define idiv_rmem(_r0)  { wc1(0xf7); wc1(0x38 | _r0); }

//-----------------------------------------------------------------------------
// 他
#define nop()            { wc1(0x90); }
#define call_eax()       { wc2(0xd0ff); }
#define call_reg(_r0)    { wc2(0xd0ff | _r0 << 8); }
#define call_imm32(_imm) { wc1(0xe8); wc4(_imm); }
#define ret()            { wc1(0xc3); }
#define cwde()           { wc1(0x98); } // axを符号拡張しeaxに入れる
#define cdq()            { wc1(0x99); } // eaxを符号拡張しedxに入れる
#define neg32(_r0)       { wc1(0xf7); wc1(0xd8 | _r0); }
#define neg16(_r0)       { wc2(0xf766); wc1(0xd8 | _r0); }
#define neg8(_r0)        { wc1(0xf6); wc1(0xd8 | _r0); }
#define not32(_r0)       { wc1(0xf7); wc1(0xd0 | _r0); }
#define not8(_r0)        { wc1(0xf6); wc1(0xd0 | _r0); }
#define inc32(_r0)       { wc1(0x40 | _r0); }
#define inc32_mem(_addr) { wc2(0x05ff); wc4(_addr); }
#define inc32_eaxmem()   { wc2(0x00ff); }
#define inc32_edxmem()   { wc2(0x02ff); }
#define dec32(_r0)       { wc1(0x48 | _r0); }
#define dec32_mem(_addr) { wc2(0x0dff); wc4(_addr); }
#define dec32_eaxmem()   { wc2(0x08ff); }
#define dec32_edxmem()   { wc2(0x0aff); }
#define pop_reg(_r0)     { wc1(0x58 | _r0); }
#define push_reg(_r0)    { wc1(0x50 | _r0); }
#define push32_imm(_imm) { wc1(0x68); wc4(_imm); }
#define push8_imm(_imm)  { wc1(0x6a); wc1(_imm); }
#define inc16(_r0)       { wc1(0x66); wc1(0x40 | _r0); }
#define dec16(_r0)       { wc1(0x66); wc1(0x48 | _r0); }

// フラグ取得
#define seto(_r0)  { wc2(0x900f); wc1(0xc0 | _r0); }
#define setno(_r0) { wc2(0x910f); wc1(0xc0 | _r0); }
#define setb(_r0)  { wc2(0x920f); wc1(0xc0 | _r0); }
#define setae(_r0) { wc2(0x930f); wc1(0xc0 | _r0); }
#define sete(_r0)  { wc2(0x940f); wc1(0xc0 | _r0); }
#define setne(_r0) { wc2(0x950f); wc1(0xc0 | _r0); }
#define setbe(_r0) { wc2(0x960f); wc1(0xc0 | _r0); }
#define seta(_r0)  { wc2(0x970f); wc1(0xc0 | _r0); }
#define sets(_r0)  { wc2(0x980f); wc1(0xc0 | _r0); }
#define setns(_r0) { wc2(0x990f); wc1(0xc0 | _r0); }
#define setl(_r0)  { wc2(0x9c0f); wc1(0xc0 | _r0); }
#define setge(_r0) { wc2(0x9d0f); wc1(0xc0 | _r0); }
#define setle(_r0) { wc2(0x9e0f); wc1(0xc0 | _r0); }
#define setg(_r0)  { wc2(0x9f0f); wc1(0xc0 | _r0); }

// _r0 に _r1 + _imm を代入する
#define lea_reg_reg_imm8(_r0, _r1, _imm) {             \
  wc1(0x8d); wc1(0x40 | regbyte(_r0, _r1)); wc1(_imm); \
}
#define lea_reg_reg_imm32(_r0, _r1, _imm) {            \
  wc1(0x8d); wc1(0x80 | regbyte(_r0, _r1)); wc4(_imm); \
}

// _r0 に _r1 + _r2 * 4 を代入する
#define lea_reg_reg_p_reg_x_4(_r0, _r1, _r2) { \
  wc1(0x8d); wc1(0x04 | (_r0 << 3));           \
  wc1(0x80 | regbyte(_r2, _r1));               \
}

// _r0 に _r1 * 4 + _imm を代入する
#define lea_reg_reg_x_4_p_imm(_r0, _r1, _imm) { \
  wc1(0x8d); wc1(0x04 | (_r0 << 3));            \
  wc1(0x85 | (_r1 << 3)); wc4(_imm);            \
}

// _r0 に _r1 + _r2 * 4 + _imm を代入する
#define lea_reg_reg_p_reg_x_4_p_imm(_r0, _r1, _r2, _imm) { \
  wc1(0x8d); wc1(0x84 | (_r0 << 3));                       \
  wc1(0x80 | regbyte(_r2, _r1)); wc4(_imm);                \
}

// 符号／ゼロ拡張 8bit > 32bit
#define movzx32_reg_rmem8(_r0, _r1) { wc2(0xb60f); wc1(regbyte(_r0, _r1)); }
#define movsx32_reg_rmem8(_r0, _r1) { wc2(0xbe0f); wc1(regbyte(_r0, _r1)); }

// 符号／ゼロ拡張 16bit > 32bit
#define movzx32_reg_rmem16(_r0, _r1) { wc2(0xb70f); wc1(regbyte(_r0, _r1)); }
#define movsx32_reg_rmem16(_r0, _r1) { wc2(0xbf0f); wc1(regbyte(_r0, _r1)); }

// 条件付mov
#define cmovz32_reg_reg(_r0, _r1) {           \
  wc2(0x440f); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define cmovz32_reg_mem(_r0, _addr) {              \
  wc2(0x440f); wc1(0x05 | (_r0 << 3)); wc4(_addr); \
}

// ストリング命令
#define movsb()     { wc1(0xa4); }
#define rep_movsb() { wc2(0xa4f3); }

//-----------------------------------------------------------------------------
// FPU

// ebxをebpの保存用として確保する。fpu_endまでにebx(bx,bh,bl)を使用しないこと
#define fpu_begin() {                              \
  mov32_reg_reg(_ebx, _ebp);                       \
  mov32_reg_imm32(_ebp, (u32)&g_cpu->m_fpu + 128); \
}
#define fpu_end() { mov32_reg_reg(_ebp, _ebx); }

// ロード
#define fld_1()           { wc2(0xe8d9); }
#define fld_mem(_addr)    { wc2(0x05d9); wc4(_addr); }
#define fld_relm(_ofs)    { wc2(0x45d9); wc1(_ofs - 128); }
#define fild32_mem(_addr) { wc2(0x05db); wc4(_addr); } // 整数→少数変換

// 加算
#define fadd_relm(_ofs) {       \
  /* st0 += [ebp + _ofs] */     \
  wc2(0x45d8); wc1(_ofs - 128); \
}
#define fadd_st0_stx(_r0) {   \
  /* st0 += stx */            \
  wc1(0xd8); wc1(0xc0 | _r0); \
}
#define fadd_stx_st0(_r0) {   \
  /* stx += st0 */            \
  wc1(0xdc); wc1(0xc0 | _r0); \
}
#define faddp_stx_st0(_r0) {                       \
  /* stx += st0 FPUレジスタスタックをポップする */ \
  wc1(0xde); wc1(0xc0 | _r0);                      \
}

// 減算
#define fsub_relm(_ofs) {       \
  /* st0 -= [ebp + _ofs] */     \
  wc2(0x65d8); wc1(_ofs - 128); \
}
#define fsub_st0_stx(_r0) {   \
  /* st0 -= stx */            \
  wc1(0xd8); wc1(0xe0 | _r0); \
}
#define fsub_stx_st0(_r0) {   \
  /* stx -= st0 */            \
  wc1(0xdc); wc1(0xe8 | _r0); \
}
#define fsubp_stx_st0(_r0) {                       \
  /* stx -= st0 FPUレジスタスタックをポップする */ \
  wc1(0xde); wc1(0xe8 | _r0);                      \
}

// 乗算
#define fmul_relm(_ofs) {       \
  /* st0 *= [ebp + _ofs] */     \
  wc2(0x4dd8); wc1(_ofs - 128); \
}
#define fmul_st0_stx(_r0) {   \
  /* st0 *= stx */            \
  wc1(0xd8); wc1(0xc8 | _r0); \
}
#define fmul_stx_st0(_r0) {   \
  /* stx *= st0 */            \
  wc1(0xdc); wc1(0xc8 | _r0); \
}
#define fmulp_stx_st0(_r0) {                       \
  /* stx *= st0 FPUレジスタスタックをポップする */ \
  wc1(0xde); wc1(0xc8 | _r0);                      \
}

// 除算
#define fdiv_relm(_ofs) {       \
  /* st0 /= [ebp + _ofs] */     \
  wc2(0x7dd8); wc1(_ofs - 128); \
}
#define fdiv_st0_stx(_r0) {   \
  /* st0 /= stx */            \
  wc1(0xd8); wc1(0xf0 | _r0); \
}
#define fdiv_stx_st0(_r0) {   \
  /* stx /= st0 */            \
  wc1(0xdc); wc1(0xf8 | _r0); \
}
#define fdivp_stx_st0(_r0) {                       \
  /* stx /= st0 FPUレジスタスタックをポップする */ \
  wc1(0xde); wc1(0xf8 | _r0);                      \
}

// 除算(逆op)
#define fdivr_relm(_ofs) {       \
  /* st0 = [ebp + _ofs] / st0 */ \
  wc2(0x75d8); wc1(_ofs - 128);  \
}
#define fdivr_st0_stx(_r0) {  \
  /* st0 = stx / st0 */       \
  wc1(0xd8); wc1(0xf8 | _r0); \
}
#define fdivr_stx_st0(_r0) {  \
  /* stx = st0 / stx */       \
  wc1(0xdc); wc1(0xf0 | _r0); \
}
#define fdivrp_stx_st0(_r0) {                           \
  /* stx = st0 / stx FPUレジスタスタックをポップする */ \
  wc1(0xde); wc1(0xf0 | _r0);                           \
}

// ストア(_ofsはebpの符号無し相対アドレス。
//        fpu_beginを呼んでいれば regidx * 4 でfprにアクセス可能)
#define fstp_mem_st0(_addr)    { wc2(0x1dd9); wc4(_addr); }
#define fstp_relm_st0(_ofs)    { wc2(0x5dd9); wc1(_ofs - 128); }
#define fistp32_mem_st0(_addr) { wc2(0x1ddb); wc4(_addr); } // 少数→整数変換

// 比較(EFLAGSにも反映)
#define fcomi_st0_stx(_r0)  { wc1(0xdb); wc1(0xf0 | _r0); }
#define fcomip_st0_stx(_r0) { wc1(0xdf); wc1(0xf0 | _r0); }
#define ftest_st0()         { wc2(0xe4d9); }

// 絶対値・符号の反転
#define fabs_st0() { wc2(0xe1d9); } // st0 = |st0|
#define fchs_st0() { wc2(0xe0d9); } // st0 *= -1.0f

// 平方根
#define fsqrt_st0() { wc2(0xfad9); } // st0 = sqrt(st0)

// ステータスワードのストア
#define fnstsw_ax() { wc2(0xe0df); }
#define fnclex()    { wc2(0xe2db); }

// FPU環境保存と復元
#define fldenv_mem(_addr) { wc2(0x25d9); wc4(_addr); }
#define fstenv_mem(_addr) { wc2(0x35d9); wc4(_addr); }

// コントロールワード制御
#define fldcw_mem(_addr) { wc2(0x2dd9); wc4(_addr); }
#define fstcw_mem(_addr) { wc2(0x3dd9); wc4(_addr); }

// スタックヘッドのインクリメント
#define fincstp() { wc2(0xf7d9); }
#define fdecstp() { wc2(0xf6d9); }

// レジスタを空にする
#define ffree(_r0) { wc1(0xdd);  wc1(0xc0 | _r0); }

// レジスタの交換
#define fxch_st0_stx(_r0) { wc1(0xd9);  wc1(0xc8 | _r0); }

//-----------------------------------------------------------------------------
// SSE
#define movdqa_reg_rmem(_r0, _r1) {             \
  wc3(0x6f0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define movdqa_reg_reg(_r0, _r1) {              \
  wc3(0x6f0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define movdqa_rmem_reg(_r0, _r1) {             \
  wc3(0x7f0f66); wc1(0x00 | regbyte(_r1, _r0)); \
}

// 加算 ラップアラウンド
#define paddq_reg_rmem(_r0, _r1) {              \
  wc3(0xd40f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddq_reg_reg(_r0, _r1) {               \
  wc3(0xd40f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define paddd_reg_rmem(_r0, _r1) {              \
  wc3(0xfe0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddd_reg_reg(_r0, _r1) {               \
  wc3(0xfe0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define paddw_reg_rmem(_r0, _r1) {              \
  wc3(0xfd0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddw_reg_reg(_r0, _r1) {               \
  wc3(0xfd0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define paddb_reg_rmem(_r0, _r1) {              \
  wc3(0xfc0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddb_reg_reg(_r0, _r1) {               \
  wc3(0xfc0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 加算 符号付き飽和
#define paddsw_reg_rmem(_r0, _r1) {             \
  wc3(0xed0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddsw_reg_reg(_r0, _r1) {              \
  wc3(0xed0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define paddsb_reg_rmem(_r0, _r1) {             \
  wc3(0xec0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddsb_reg_reg(_r0, _r1) {              \
  wc3(0xec0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 加算 符号無し飽和
#define paddusw_reg_rmem(_r0, _r1) {            \
  wc3(0xdd0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddusw_reg_reg(_r0, _r1) {             \
  wc3(0xdd0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define paddusb_reg_rmem(_r0, _r1) {            \
  wc3(0xdc0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define paddusb_reg_reg(_r0, _r1) {             \
  wc3(0xdc0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 減算 ラップアラウンド
#define psubq_reg_rmem(_r0, _r1) {              \
  wc3(0xfb0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubq_reg_reg(_r0, _r1) {               \
  wc3(0xfb0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define psubd_reg_rmem(_r0, _r1) {              \
  wc3(0xfa0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubd_reg_reg(_r0, _r1) {               \
  wc3(0xfa0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define psubw_reg_rmem(_r0, _r1) {              \
  wc3(0xf90f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubw_reg_reg(_r0, _r1) {               \
  wc3(0xf90f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define psubb_reg_rmem(_r0, _r1) {              \
  wc3(0xf80f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubb_reg_reg(_r0, _r1) {               \
  wc3(0xf80f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 減算 符号付き飽和
#define psubsw_reg_rmem(_r0, _r1) {             \
  wc3(0xe90f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubsw_reg_reg(_r0, _r1) {              \
  wc3(0xe90f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define psubsb_reg_rmem(_r0, _r1) {             \
  wc3(0xe80f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubsb_reg_reg(_r0, _r1) {              \
  wc3(0xe80f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 減算 符号無し飽和
#define psubusw_reg_rmem(_r0, _r1) {            \
  wc3(0xd90f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubusw_reg_reg(_r0, _r1) {             \
  wc3(0xd90f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define psubusb_reg_rmem(_r0, _r1) {            \
  wc3(0xd80f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define psubusb_reg_reg(_r0, _r1) {             \
  wc3(0xd80f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 最大値・最小値
#define pmaxsw_reg_rmem(_r0, _r1) {             \
  wc3(0xee0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pmaxsw_reg_reg(_r0, _r1) {              \
  wc3(0xee0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pminsw_reg_rmem(_r0, _r1) {             \
  wc3(0xea0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pminsw_reg_reg(_r0, _r1) {              \
  wc3(0xea0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 比較
#define pcmpgtd_reg_rmem(_r0, _r1) {            \
  wc3(0x660f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pcmpgtd_reg_reg(_r0, _r1) {             \
  wc3(0x660f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pcmpgtw_reg_rmem(_r0, _r1) {            \
  wc3(0x650f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pcmpgtw_reg_reg(_r0, _r1) {             \
  wc3(0x650f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pcmpgtb_reg_rmem(_r0, _r1) {            \
  wc3(0x640f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pcmpgtb_reg_reg(_r0, _r1) {             \
  wc3(0x640f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

#define pcmpeqd_reg_rmem(_r0, _r1) {            \
  wc3(0x760f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pcmpeqd_reg_reg(_r0, _r1) {             \
  wc3(0x760f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pcmpeqw_reg_rmem(_r0, _r1) {            \
  wc3(0x750f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pcmpeqw_reg_reg(_r0, _r1) {             \
  wc3(0x750f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pcmpeqb_reg_rmem(_r0, _r1) {            \
  wc3(0x740f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pcmpeqb_reg_reg(_r0, _r1) {             \
  wc3(0x740f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// アンパック
#define punpcklbw_reg_rmem(_r0, _r1) {          \
  wc3(0x600f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define punpcklbw_reg_reg(_r0, _r1) {           \
  wc3(0x600f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define punpcklwd_reg_rmem(_r0, _r1) {          \
  wc3(0x610f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define punpcklwd_reg_reg(_r0, _r1) {           \
  wc3(0x610f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define punpckldq_reg_rmem(_r0, _r1) {          \
  wc3(0x620f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define punpckldq_reg_reg(_r0, _r1) {           \
  wc3(0x620f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define punpcklqdq_reg_reg(_r0, _r1) {          \
  wc3(0x6c0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

#define punpckhbw_reg_rmem(_r0, _r1) {          \
  wc3(0x680f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define punpckhbw_reg_reg(_r0, _r1) {           \
  wc3(0x680f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define punpckhwd_reg_rmem(_r0, _r1) {          \
  wc3(0x690f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define punpckhwd_reg_reg(_r0, _r1) {           \
  wc3(0x690f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define punpckhdq_reg_rmem(_r0, _r1) {          \
  wc3(0x6a0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define punpckhdq_reg_reg(_r0, _r1) {           \
  wc3(0x6a0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define punpckhqdq_reg_reg(_r0, _r1) {          \
  wc3(0x6d0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// バイト単位シフト
#define psrldq_reg_imm(_r0, _imm) { wc4(0xd8730f66 | (_r0 << 24)); wc1(_imm); }
#define pslldq_reg_imm(_r0, _imm) { wc4(0xf8730f66 | (_r0 << 24)); wc1(_imm); }

// 論理演算
#define por_reg_rmem(_r0, _r1) {                \
  wc3(0xeb0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define por_reg_reg(_r0, _r1) {                 \
  wc3(0xeb0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pxor_reg_rmem(_r0, _r1) {               \
  wc3(0xef0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pxor_reg_reg(_r0, _r1) {                \
  wc3(0xef0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pand_reg_rmem(_r0, _r1) {               \
  wc3(0xdb0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pand_reg_reg(_r0, _r1) {                \
  wc3(0xdb0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pandn_reg_rmem(_r0, _r1) {              \
  /* dstのみをnotしてsrcとのandを取る */        \
  wc3(0xdf0f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pandn_reg_reg(_r0, _r1) {               \
  wc3(0xdf0f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// 乗除算
#define pmuludq_reg_rmem(_r0, _r1) {            \
  wc3(0xf40f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pmuludq_reg_reg(_r0, _r1) {             \
  wc3(0xf40f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pmulhw_reg_rmem(_r0, _r1) {             \
  wc3(0xe50f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pmulhw_reg_reg(_r0, _r1) {              \
  wc3(0xe50f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}
#define pmullw_reg_rmem(_r0, _r1) {             \
  wc3(0xd50f66); wc1(0x00 | regbyte(_r0, _r1)); \
}
#define pmullw_reg_reg(_r0, _r1) {              \
  wc3(0xd50f66); wc1(0xc0 | regbyte(_r0, _r1)); \
}

// シャッフル
#define pshufd_reg_reg(_r0, _r1, _i3, _i2, _i1, _i0) { \
  wc3(0x700f66); wc1(0xc0 | regbyte(_r0, _r1));        \
  wc1(_i0 << 6 | _i1 << 4 | _i2 << 2 | _i3);           \
}
#define pshufhw_reg_reg(_r0, _r1, _i3, _i2, _i1, _i0) { \
  wc3(0x700ff3); wc1(0xc0 | regbyte(_r0, _r1));         \
  wc1(_i0 << 6 | _i1 << 4 | _i2 << 2 | _i3);            \
}
#define pshuflw_reg_reg(_r0, _r1, _i3, _i2, _i1, _i0) { \
  wc3(0x700ff2); wc1(0xc0 | regbyte(_r0, _r1));         \
  wc1(_i0 << 6 | _i1 << 4 | _i2 << 2 | _i3);            \
}

// 汎用レジスタとのやり取り
#define pinsrw_xmm_reg_imm8(_x0, _r0, _i0) {    \
  wc3(0xc40f66); wc1(0xc0 | regbyte(_x0, _r0)); \
  wc1(_i0); /* xmmへ転送 */                     \
}
#define pextrw_reg_xmm_imm8(_r0, _x0, _i0) {    \
  wc3(0xc50f66); wc1(0xc0 | regbyte(_r0, _x0)); \
  wc1(_i0); /* regへ転送 */                     \
}

#define _GPR_D(_a, _b) ((u32)&g_cpu->m_gpr[_a].d[_b])
#define _FPR_D(_a)     ((u32)&g_cpu->m_fpu.fpr[_a])

//-----------------------------------------------------------------------------
// extern
extern DWORD    g_labelRefCount;
extern LABELREF g_labelRef[LABEL_REF_MAX];
extern u8*      g_labelDef[LABEL_DEF_MAX];

#endif // _cpu_x86macro
