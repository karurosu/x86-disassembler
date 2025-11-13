//
//  main.c
//  disassembler
//
//  Created by karurosu on 8/30/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include <apr.h>
#include <apr_general.h>
#include <apr_pools.h>
#include <stdlib.h>
#include <stdio.h>
#include <apr_strings.h>
#include <apr_time.h>

/* Basic disassembler stuff */
#include "dis.h"
#include "dis_elfParser.h"
#include "opdb.h"
#include "printer_stdout.h"

int main(int argc, char **argv)
{
    int c;
    bool r;
    int parser_size = 1;
    dis_parser parsers[] = {elfParser};
    dis_parser *parser = NULL;
    dis_binary *bin;
    dis_section curScn;
    opdb *db = &defaultDB;
    dis_printer *printer = &stdOutPrinter;
    dis_dissassembler *disassembler = &defaultDisassembler;
    
    char* dbpath;
    
    
    
    apr_pool_t *pool = NULL;
    apr_initialize();
    atexit(apr_terminate);
    
    apr_pool_create(&pool, NULL);
    
#pragma mark header
    printf(
 "\n\
    ______  _                                         _      _\n\
    |  _  \\(_)                                       | |    | |\n\
    | | | | _  ___   __ _  ___  ___   ___  _ __ ___  | |__  | |  ___  _ __\n\
    | | | || |/ __| / _` |/ __|/ __| / _ \\| '_ ` _ \\ | '_ \\ | | / _ \\| '__|\n\
    | |/ / | |\\__ \\| (_| |\\__ \\\\__ \\|  __/| | | | | || |_) || ||  __/| |\n\
    |___/  |_||___/ \\__,_||___/|___/ \\___||_| |_| |_||_.__/ |_| \\___||_|\n\
                                                                    v0.1\n\
");
    
    if (argc != 2){
        printf("usage: Disassembler file-name\n");
        exit(-2);
    }

#pragma mark parser initialization
    //Initialize and try each parser
    for(c = 0; c < parser_size; c++) {
        r = parsers[c].initialize(&parsers[c]);
        if (r == false) {
            continue;
        }
        r = parsers[c].canParse(&parsers[c], argv[1]);
        if (r == true){
            parser = parsers+c;
            break;
        }
    }
    
    if (parser == NULL) {
        printf("Cannot find suitable parser! Exit\n");
        exit(-1);
    }
    
    printf("Using parser: %s\n", parser->name);

#pragma mark binary loading
    //Create the new binary
    
    r = parser->parse(parser,argv[1], pool, &bin);
    
    if (r == false){
        printf("There was a problem parsing the image!\n");
        exit(-3);
    }
    
    printf("Opened a binary image of type: %s\n at location: %s\n" , bin->type, bin->fileName);
    
#pragma mark DB loading
    //Load DB, hardcoded fort the time being
    dbpath = "/Volumes/Kaolla Su/Dropbox/Tec/Analisis y Diseno - Proyectos/x86.db";
    //dbpath = "x86.db";
    r = db->initialize(db, pool, dbpath);
    if (r == false) {
        printf("Cannot initialize database!\n");
        
        bin->clean(bin);
        apr_pool_destroy(pool);
        exit(-4);
    }
    printf("DB loaded: %d indexes and %d entries.\n", db->header->indexes, db->header->entries);
    
#pragma mark printer intialization
    printer->initialize(printer, db);
    
#pragma mark disassembler initialization
    disassembler->initialize(disassembler, pool, printer, db);
    
#pragma mark section parsing
    while(bin->nextSection(bin, &curScn, pool)) {
        r = disassembler->disassembleSection(disassembler, &curScn);
        if (r == false) {
            printf("Cannot disassemble section!\n");
        }
        curScn.clean(&curScn);
        printf("\n");
    }
    
#pragma mark cleanup
    db->clean(db);
    bin->clean(bin);
    disassembler->clean(disassembler);
    apr_pool_destroy(pool);
    
    return 0;
}
