//
//  opdb.c
//  disassembler
//
//  Created by karurosu on 9/15/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "opdb.h"


bool opdb_defaultInitialize(IN opdb* self, IN apr_pool_t* parentPool, IN char* path) {
    apr_status_t rv;
    apr_file_t *fp;
    apr_finfo_t finfo;
    
    //Create this instance's pool
    apr_pool_create(&self->pool, parentPool);
    
    if ((rv = apr_file_open(&fp, path, APR_READ | APR_BINARY, APR_OS_DEFAULT, self->pool)) != APR_SUCCESS) {
        char errbuf[256];
        printf("Cannot open DB file\n");
        apr_strerror(rv, errbuf, sizeof(errbuf));
        debug_print("error: %d, %s\n", rv, errbuf);
        self->clean(self);
        return false;
    }
    
    if ((rv = apr_file_info_get(&finfo, APR_FINFO_SIZE, fp)) != APR_SUCCESS) {
        printf("Cannot get DB file info\n");
        self->clean(self);
        return false;
    }
    
    if ((rv = apr_mmap_create(&self->db, fp, 0, finfo.size, APR_MMAP_READ, self->pool)) != APR_SUCCESS) {
        printf("Cannot MMAP DB file\n");
        self->clean(self);
        return false;
    }
    
    /* you can close the file after apr_mmap_create()'s completion */
    apr_file_close(fp);
    
    debug_print("Mapped file of size = %" APR_OFF_T_FMT ", mmap-size = %" APR_SSIZE_T_FMT "\n", finfo.size, self->db->size);
    
    self->header = (struct opdb_table *) self->db->mm;
    
    //Check signature
    if (!strcmp("YNSI", self->header->signature) ){
        printf("Signature does not match\n");
        self->clean(self);
        return false;
    }
    
    //Now lets set the first Entry and first Index
    self->firstIndex = &self->header->firstIndex;
    self->firstEntry = (struct opdb_entry *) (self->firstIndex + self->header->indexes);
    
    return true;
}


bool opdb_defaultClean(IN struct opdb *self) {
    printf("Cleaning opDB\n");
    if (self->db != NULL) {
        apr_mmap_delete(self->db);
    }
    apr_pool_destroy(self->pool);
    return true;
}

bool opdb_defaultSearch(IN struct opdb *self, IN struct opdb_index *index, IN unsigned char opcode, OUT opdb_result *result) {
    struct opdb_index_entry *target;
    struct opdb_entry *first;
    int i = 0;
    
    //lets see if we have an index or we start from the first one
    if (index == NULL) {
        //debug_print("Search from the beginning\n");
        index = self->firstIndex;
    }
    
    debug_print("Searching for opcode 0x%1x in index: %s\n", opcode, index->name);
    //locate the opcode offset
    target = index->entries+(opcode);
    
    if (target->offset == 0) {
        //There is nothing at that index
        result->data = NULL;
        result->type = NOT_FOUND;
        result->count = 0;
        return true;
    }
    
    if (target->flags == 0) {
        //Found an index
        result->data = self->firstIndex + target->offset;
        result->type = RESULT_INDEX;
        result->count = 1;
        return true;
    }
    else if (target->flags == 1) {
        //find opcode entry, the minus 1 is due to spec. 
        first = self->firstEntry+(target->offset-1);
        result->data = first;
        result->type = RESULT_ENTRY;
        
        if (index->primary == true) {
            while ( i+target->offset < self->header->entries && (first+i)->primaryopcode == opcode) {
                i++;
            }
        } else {
            while ( i+target->offset < self->header->entries && (first+i)->secondaryopcode == opcode) {
                i++;
            }
        }
        
        result->count = i;
        return true;
    }
    
    return false;
}

opdb defaultDB = {.initialize = &opdb_defaultInitialize, .clean=&opdb_defaultClean, .search=&opdb_defaultSearch};