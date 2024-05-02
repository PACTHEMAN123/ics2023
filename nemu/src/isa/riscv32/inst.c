/***************************************************************************************
* Copyright (c) 2014-2022 Zihao Yu, Nanjing University
*
* NEMU is licensed under Mulan PSL v2.
* You can use this software according to the terms and conditions of the Mulan PSL v2.
* You may obtain a copy of Mulan PSL v2 at:
*          http://license.coscl.org.cn/MulanPSL2
*
* THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
* EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
* MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
*
* See the Mulan PSL v2 for more details.
***************************************************************************************/

#include "local-include/reg.h"
#include <cpu/cpu.h>
#include <cpu/ifetch.h>
#include <cpu/decode.h>

#define R(i) gpr(i)
#define Mr vaddr_read
#define Mw vaddr_write

void output_ftrace(vaddr_t, vaddr_t, int);

enum {
  TYPE_I, TYPE_U, TYPE_S, TYPE_J, TYPE_R, TYPE_B,
  TYPE_N, // none
};

#define src1R() do { *src1 = R(rs1); } while (0)
#define src2R() do { *src2 = R(rs2); } while (0)
#define immI() do { *imm = SEXT(BITS(i, 31, 20), 12); } while(0)
#define immU() do { *imm = SEXT(BITS(i, 31, 12), 20) << 12; } while(0)
#define immS() do { *imm = (SEXT(BITS(i, 31, 25), 7) << 5) | BITS(i, 11, 7); } while(0)
#define immJ() do { *imm = SEXT(((BITS(i,31,31)<<20)|(BITS(i,19,12)<<12)|(BITS(i,20,20)<<11)|(BITS(i,30,25)<<5)|(BITS(i,24,21)<<1)),21); } while(0)
#define immB() do { *imm = SEXT(((BITS(i,31,31)<<12)|(BITS(i,30,25)<<5)|(BITS(i,11,8)<<1)|(BITS(i,7,7)<<11)),13); } while(0)

static void decode_operand(Decode *s, int *rd, word_t *src1, word_t *src2, word_t *imm, int type) {
  uint32_t i = s->isa.inst.val;
  int rs1 = BITS(i, 19, 15);
  int rs2 = BITS(i, 24, 20);
  *rd     = BITS(i, 11, 7);
  switch (type) {
    case TYPE_I: src1R();          immI(); break;
    case TYPE_U:                   immU(); break;
    case TYPE_S: src1R(); src2R(); immS(); break;
    case TYPE_J: 		   immJ(); break;
    case TYPE_R: src1R(); src2R();	   break;
    case TYPE_B: src1R(); src2R(); immB(); break;
  }
}

static int decode_exec(Decode *s) {
  int rd = 0;
  word_t src1 = 0, src2 = 0, imm = 0;
  s->dnpc = s->snpc;

#define INSTPAT_INST(s) ((s)->isa.inst.val)
#define INSTPAT_MATCH(s, name, type, ... /* execute body */ ) { \
  decode_operand(s, &rd, &src1, &src2, &imm, concat(TYPE_, type)); \
  __VA_ARGS__ ; \
}

  INSTPAT_START();
  INSTPAT("??????? ????? ????? ??? ????? 00101 11", auipc  , U, R(rd) = s->pc + imm);
  INSTPAT("??????? ????? ????? ??? ????? 01101 11", lui    , U, R(rd) = imm);

  INSTPAT("??????? ????? ????? 011 ????? 00100 11", sltiu  , I, R(rd) = (src1 < imm));
  INSTPAT("??????? ????? ????? 100 ????? 00000 11", lbu    , I, R(rd) = Mr(src1 + imm, 1));
  INSTPAT("??????? ????? ????? 010 ????? 00000 11", lw     , I, R(rd) = SEXT(Mr(src1 + imm, 4), 32));
  INSTPAT("??????? ????? ????? 000 ????? 11001 11", jalr   , I, R(rd) = s->pc + 4; s->dnpc = (src1 + imm)&(~1) 
#ifdef CONFIG_FTRACE 
  ;if(rd == 0 && imm == 0)output_ftrace(s->pc, s->dnpc, 0); else output_ftrace(s->pc, s->dnpc, 1) 
#endif 
  );
  INSTPAT("??????? ????? ????? 000 ????? 00100 11", addi   , I, R(rd) = src1 + imm);
  INSTPAT("010000? ????? ????? 101 ????? 00100 11", srai   , I, R(rd) = (signed)src1 >> (imm & 0x3f));
  INSTPAT("??????? ????? ????? 111 ????? 00100 11", andi   , I, R(rd) = src1 & imm);
  INSTPAT("??????? ????? ????? 100 ????? 00100 11", xori   , I, R(rd) = src1 ^ imm);
  INSTPAT("000000? ????? ????? 101 ????? 00100 11", srli   , I, R(rd) = src1 >> (imm & 0x3f));
  INSTPAT("000000? ????? ????? 001 ????? 00100 11", slli   , I, R(rd) = src1 << (imm & 0x3f));
  INSTPAT("??????? ????? ????? 001 ????? 00000 11", lh     , I, R(rd) = SEXT(Mr(src1 + imm, 2), 16));
  INSTPAT("??????? ????? ????? 101 ????? 00000 11", lhu    , I, R(rd) = Mr(src1 + imm, 2));

  INSTPAT("??????? ????? ????? 000 ????? 01000 11", sb     , S, Mw(src1 + imm, 1, src2));
  INSTPAT("??????? ????? ????? 010 ????? 01000 11", sw	   , S, Mw(src1 + imm, 4, src2));
  INSTPAT("??????? ????? ????? 001 ????? 01000 11", sh     , S, Mw(src1 + imm, 2, src2));

  INSTPAT("??????? ????? ????? ??? ????? 11011 11", jal	   , J, R(rd) = s->pc + 4; s->dnpc = s->pc + imm 
#ifdef CONFIG_FTRACE 
  ;output_ftrace(s->pc, s->dnpc, 1) 
#endif
  );

  INSTPAT("0000000 ????? ????? 000 ????? 01100 11", add    , R, R(rd) = src1 + src2);
  INSTPAT("0100000 ????? ????? 000 ????? 01100 11", sub    , R, R(rd) = src1 - src2);
  INSTPAT("0000000 ????? ????? 011 ????? 01100 11", sltu   , R, R(rd) = (src1 < src2));
  INSTPAT("0000000 ????? ????? 100 ????? 01100 11", xor    , R, R(rd) = src1 ^ src2);
  INSTPAT("0000000 ????? ????? 110 ????? 01100 11", or     , R, R(rd) = src1 | src2);
  INSTPAT("0000000 ????? ????? 001 ????? 01100 11", sll    , R, R(rd) = src1 << src2);
  INSTPAT("0000000 ????? ????? 111 ????? 01100 11", and    , R, R(rd) = src1 & src2);
  INSTPAT("0000001 ????? ????? 000 ????? 01100 11", mul    , R, R(rd) = src1 * src2);
  INSTPAT("0000001 ????? ????? 100 ????? 01100 11", div    , R, R(rd) = (signed)src1 / (signed)src2);
  INSTPAT("0000001 ????? ????? 110 ????? 01100 11", rem    , R, R(rd) = (signed)src1 % (signed)src2);
  INSTPAT("0000000 ????? ????? 010 ????? 01100 11", slt    , R, R(rd) = (signed)src1 < (signed)src2);
  INSTPAT("0000001 ????? ????? 001 ????? 01100 11", mulh   , R, R(rd) = ((((int64_t)(int32_t)src1) * ((int64_t)(int32_t)src2))>>32));
  INSTPAT("0000001 ????? ????? 111 ????? 01100 11", remu   , R, R(rd) = src1 % src2);
  INSTPAT("0000001 ????? ????? 101 ????? 01100 11", divu   , R, R(rd) = src1 / src2);
  INSTPAT("0100000 ????? ????? 101 ????? 01100 11", sra    , R, R(rd) = (int32_t)src1 >> (src2 & 0x1f));
  INSTPAT("0000000 ????? ????? 101 ????? 01100 11", srl    , R, R(rd) = src1 >> (src2 & 0x1f));

  INSTPAT("??????? ????? ????? 000 ????? 11000 11", beq    , B, if(src1 == src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 001 ????? 11000 11", bne    , B, if(src1 != src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 101 ????? 11000 11", bge    , B, if((signed)src1 >= (signed)src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 111 ????? 11000 11", bgeu   , B, if(src1 >= src2) s->dnpc = s->pc +imm);
  INSTPAT("??????? ????? ????? 100 ????? 11000 11", blt    , B, if((signed)src1 < (signed)src2) s->dnpc = s->pc + imm);
  INSTPAT("??????? ????? ????? 110 ????? 11000 11", bltu   , B, if(src1 < src2) s->dnpc = s->pc + imm);

  INSTPAT("0000000 00001 00000 000 00000 11100 11", ebreak , N, NEMUTRAP(s->pc, R(10))); // R(10) is $a0
  INSTPAT("??????? ????? ????? ??? ????? ????? ??", inv    , N, INV(s->pc));
  INSTPAT_END();

  R(0) = 0; // reset $zero to 0

  return 0;
}

int isa_exec_once(Decode *s) {
  s->isa.inst.val = inst_fetch(&s->snpc, 4);
  return decode_exec(s);
}


/* ftrace */
#include <elf.h>
typedef struct {
  uint32_t size;
  char name[32];
  uint32_t value;
}FUNC;

FUNC func[32];
int findex = 0;

void init_ftrace(const char *elf) {
  if(elf != NULL) {
    
    FILE *fp = fopen(elf, "rb");
    Assert(fp, "Can not open '%s'", elf);
    uint32_t symtab_size = 0;
    uint32_t symtab_entsize = 0;
    Elf32_Off symtab_offset = 0;
    Elf32_Off strtab_offset = 0;

    /* access elf header */
    Elf32_Ehdr ehdr;
    size_t ret = fread(&ehdr, sizeof(ehdr), 1, fp);

    /* access section header */
    Elf32_Shdr shdr;
    for(int i = 0; i < ehdr.e_shnum ; i++) {
      fseek(fp, ehdr.e_shoff + i * ehdr.e_shentsize, SEEK_SET);
      ret = fread(&shdr, sizeof(shdr), 1, fp);
      /* symbol table and string table */
      if(shdr.sh_type == SHT_SYMTAB){
        symtab_offset = shdr.sh_offset;
        symtab_size = shdr.sh_size;
        symtab_entsize = shdr.sh_entsize;
        uint32_t strtab_index = shdr.sh_link;
        fseek(fp, ehdr.e_shoff + strtab_index * ehdr.e_shentsize, SEEK_SET);
        ret = fread(&shdr, sizeof(shdr), 1, fp);
        strtab_offset = shdr.sh_offset;
      }
    }

    /* scan the symbol table */
    Elf32_Sym symtab;
    for(int i = 0; i * symtab_entsize < symtab_size ; i++) {
      fseek(fp, symtab_offset + i * symtab_entsize, SEEK_SET);
      ret = fread(&symtab, sizeof(symtab), 1, fp);
      //Log("size : %d,value : %x",symtab.st_size, symtab.st_value);
      if((0xf & symtab.st_info) == STT_FUNC){
        func[findex].size = symtab.st_size;
        func[findex].value = symtab.st_value;
        fseek(fp, strtab_offset + symtab.st_name, SEEK_SET);
        /* get the name */
        ret = (size_t)fgets(func[findex].name, 32, fp);
        Log("[%d] %s: size: %d, value: %#x", findex, func[findex].name, func[findex].size, func[findex].value); 
        findex ++;
      }
    }

    Log("ret : %d", (int)ret);
    //Log("section header offset: %#x", ehdr.e_shoff);
    //Log("symbol table offset: %#x, entsize: %#x, size: %#x", symtab_offset, symtab_entsize, symtab_size);
    //Log("string table offset: %#x", strtab_offset);
    fclose(fp);
  } 
  Log("Ftrace is on, reading %s", elf); 
} 

/* display ftrace by getting address */
void output_ftrace(vaddr_t pc, vaddr_t dnpc, int sign) {
  for(int i = 0; i < findex; i++) {
    if((dnpc >= func[i].value) && (dnpc < func[i].value + func[i].size)){
      if(sign)Log("%#8x: call [%s@%#8x]", pc, func[i].name, dnpc);
      else Log("%#8x: ret [%s]", pc, func[i].name);
    } 
  }
}


