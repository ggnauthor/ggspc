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

  slt a0, t0, k0
  slt a1, t0, k1
  slt a2, t7, k0
  slt a3, t7, k1

  slt s0, t0, v0
  slt s1, t1, v0
  slt s2, t2, v0
  slt s3, t3, v0

  slt s4, t4, v0
  slt s5, t5, v0
  slt s6, t6, v0
  slt s7, t7, v0

  slt t0, t0, v1
  slt t1, t1, v1
  slt t2, t2, v1
  slt t3, t3, v1

  slt t4, t4, v1
  slt t5, t5, v1
  slt t6, t6, v1
  slt t7, t7, v1

;*******************************************
  nop
code            ENDS
                END
