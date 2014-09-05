#include "elf.h"

#include "main.h"

#if defined _DEBUGGER
#include "debugger/debugger.h"
#include "debugger/source_info.h"
#endif

#include <utils/ds_util.h>

#define _CRTDBG_MAP_ALLOC
#include <crtdbg.h>
#include "debug_malloc.h"

namespace {

enum {
  ELFCLASSNONE = 0, // Invalid class
  ELFCLASS32   = 1, // 32-bit objects
  ELFCLASS64   = 2, // 64-bit objects
};

enum {
  ELFDATANONE = 0, // Invalid data encoding
  ELFDATA2LSB = 1, // Little Endian
  ELFDATA2MSB = 2, // Big Endian
};

enum {
  ET_NONE = 0, // 未知のタイプ
  ET_REL,      // 再配置可能なファイル
  ET_EXEC,     // 実行可能ファイル
  ET_DYN,      // 共有オブジェクト
  ET_CORE,     // コアファイル
};

enum {
  EM_NONE   = 0,  // Invalid Machine
  EM_M32    = 1,  // AT&T WE 32100
  EM_SPARC  = 2,  // SPARC
  EM_386    = 3,  // Intel 80386
  EM_68K    = 4,  // Motorola 68000
  EM_88K    = 5,  // Motorola 88000
  EM_486    = 6,  // Intel 80486
  EM_860    = 7,  // Intel 80860
  EM_MIPS   = 8,  // MIPS R3000 Big-Endian only
  EM_PPC    = 20, // PowerPC
  EM_IA_64  = 50, // HP/Intel IA-64
  EM_X86_64 = 62, // AMD x86-64
};

enum {
  PT_NULL = 0,
  PT_LOAD,
  PT_DYNAMIC,
  PT_INTERP,
  PT_NOTE,
  PT_SHLIB,
  PT_PHDR,
  PT_LOPROC = 0x70000000,
  PT_HIPROC = 0x7fffffff,
};

enum {
  PF_X = 1,
  PF_W = 2,
  PF_R = 4,
};

enum {
  SHT_NULL = 0,
  SHT_PROGBITS,
  SHT_SYMTAB,
  SHT_STRTAB,
  SHT_RELA,
  SHT_HASH,
  SHT_DYNAMIC,
  SHT_NOTE,
  SHT_NOBITS,
  SHT_REL,
  SHT_SHLIB,
  SHT_DYNSYM,
};

enum {
  SHF_WRITE     = 0x00000001,
  SHF_ALLOC     = 0x00000002,
  SHF_EXECINSTR = 0x00000004,
  SHF_MASKPROC  = 0xf0000000,
};

const char* SHT_Labels[] = {
  "null",   "progbits", "symtab",  "strtab",
  "rela",   "hash",     "dynamic", "note",
  "nobits", "rel",      "shlib",   "dynsym",
};

enum {
  DW_TAG_null                   = 0x00,
  DW_TAG_array_type             = 0x01,
  DW_TAG_class_type             = 0x02,
  DW_TAG_entry_point            = 0x03,
  DW_TAG_enumeration_type       = 0x04,
  DW_TAG_formal_parameter       = 0x05,
  DW_TAG_imported_declaration   = 0x08,
  DW_TAG_label                  = 0x0a,
  DW_TAG_lexical_block          = 0x0b,
  DW_TAG_member                 = 0x0d,
  DW_TAG_pointer_type           = 0x0f,
  DW_TAG_reference_type         = 0x10,
  DW_TAG_compile_unit           = 0x11,
  DW_TAG_string_type            = 0x12,
  DW_TAG_structure_type         = 0x13,
  DW_TAG_subroutine_type        = 0x15,
  DW_TAG_typedef                = 0x16,
  DW_TAG_union_type             = 0x17,
  DW_TAG_unspecified_parameters = 0x18,
  DW_TAG_variant                = 0x19,
  DW_TAG_common_block           = 0x1a,
  DW_TAG_common_inclusion       = 0x1b,
  DW_TAG_inheritance            = 0x1c,
  DW_TAG_inlined_subroutine     = 0x1d,
  DW_TAG_module                 = 0x1e,
  DW_TAG_ptr_to_member_type     = 0x1f,
  DW_TAG_set_type               = 0x20,
  DW_TAG_subrange_type          = 0x21,
  DW_TAG_with_stmt              = 0x22,
  DW_TAG_access_declaration     = 0x23,
  DW_TAG_base_type              = 0x24,
  DW_TAG_catch_block            = 0x25,
  DW_TAG_const_type             = 0x26,
  DW_TAG_constant               = 0x27,
  DW_TAG_enumerator             = 0x28,
  DW_TAG_file_type              = 0x29,
};

// line Standard Opcodes
enum {
  DW_LNS_exop = 0,
  DW_LNS_copy,
  DW_LNS_advance_pc,
  DW_LNS_advance_line,
  DW_LNS_set_file,
  DW_LNS_set_column,
  DW_LNS_negate_stmt,
  DW_LNS_set_basic_block,
  DW_LNS_const_add_pc,
  DW_LNS_fixed_advance_pc,
  DW_LNS_advance_address,
};

// line Extended Opcodes
enum {
  DW_LNE_end_sequence  = 1,
  DW_LNE_set_address,
  DW_LNE_define_file,
};

#pragma pack(push,1)

typedef struct {
  u8  e_ident[16]; // ELF Identification
  u16 e_type;      // object file type
  u16 e_machine;   // machine
  u32 e_version;   // object file version
  u32 e_entry;     // virtual entry point
  u32 e_phoff;     // program header table offset
  u32 e_shoff;     // section header table offset
  u32 e_flags;     // processor-specific flags
  u16 e_ehsize;    // ELF header size
  u16 e_phentsize; // program header entry size
  u16 e_phnum;     // number of program header entries
  u16 e_shentsize; // section header entry size
  u16 e_shnum;     // number of section header entries
  u16 e_shstrndx;  // section header table's 
                   // "section header string table" entry offset
} ELF32_HDR;

typedef struct {
  u32 p_type;   // segment type
  u32 p_offset; // segment offset
  u32 p_vaddr;  // virtual address of segment
  u32 p_paddr;  // physical address - ignored?
  u32 p_filesz; // number of bytes in file for seg.
  u32 p_memsz;  // number of bytes in mem. for seg.
  u32 p_flags;  // flags
  u32 p_align;  // memory alignment
} ELF32_PHDR;

typedef struct {
  u32 sh_name;      // name - index into section header string table section
  u32 sh_type;      // type
  u32 sh_flags;     // flags
  u32 sh_addr;      // address
  u32 sh_offset;    // file offset
  u32 sh_size;      // section size
  u32 sh_link;      // section header table index link
  u32 sh_info;      // extra information
  u32 sh_addralign; // address alignment
  u32 sh_entsize;   // section entry size
} ELF32_SHDR;

typedef struct {
  u32 sym_name;
  u32 sym_value;
  u32 sym_size;
  u8  sym_info;
  u8  sym_other;
  u16 sym_shndx;
} ELF32_SYM;

typedef struct {
  u32 length;
  u16 version;
  u32 abbrev_offset;
  u8  pointer_size;
} DWARF2_CUHeader;

typedef struct {
  u32 stmt_list;
  u32 high_pc;
  u32 low_pc;
  u32 name_ofs;
  u32 comp_dir_ofs;
  u32 producer_ofs;
  u8  language;
} DWARF2_CU_compile_unit;

typedef struct {
  u32 sibling;
  u32 name_ofs;
  u8  byte_size;
  u8  decl_file;
  u8  decl_line;
} DWARF2_CU_enumeration_type;

typedef struct {
  u32 name_ofs;
  u8  const_value;
} DWARF2_CU_enumerator;

typedef struct {
  u32 name_ofs;
  u8  decl_file;
  u8  decl_line;
  u32 type;
  u32 data_member_location;
} DWARF2_CU_member;

typedef struct {
  u32 length;
  u16 version;
  u32 prologue_len;
  u8  min_inst_len;
  u8  init_val_stmt;
  s8  line_base;
  u8  line_range;
  u8  opcode_base;
} DWARF2_LineHeader;

#pragma pack(pop)

} // namespace

u32 Elf::load(char* p_fname, u32* p_pgm_head,
              u32* p_pgm_size, u8* p_buf, u32 p_buf_size) {
  FILE* fp = fopen(p_fname, "rb");
  if (fp == NULL) return 0xffffffff;

  u32 elf_size = ds_util::fsize32(fp);
  u8* elf_data = (u8*)malloc(elf_size);

  fread(elf_data, elf_size, 1, fp);
  fclose(fp);

  ELF32_HDR*  hdr  = (ELF32_HDR*)elf_data;
  ELF32_PHDR* phdr = (ELF32_PHDR*)&elf_data[hdr->e_phoff];
  ELF32_SHDR* shdr = (ELF32_SHDR*)&elf_data[hdr->e_shoff];

  assert(hdr->e_ident[4] == ELFCLASS32);
  assert(hdr->e_ident[5] == ELFDATA2LSB);
  //assert(hdr->e_type == ET_EXEC); // iopでは0xff80と定義されている
  assert(hdr->e_machine == EM_MIPS);

  DBGOUT_ELF("elf entry:%08x flag:%08x\n",
    hdr->e_entry, hdr->e_flags, hdr->e_phnum, hdr->e_shnum);
  DBGOUT_ELF("    ph:%08x[%08x](%d)\n",
    hdr->e_phoff, hdr->e_phentsize, hdr->e_phnum);
  DBGOUT_ELF("    sh:%08x[%08x](%d)\n",
    hdr->e_shoff, hdr->e_shentsize, hdr->e_shnum);

  *p_pgm_size = 0;

  // Load Program
  //memset(p_buf, 0, p_bufSize);
  for (int i = 0; i < hdr->e_phnum; i++) {
    if (phdr[i].p_type != PT_LOAD) continue;
    if (phdr[i].p_filesz <= 0) continue;

    //if (*p_pgmsize == 0 && (phdr[i].p_flags & PF_X))
    if (*p_pgm_size == 0) {
      *p_pgm_head = phdr[i].p_vaddr & 0x1ffffff;
      *p_pgm_size = phdr[i].p_filesz;
    }

    memcpy(&p_buf[phdr[i].p_vaddr & 0x1ffffff],
      &elf_data[phdr[i].p_offset],
      phdr[i].p_filesz);
    DBGOUT_ELF( "program%d %08x[%08x] flags:%08x\n",
      i, phdr[i].p_vaddr, phdr[i].p_memsz, phdr[i].p_flags);
  }

  // Load Section
  int shidx_sym = -1;
  int shidx_str = -1;
  int shidx_debug_line = -1;
  int shidx_ro = -1;

  char* section_names = (char*)&elf_data[shdr[hdr->e_shstrndx].sh_offset];
  for (int i = 0; i < hdr->e_shnum; i++) {
    if (shdr[i].sh_size <= 0) continue;
    
    DBGOUT_ELF("section%d %08x[%08x] %s %s\n",
      i, shdr[i].sh_addr, shdr[i].sh_size, &section_names[shdr[i].sh_name],
      (shdr[i].sh_type <= 11) ? SHT_Labels[shdr[i].sh_type] : "***");

    //if ((shdr[i].sh_type & 0xff) == SHT_SYMTAB) shidx_sym = i;
    //  else if ((shdr[i].sh_type & 0xff) == SHT_STRTAB) shidx_str = i;

    if (strcmp(&section_names[shdr[i].sh_name], ".symtab") == 0)
      shidx_sym = i;
    if (strcmp(&section_names[shdr[i].sh_name], ".strtab") == 0)
      shidx_str = i;
    if (strcmp(&section_names[shdr[i].sh_name], ".rodata") == 0)
      shidx_ro = i;
    if (strcmp(&section_names[shdr[i].sh_name], ".debug_line") == 0)
      shidx_debug_line = i;

    if (shdr[i].sh_addr != 0x00000000 &&
        shdr[i].sh_type != 1 && shdr[i].sh_type != 8) {
      memcpy(&p_buf[shdr[i].sh_addr & 0x1ffffff],
             &elf_data[shdr[i].sh_offset],
             shdr[i].sh_size);
    }
  }

#if defined _DEBUGGER
  if (g_dbg) {
    // デバッガクラスにシンボル情報を構築
    if (shidx_sym != -1 && shidx_str != -1) {
      for (int j = 0; j < (int)(shdr[shidx_sym].sh_size / sizeof(ELF32_SYM)); j++) {
        ELF32_SYM* sym_tab = (ELF32_SYM*)(&elf_data[shdr[shidx_sym].sh_offset] + shdr[shidx_sym].sh_entsize * j);

        s8* name = (s8*)&elf_data[shdr[shidx_str].sh_offset + sym_tab->sym_name];
        if (name[0] != '\0') {
          DBGOUT_SYM("  %03d addr=%08x name=%s\n", j, sym_tab->sym_value, name);
          Symbol* symbol = new Symbol(sym_tab->sym_value, name);
          g_dbg->symbol_map_.insert(SymbolMapValue(sym_tab->sym_value, symbol));
        }
      }
    }

    // デバッガクラスにデバッグ情報を構築
    if (shidx_debug_line != -1) {
      u32  pos = shdr[shidx_debug_line].sh_offset;
      while (pos < shdr[shidx_debug_line].sh_offset + shdr[shidx_debug_line].sh_size) {
        DWARF2_LineHeader* line_hdr = (DWARF2_LineHeader*)&elf_data[pos];
        
        pos += sizeof(DWARF2_LineHeader) + line_hdr->opcode_base - 1;

        // Directory Table
        vector<u8*> tmp_dir_tab;
        while (elf_data[pos] != '\0') {
          tmp_dir_tab.push_back(&elf_data[pos]);
          pos += strlen((char*)&elf_data[pos]) + 1;
        }
        pos++;

        // File Name Table
        int fname_tab_head_idx = g_dbg->src_file_ary_.size();
        while (elf_data[pos] != '\0') {
          char* fname = (char*)&elf_data[pos];
          pos += strlen((char*)&elf_data[pos]) + 1;
          int dirID = elf_data[pos++];
          pos += 2;

          if (dirID > 0) {
            char str[1024];
            sprintf(str, "%s/%s", tmp_dir_tab[dirID - 1], fname);
            g_dbg->src_file_ary_.push_back(new SourceFileEntry(str, strlen((char*)tmp_dir_tab[dirID - 1]) + 1));
          } else {
            g_dbg->src_file_ary_.push_back(new SourceFileEntry(fname, 0));
          }
        }
        pos++;

        DBGOUT_SYM("[File Name Table]\n");
        for (int i = fname_tab_head_idx; i < static_cast<int>(g_dbg->src_file_ary_.size()); i++) {
          DBGOUT_SYM("%2d:%s\n", i - fname_tab_head_idx, g_dbg->src_file_ary_[i]->path);
        }

        // File Location Table
        if (fname_tab_head_idx < static_cast<int>(g_dbg->src_file_ary_.size())) {  // ファイルが無い場合は無視
          int file = fname_tab_head_idx;
          int line = 1;
          u32 addr = 0x00000000;
          bool end_seq = false;
          while (!end_seq) {
            u8 opcode = elf_data[pos++];
            if (opcode >= line_hdr->opcode_base) {
              // special opcode
              addr += (opcode - line_hdr->opcode_base) / line_hdr->line_range * line_hdr->min_inst_len;
              line += (opcode - line_hdr->opcode_base) % line_hdr->line_range + line_hdr->line_base;
              DBGOUT_SYM("add entry addr=%08x file=%s:%d\n", addr, g_dbg->src_file_ary_[file]->name, line);
              g_dbg->src_loc_ary_addr_.push_back(new SourceLocationEntry(addr, file, line));
            } else {
              switch (opcode) {
                case DW_LNS_exop: {
                  u8 len = elf_data[pos++];
                  u8 exop = elf_data[pos++];
                  switch (exop) {
                    case DW_LNE_end_sequence:
                      DBGOUT_SYM("end sequence addr=%08x\n", addr);
                      // ファイルの終端として登録
                      g_dbg->src_loc_ary_addr_.push_back(new SourceLocationEntry(addr, file, 0x7fffffff));
                      end_seq = true;
                      break;
                    case DW_LNE_set_address:
                      addr = *((u32*)&elf_data[pos]);
                      pos += 4;
                      DBGOUT_SYM("set address %08x\n", addr);
                      break;
                    case DW_LNE_define_file:
                      DBGOUT_SYM("define file\n", addr);
                      break;
                    default:
                      DBGOUT_SYM("unknown exop %02x\n", exop);
                      break;
                  }
                } break;
                case DW_LNS_copy:
                  DBGOUT_SYM("add entry addr=%08x file=%s:%d\n", addr, g_dbg->src_file_ary_[file]->name, line);
                  g_dbg->src_loc_ary_addr_.push_back(new SourceLocationEntry(addr, file, line));
                  break;
                case DW_LNS_advance_pc: {
                  int value = decULEB128(&elf_data[pos], (int*)&pos);
                  addr += value * line_hdr->min_inst_len;
                  DBGOUT_SYM("advance pc addr=%08x\n", addr);
                } break;
                case DW_LNS_advance_line: {
                  int value = decSLEB128(&elf_data[pos], (int*)&pos);
                  line += value;
                  DBGOUT_SYM("advance line val=%d line=%d\n", value, line);
                } break;
                case DW_LNS_set_file:
                  file = decULEB128(&elf_data[pos], (int*)&pos) + fname_tab_head_idx - 1;
                  DBGOUT_SYM("set file %d\n", file);
                  break; 
                case DW_LNS_set_column:
                  DBGOUT_SYM("set column (not implemented)\n");
                  break;
                case DW_LNS_negate_stmt:
                  DBGOUT_SYM("negate stmt (not implemented)\n");
                  break;
                case DW_LNS_set_basic_block:
                  DBGOUT_SYM("set basic block (not implemented)\n");
                  break;
                case DW_LNS_const_add_pc:
                  addr += (255 - line_hdr->opcode_base) / line_hdr->line_range * line_hdr->min_inst_len;
                  DBGOUT_SYM("const add pc %08x\n", addr);
                  break;
                case DW_LNS_fixed_advance_pc:
                  DBGOUT_SYM("fixed advance pc (not implemented)\n");
                  break;
              }
            }
          }
        }
        DBGOUT_SYM("---------------------------------------------\n");
      }

      // アセンブラからソースを参照する場合はアドレスでソートしてあると便利
      qsort(&g_dbg->src_loc_ary_addr_[0],
        static_cast<int>(g_dbg->src_loc_ary_addr_.size()),
        sizeof(SourceLocationEntry*),
        sortSourceLocationEntryByAddr);

      // ソースからアセンブラを参照する場合はファイル、行でソートしてあると便利
      g_dbg->src_loc_ary_line_ = g_dbg->src_loc_ary_addr_;
      qsort(&g_dbg->src_loc_ary_line_[0],
        static_cast<int>(g_dbg->src_loc_ary_line_.size()),
        sizeof(SourceLocationEntry*),
        sortSourceLocationEntryByNameLine);
    }

    // モジュール内から文字列を探索
    // 連続した文字を文字列と見なしているだけなので誤検出や見落しがあるのは仕方ない
    if (shidx_ro > 0) {
      u8* ptr = &elf_data[shdr[shidx_ro].sh_offset];

      // ASCII文字の先頭を探索
      char* str = NULL;
      for (u32 j = 0; j < shdr[shidx_ro].sh_size; j++) {
        if (str != NULL && ptr[j] == '\0') {
          int offset = (u8*)str - ptr;
          if (j - offset >= 4) { // そのままではノイズを拾いすぎるので4文字以上を対象とする
            String* string = new String(shdr[shidx_ro].sh_addr + offset, str);
            g_dbg->string_map_.insert(StringMapValue(string->va(), string));
          }
          str = NULL;
        }

        bool ascii = IS_ASCII(ptr[j]);
        if (ascii || ptr[j] == '\r' || ptr[j] == '\n' || ptr[j] == '\t') {
          if (str == NULL) {
            str = (char*)&ptr[j];
          }
        } else str = NULL;
      }
    }
  }
  DBGOUT_ELF("---------------------------------------------\n");
#endif

  u32 entry_point = hdr->e_entry;
  free(elf_data);

  return entry_point;
}

#if defined _DEBUGGER

int Elf::decSLEB128(u8* p_data, int* p_bc) {
  // 7bit目は継続ビット
  // 6bit目は符号ビット
  // 5-0bitで-64～63までの値を扱い、
  // それ以上は次バイトにてx128スケールで指定される
  u8*  ptr = p_data;
  int result = 0;
  int weight = 1;
  while (1) {
    if (*ptr & 0x40) {
      result -= (0x80 - *ptr) * weight;
    } else {
      result += (*ptr & 0x3f) * weight;
    }
    if (p_bc) (*p_bc)++;
    if ((*ptr & 0x80) == 0) break;
    ptr++;
    weight *= 128;    
  }
  return result;
}

int Elf::decULEB128(u8* p_data, int* p_bc) {
  // 7bit目は継続ビット
  // 6-0bitで0～127までの値を扱い、
  // それ以上は次バイトにてx128スケールで指定される
  u8*  ptr = p_data;
  int result = 0;
  int weight = 1;
  while (1) {
    result += (*ptr & 0x7f) * weight;
    if (p_bc) (*p_bc)++;
    if ((*ptr & 0x80) == 0) break;
    ptr++;
    weight *= 128;    
  }
  return result;
}

int Elf::sortSourceLocationEntryByAddr(const void* p_a, const void* p_b) {
  SourceLocationEntry* slea = *((SourceLocationEntry**)p_a);
  SourceLocationEntry* sleb = *((SourceLocationEntry**)p_b);

  // ２つのファイルの終端と始点が重複する場合、終点が先に来るように
  if (slea->addr == sleb->addr) {
    if (slea->line == 0x7fffffff) return -1;
    if (sleb->line == 0x7fffffff) return  1;
  }
  // アドレス
  return slea->addr - sleb->addr;
}

int Elf::sortSourceLocationEntryByNameLine(const void* p_a, const void* p_b) {
  SourceLocationEntry* slea = *((SourceLocationEntry**)p_a);
  SourceLocationEntry* sleb = *((SourceLocationEntry**)p_b);

  if (slea->addr == sleb->addr) {
    if (slea->line == 0x7fffffff) return -1;
    if (sleb->line == 0x7fffffff) return  1;
  }
  // 行
  return slea->line - sleb->line;
}
#endif
