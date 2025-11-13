//
//  base.h
//  Base includes
//
//  Created by karurosu on 9/15/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#ifndef disassembler_base_h
#define disassembler_base_h

#define IN
#define OUT

#include <apr.h>
#include <apr_general.h>
#include <apr_pools.h>
#include <stdbool.h>
#include <stdio.h>

#ifndef DEBUG
#define DEBUG 0
#endif

#define debug_print(...) \
    do { if (DEBUG) fprintf(stderr, __VA_ARGS__); } while (0)


#endif
