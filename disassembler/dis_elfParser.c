//
//  dis_elfParser.c
//  disassembler
//
//  Created by karurosu on 9/8/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "dis_elfParser.h"

bool cleanBinaryElf(IN struct dis_binary *self) {
    debug_print("Cleaning Binary\n");
    elf_end(((dis_elfBinary *)self)->elf);
    apr_pool_destroy(self->pool);
    return true;
}

bool sectionCleanFunctionElf(IN dis_section *self) {
    debug_print("Cleaning section.\n");
    apr_pool_destroy(self->pool);
    return true;
}

bool nextSectionElf(IN struct dis_binary *handle, OUT dis_section *section, IN apr_pool_t *parentPool) {
    //Custom pointer to make my life easier
    dis_elfBinary *self = (dis_elfBinary *)handle;
    Elf_Data *data;
    GElf_Shdr shdr;
    apr_status_t st;
    char *name;
    
    //Create the sections pool
    st = apr_pool_create(&section->pool, parentPool);
    section->clean = sectionCleanFunctionElf;
    section->readChar = defaultSectionReadChar;
    section->readShort = defaultSectionReadShort;
    section->readInt = defaultSectionReadInt;
    section->readMem = defaultSectionReadMem;
    section->cursor = 0;
    
    //Get next section
    self->lastScn = elf_nextscn(self->elf, self->lastScn);
    if (self->lastScn == NULL){
        //last section found
        return false;
    }
    
    if (gelf_getshdr(self->lastScn, &shdr) != &shdr){
        printf("getshdr() failed: %s.\n",elf_errmsg(-1));
        return false;
    }
    
    if ((name = elf_strptr(self->elf, self->strtable , shdr.sh_name))== NULL) {
        printf("elf_strptr() failed: %s.\n",elf_errmsg(-1));
    }
    
    section->name = apr_pstrdup(section->pool, name);
    section->codeSection = false;
    
    if (shdr.sh_type == SHT_PROGBITS)
    {
        if(shdr.sh_flags & SHF_EXECINSTR) {
            section->codeSection = true;
        }
        else {
            return true;
        }
    }
    else {
        return true;
    }
    
    data = NULL;
    
    if ( (data = elf_getdata(self->lastScn, data) ) == NULL) {
        printf("There is no data in the section\n");
        return true;
    }
    
    if( data->d_type != ELF_T_BYTE )
    {
        printf("Unsupported data type\n");
        return true;
    }
    
    section->size = data->d_size;
    
    if( data->d_buf == ELF_T_BYTE )
    {
        printf("Buffer is NULL\n");
        return true;
    }
    
    section->buffer = data->d_buf;
    
    return true;
}

bool parseElf(IN dis_parser *self, IN char* filePath, IN apr_pool_t *parentPool, OUT dis_binary **bin) {
    dis_elfBinary *binary;
    GElf_Ehdr ehdr;
    int class;
    apr_status_t st;
    
    //New instance
    binary = apr_palloc(parentPool, sizeof(dis_elfBinary));
    
    //Set information
    st = apr_pool_create(&(binary->header.pool), parentPool);
    binary->header.type = apr_pstrdup(binary->header.pool, "ELF Binary");
    binary->header.fileName = apr_pstrdup(binary->header.pool, filePath);
    binary->header.clean = cleanBinaryElf;
    binary->header.nextSection = nextSectionElf;
    binary->lastScn = NULL;
    
    //Open the ELF file.
    if ((binary->header.fd = open(filePath, O_RDONLY|O_BINARY, 0)) < 0) {
        printf("open \%s\" failed\n", filePath);
    }
    
    binary->elf = elf_begin(binary->header.fd, ELF_C_READ , NULL);
    
    if (binary->elf == NULL) {
        printf("elf_begin() failed: %s.\n", elf_errmsg(-1));
        return false;
    }
    
    //Validate header
    if (gelf_getehdr(binary->elf, &ehdr) == NULL) {
        printf("getehdr() failed: %s.\n",elf_errmsg(-1));
        return false;
    }
    
    if ((class = gelf_getclass(binary->elf)) == ELFCLASSNONE) {
        printf("getclass() failed: %s.\n",elf_errmsg(-1));
        return false;
    }
    
    //TODO: Only 32 bit elfs are supported
    if (class != ELFCLASS32)
    {
        printf("Only 32 bit ELF files are supported at the moment!\n");
        return false;
    }
    
    //Get string table
    if (elf_getshdrstrndx(binary->elf, &binary->strtable) != 0) {
        printf("elf_getshdrstrndx() failed: %s.\n",elf_errmsg(-1));
        return false;
    }
    
    //Save the newly created instance
    *bin = (dis_binary *) binary;
    
    return true;
}

bool initializeElf(IN dis_parser *self) {
    printf("Initializing ELF parser.\n");
    if (elf_version(EV_CURRENT) == EV_NONE) {
        printf("ELF library initialization failed: %s", elf_errmsg(-1));
        return false;
    }
    return true;
}
/**
 Open a file and try to parse it as an ELF. Return if possible.
 @param self: parser instance associated with tthe method
 @param filePath: filename to parse
 @return: true if parseable, false otherwise
 **/
bool canParseElf(IN dis_parser *self, IN char *filePath) {
    Elf *elf;
    int fd;
    Elf_Kind ek;
    bool able = false;
    
    if ((fd = open(filePath, O_RDONLY|O_BINARY, 0)) < 0) {
        printf("open \%s\" failed", filePath);
    }
    
    if ((elf = elf_begin(fd, ELF_C_READ , NULL)) == NULL) {
        printf("elf_begin() failed: %s.", elf_errmsg(-1));
    }
    
    ek = elf_kind(elf);
    
    if (ek == ELF_K_ELF) {
        able = true;
    }
    
    elf_end(elf);
    close(fd);
    
    return able;
}
dis_parser elfParser = {.parse = &parseElf, .initialize = &initializeElf, .canParse = &canParseElf, .name="elf"};