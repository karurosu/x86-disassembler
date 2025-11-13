//
//  Binary.h
//  disassembler
//
//  Created by karurosu on 9/8/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "base.h"
#include "dis_portability.h"
#include "opdb.h"

#ifndef disassembler_Binary_h
#define disassembler_Binary_h

typedef struct dis_parser dis_parser;
typedef struct dis_binary dis_binary;
typedef struct dis_section dis_section;
typedef struct dis_printer dis_printer;
typedef struct dis_opcode dis_opcode;
typedef struct dis_disassembler dis_dissassembler;

struct dis_opcode_operand;

typedef bool (*sectionCleanFunction)(IN struct dis_section *);
typedef bool (*sectionReadCharFunction)(IN struct dis_section *, OUT unsigned char **);
typedef bool (*sectionReadShortFunction)(IN struct dis_section *, OUT unsigned short **);
typedef bool (*sectionReadIntFunction)(IN struct dis_section *, OUT unsigned int **);
typedef bool (*sectionReadMemFunction)(IN struct dis_section *, OUT void **, IN int);

bool defaultSectionReadChar(IN struct dis_section *, OUT unsigned char **);
bool defaultSectionReadShort(IN struct dis_section *, OUT unsigned short **);
bool defaultSectionReadInt(IN struct dis_section *, OUT unsigned int **);
bool defaultSectionReadMem(IN struct dis_section *, OUT void **, IN int);


struct dis_section {
    /* data members */
    char *name;
    bool codeSection;
    unsigned long size;
    void *buffer;
    unsigned long cursor;
    apr_pool_t *pool;
    
    /* function members */
    sectionCleanFunction clean;
    sectionReadCharFunction readChar;
    sectionReadIntFunction readInt;
    sectionReadShortFunction readShort;
    sectionReadMemFunction readMem;
};

typedef bool (*binaryCleanFunction)(IN struct dis_binary *);
typedef bool (*nextSectionFunction)(IN struct dis_binary *, OUT dis_section *section, IN apr_pool_t *parentPool);

struct dis_binary{
    /* data members */
    char *type;
    char *fileName;
    apr_pool_t *pool;
    int fd;
    
    /* function members */
    binaryCleanFunction clean;
    nextSectionFunction nextSection;
};

typedef bool (*parserParseFunction)(IN struct dis_parser *, IN char *, IN apr_pool_t *, OUT dis_binary **);
typedef bool (*parserCanParseFunction)(IN struct dis_parser *, IN char *);
typedef bool (*parserInitializeFunction)(IN struct dis_parser *);

struct dis_parser {
    /* data members */
    char *name;
    
    /* function members */
    parserCanParseFunction canParse;
    parserInitializeFunction initialize;
    parserParseFunction parse;
};

typedef enum operand_type {
    REGISTER_OPERAND,
    DATA_OPERAND,
    ADDRESS_OPERAND
} operand_type;

struct dis_opcode_operand {
    operand_type type;
    char representation[10];
};

struct dis_opcode{
    /* information about the instruction */
    dis_section *section;
    unsigned long offset;
    unsigned char size;
    unsigned char opcode_offset;
    unsigned char modrm_offset;
    struct opdb_entry *opcode_data;
    /* 16/32-bit mode settings */
    unsigned char addr_size;        /* default address size : 2 or 4 */
    unsigned char op_size;          /* default operand size : 2 or 4 */
    unsigned char prefixes[4];
    struct dis_opcode_operand operands[4];
    size_t operand_count;
};

typedef enum dis_address_methods {
    NO_METHOD = 0, A,BA,BB,BD,C,D,E,ES,EST,F,G,H,I,J,M,N,O,P,Q,R,S,SC,T,U,V,W,X,Y,Z, S2, S30, S33, rAX
} dis_address_methods;

typedef enum operand_types {
    NO_TYPE = 0, a, b, bcd, bs, bsq, bss, c, d, di, dq, dqp, dr, ds, e, er, p, pi, pd, ps, psq, pt, ptp, q, qi, qp, s, sd, si, sr, ss, st, stx, t, v, vds, vq, vqp, vs, w, wi
} operand_types;

typedef bool (*parseOperandFunction)(IN struct dis_section *, IN struct dis_opcode *, IN int, IN dis_address_methods);

extern parseOperandFunction parseOperandFunctions[34];

typedef bool (*printerPrintSectionFunction)(IN struct dis_printer *, IN dis_section*);
typedef bool (*printerPrintOpcodeFunction)(IN struct dis_printer *, IN dis_opcode*);
typedef bool (*printerPrinterInitializeFunction)(IN struct dis_printer *, IN opdb *);

struct dis_printer {
    opdb *db;
    
    printerPrinterInitializeFunction initialize;
    printerPrintOpcodeFunction printOpcode;
    printerPrintSectionFunction printSection;
};

typedef bool (*disassemblerInitializeFunction)(IN struct dis_disassembler *, IN apr_pool_t *, IN dis_printer *, IN opdb *);
typedef bool (*disassemblerCleanupFunction)(IN struct dis_disassembler *);
typedef bool (*disassemblerDisassembleSectionFunction)(IN struct dis_disassembler *, IN dis_section *);
typedef bool (*disassemblerDisassembleNextFunction)(IN struct dis_disassembler *, IN dis_section *);

struct dis_disassembler {
    apr_pool_t *pool;
    dis_printer *printer;
    opdb *db;
    
    /*Functional methods */
    disassemblerInitializeFunction initialize;
    disassemblerCleanupFunction clean;
    disassemblerDisassembleSectionFunction disassembleSection;
    disassemblerDisassembleNextFunction disassembleNext;
};

extern dis_dissassembler defaultDisassembler;

#endif
