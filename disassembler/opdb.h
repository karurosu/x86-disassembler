//
//  opdb.h
//  disassembler
//  Opcode DataBase -  used to store and parse the DB
//
//  Created by karurosu on 9/10/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "base.h"
#include <apr_mmap.h>


#ifndef disassembler_opdb_h
#define disassembler_opdb_h

typedef struct opdb opdb;
typedef struct opdb_result opdb_result;

struct opdb_index_entry {
    char flags;
    unsigned int offset;
}  __attribute__((__packed__));

struct opdb_index {
    unsigned char opcode;
    char name[8];
    bool primary;
    struct opdb_index_entry entries[256];
} __attribute__((__packed__));

struct opdb_table {
    char signature[4];
    unsigned int indexes;
    unsigned int entries;
    struct opdb_index firstIndex;
}  __attribute__((__packed__));

struct opdb_entry{
    unsigned char prefix;
    unsigned char primaryopcode;
    unsigned char secondaryopcode;
    unsigned char opcodeext;
    unsigned int fields;
    char mnemonic[16];
    bool modrm;
    
    /*field info*/
    unsigned char operand_methods[4];
    unsigned char operand_types[4];
    unsigned char operand_flags[4];
    
}  __attribute__((__packed__));

typedef enum {
    NOT_FOUND = 0,
    RESULT_ENTRY = 1,
    RESULT_INDEX = 2
} opdb_result_type;

struct opdb_result {
    void *data;
    opdb_result_type type;
    int count;
};

typedef bool (*opdb_initializeFunction)(IN struct opdb*, IN apr_pool_t*, IN char* );
typedef bool (*opdb_cleanFunction)(IN struct opdb *);
typedef bool (*opdb_searchFunction)(IN struct opdb *, IN struct opdb_index *, IN unsigned char, OUT opdb_result* );

struct opdb {
    /* data members */
    void *base;
    void *current;
    struct opdb_index *firstIndex;
    struct opdb_entry *firstEntry;
    struct opdb_table *header;
    apr_pool_t *pool;
    apr_mmap_t *db;
    
    /* function members */
    opdb_initializeFunction initialize;
    opdb_cleanFunction clean;
    opdb_searchFunction search;
};

extern opdb defaultDB;

#endif
