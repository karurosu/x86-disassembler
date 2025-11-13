//
//  dis_elfParser.h
//  disassembler
//
//  Created by karurosu on 9/8/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "dis.h"

#include <apr_strings.h>

#include <gelf.h>
#include <libelf.h>

#include <unistd.h>
#include <fcntl.h>

#ifndef disassembler_dis_elfParser_h
#define disassembler_dis_elfParser_h

extern dis_parser elfParser;

typedef struct  {
    dis_binary header;
    Elf *elf;
    size_t strtable;
    Elf_Scn *lastScn;
    
} dis_elfBinary;

#endif
