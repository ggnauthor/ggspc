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

  bgtzl s0, label1  ;ジャンプしない
  addi a0, zero, 1  ;実行されない
  j label2
  nop
label1:
  addi t0, zero, 1
label2:
  bgtzl s1, label3  ;ジャンプしない
  addi a1, zero, 1  ;実行されない
  j label4
  nop
label3:
  addi t1, zero, 1
label4:
  bgtzl s2, label5  ;ジャンプしない
  addi a2, zero, 1  ;実行されない
  j label6
  nop
label5:
  addi t2, zero, 1
label6:
  bgtzl s3, label7  ;ジャンプする
  addi a3, zero, 1  ;実行される
  j label8
  nop
label7:
  addi t3, zero, 1
label8:
  
  ;上方ジャンプのテスト
  addi at, zero, 2
loop:
  bgtzl at, loop
  addi at, at, -1    ;最後は実行されないのでat==0

;*******************************************
  nop
code            ENDS
                END
