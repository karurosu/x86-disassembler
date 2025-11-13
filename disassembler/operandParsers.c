//
//  operandParsers.c
//  disassembler
//
//  Created by karurosu on 10/3/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "dis.h"

const char address_methods_strings[][5] = {"NONE","A","BA","BB","BD","C","D","E","ES","EST","F","G","H","I","J","M","N","O","P","Q","R","S","SC","T","U","V","W","X","Y","Z", "S2", "S30", "S33", "rAX"};

bool parseOperandError(IN struct dis_section *section, IN struct dis_opcode *instruction, IN int index, IN dis_address_methods mode) {
    printf("Mode is not supported: %s\n",address_methods_strings[mode]);
    return false;
}

bool parseOperandZ(IN struct dis_section *section, IN struct dis_opcode *instruction, IN int index, IN dis_address_methods mode) {
    //read opcode
    unsigned char reg = *(((char *)section->buffer)+instruction->opcode_offset+instruction->offset);
    const char regs[][4] = {"eax", "ecx", "edx", "ebx", "esp", "ebp", "esi", "edi"};
    reg &= 7;
    instruction->operands[index].type = REGISTER_OPERAND;
    strcpy(instruction->operands[index].representation, regs[reg]);
    return true;
}

parseOperandFunction parseOperandFunctions[34] = {
    &parseOperandError,//0 NONE
    &parseOperandError,//1 A
    &parseOperandError,//2 BA
    &parseOperandError,//3 BB
    &parseOperandError,//4 BD
    &parseOperandError,//5 C
    &parseOperandError,//6 D
    &parseOperandError,//7 E
    &parseOperandError,//8 ES
    &parseOperandError,//9 EST
    &parseOperandError,//10 F
    &parseOperandError,//11 G
    &parseOperandError,//12 H
    &parseOperandError,//13 I
    &parseOperandError,//14 J
    &parseOperandError,//15 M
    &parseOperandError,//16 N
    &parseOperandError,//17 O
    &parseOperandError,//18 P
    &parseOperandError,//19 Q
    &parseOperandError,//20 R
    &parseOperandError,//21 S
    &parseOperandError,//22 SC
    &parseOperandError,//23 T
    &parseOperandError,//24 U
    &parseOperandError,//25 V
    &parseOperandError,//26 W
    &parseOperandError,//27 X
    &parseOperandError,//28 Y
    &parseOperandZ,//29 Z
    &parseOperandError,//30 S2
    &parseOperandError,//31 S30
    &parseOperandError,//32 S33
    &parseOperandError //33 rAX
};