//
//  printer_stdout.c
//  disassembler
//
//  Created by karurosu on 9/18/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "printer_stdout.h"


bool stdout_printSection(IN struct dis_printer *self, IN dis_section *section) {
    printf("\nSection %s\n", section->name);
    if(section->codeSection){
        printf("Code: YES\n");
    }
    else {
        printf("Code: NO\n");
    }
    return true;
}

bool stdout_printOpcode(IN struct dis_printer *self, IN dis_opcode *opcode) {
    int i = 0;
    unsigned char *begin = ((unsigned char *) opcode->section->buffer) + opcode->offset;
    //Print address
    printf("%08ld:\t", opcode->offset);
    //Print dump:
    for (i = 0; i < opcode->size; i++) {
        printf("%1x ", *begin);
        begin++;
    }
    for (i = opcode->size; i < 8; i++)
    {
        printf("   ");
    }
    
    //print prefixes
    
    //print mnemonic
    printf("%s   ",opcode->opcode_data->mnemonic);
    
    //print operands
    for (i = 0; i < opcode->operand_count; i++) {
        if (opcode->operands[i].type == REGISTER_OPERAND){
            printf(" %%%s ", opcode->operands[i].representation);
        }
    }
    
    printf("\n");
    return true;
}

bool stdout_initialize(IN struct dis_printer *self, opdb *db) {
    self->db = db;
    return true;
}

dis_printer stdOutPrinter = {.initialize = &stdout_initialize, .printOpcode = &stdout_printOpcode, .printSection= &stdout_printSection};