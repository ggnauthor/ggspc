                TARGET      BIN, R5900
                INCLUDE     "..\..\tools\asm5900\ps2lib\macros.inc"
IF !CRUNCHED
                DB  7Fh,"ELF"   ; indentifier
                DB  1           ; File Class: 1=32bit, 2=64bit objects
                DB  1           ; Data Encoding: 1=ELFDATA2LSB, 2=ELFDATA2MSB
                DB  1           ; ELF header version (must be 1)
                DB  0,'n','S','X','2',0,0,0,0
                DH  2           ; ELF type: 0=NONE, 1=REL, 2=EXEC, 3=SHARED, 4=CORE
                DH  8           ; Processor: 8=MIPS
                DW  1           ; Version: 1=current
                DW  code.main   ; Entry point address
                DW  program     ; Start of program headers (offset from file start)
                DW  0           ; Start of section headers (offset from file start)
                DW  20924001h   ; Processor specific flags = 0x20924001 noreorder, mips
                DH  34h         ; ELF header size (0x34 = 52 bytes)
                DH  20h         ; Program headers entry size
                DH  1           ; Number of program headers
                DH  0           ; Section headers entry size
                DH  0           ; Number of section headers
                DH  0           ; Section header stringtable index
  program:      DW  1           ; Segment type: 1=Load the segment into memory, no. of bytes specified by 0x10 and 0x14
                DW  code._fofs  ; Offset from file start to program segment.
                DW  code._ofs   ; Virtual address of the segment
                DW  code._ofs   ; Physical address of the segment
                DW  code._fsize ; Number of bytes in the file image of the segment
                DW  ((code._size+15)>>4)<<4 ; Number of bytes in the memory image of the segment
                DW  0h          ; Flags for segment
               ;DW  0h          ; Alignment. The address of 0x08 and 0x0C must fit this alignment. 0=no alignment
code            SEGMENT     0x000000
                DW          0
ELSE
code            SEGMENT     0x400000
ENDIF

main:
;*********** YOUR CODE HERE **************

  mfc0 t0, Index
  mfc0 t1, Random
  mfc0 t2, EntryLo0
  mfc0 t3, EntryLo1
  mfc0 t4, Context
  mfc0 t5, PageMask
  mfc0 t6, Wired
;  mfc0 at, C0r7    ; Reserved
  mfc0 t7, BadVaddr
  mfc0 t8, Count
  mfc0 t9, EntryHi
  mfc0 s0, Compare
  mfc0 s1, Status
  mfc0 s2, Cause
  mfc0 s3, EPC
  mfc0 s4, PRId
  mfc0 s5, Config
;  mfc0 at, LLAddr    ; Reserved
;  mfc0 at, WatchLo  ; Reserved
;  mfc0 at, WatchHi  ; Reserved
;  mfc0 at, XContext  ; Reserved
;  mfc0 at, C0r21    ; Reserved
;  mfc0 at, C0r22    ; Reserved
  mfc0 s6, C0r23    ; BadPaddr
  mfc0 s7, C0r24    ; Debug
  mfc0 a0, C0r25    ; Pref
;  mfc0 at, ECC    ; Reserved
;  mfc0 at, CacheERR  ; Reserved
  mfc0 a1, TagLo
  mfc0 a2, TagHi
  mfc0 a3, ErrorEPC
;  mfc0 at, C0r31    ; Reserved

  mtc0 v1, Index
  addi v1, 1
  mtc0 v1, Random
  addi v1, 1
  mtc0 v1, EntryLo0
  addi v1, 1
  mtc0 v1, EntryLo1
  addi v1, 1
  mtc0 v1, Context
  addi v1, 1
  mtc0 v1, PageMask
  addi v1, 1
  mtc0 v1, Wired
;  mtc0 v1, C0r7    ; Reserved
  addi v1, 1
  mtc0 v1, BadVaddr
  addi v1, 1
  mtc0 v1, Count
  addi v1, 1
  mtc0 v1, EntryHi
  addi v1, 1
  mtc0 v1, Compare
  addi v1, 1
  mtc0 v1, Status
  addi v1, 1
  mtc0 v1, Cause
  addi v1, 1
  mtc0 v1, EPC
  addi v1, 1
  mtc0 v1, PRId
  addi v1, 1
  mtc0 v1, Config
;  mtc0 v1, LLAddr    ; Reserved
;  mtc0 v1, WatchLo  ; Reserved
;  mtc0 v1, WatchHi  ; Reserved
;  mtc0 v1, XContext  ; Reserved
;  mtc0 v1, C0r21    ; Reserved
;  mtc0 v1, C0r22    ; Reserved
  addi v1, 1
  mtc0 v1, C0r23    ; BadPaddr
  addi v1, 1
  mtc0 v1, C0r24    ; Debug
  addi v1, 1
  mtc0 v1, C0r25    ; Pref
;  mtc0 v1, ECC    ; Reserved
;  mtc0 v1, CacheERR  ; Reserved
  addi v1, 1
  mtc0 v1, TagLo
  addi v1, 1
  mtc0 v1, TagHi
  addi v1, 1
  mtc0 v1, ErrorEPC
;  mtc0 v1, C0r31    ; Reserved

;*******************************************
  nop
code            ENDS
                END
