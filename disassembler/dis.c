//
//  dis.c
//  disassembler
//
//  Created by karurosu on 9/22/12.
//  Copyright (c) 2012 karurosu. All rights reserved.
//

#include "dis.h"

//Values are hardcoded to x86

bool defaultSectionReadChar (IN struct dis_section *self, OUT unsigned char **buffer){
    return self->readMem(self, (void *)buffer, 1);
}

bool defaultSectionReadShort(IN struct dis_section *self, OUT unsigned short **buffer) {
    return self->readMem(self, (void *)buffer,  2);
}

bool defaultSectionReadInt(IN struct dis_section *self, OUT unsigned int **buffer) {
    return self->readMem(self, (void *)buffer,  4);
}

bool defaultSectionReadMem(IN struct dis_section *self, OUT void **buffer, IN int size) {
    if(self->codeSection){
        //Check if we have enough data
        if (self->cursor + size > self->size) {
            debug_print("Out of output!'n");
            return false;
        }
        *buffer = (char *) self->buffer + self->cursor;
        //Switched to pointers for speed.
        //memcpy(buffer, start , size);
        self->cursor += size;

        return true;
    }
    else {
        return false;
    }
}

bool disassemblerInitialize (IN struct dis_disassembler *self, IN apr_pool_t *pool, IN dis_printer *printer, IN opdb *db){
    apr_pool_create(&self->pool, pool);
    self->printer = printer;
    self->db = db;
    return true;
}

bool disassemblerCleanup(IN struct dis_disassembler *self) {
    apr_pool_destroy(self->pool);
    return true;
}

bool disassemblerDisassembleNext(IN struct dis_disassembler *self, IN dis_section *section) {
    dis_opcode result;
    opdb_result searchResult;
    void *pointer = NULL;
    bool r;
    int i,c = 0;
    char cleanopcode = 0;
    
    //Initialize the instruction
    
    //Set base data
    result.section = section;
    result.offset = section->cursor;
    result.opcode_offset = 0;
    result.modrm_offset = 0xff;
    
    //Begin disassembling!
    
    //Try to read th 4 prefixes
    for (i = 0; i < 4; i++) {
        section->readChar(section, (unsigned char**) &pointer);
        //Find if we have a prefix
        r = self->db->search(self->db, NULL, *(unsigned char *) pointer, &searchResult);
        if (r == false)
        {
            printf("Error searching!\n");
            return false;
        }
        if (searchResult.type == NOT_FOUND)
        {
            //this may be an opcode with a register encoded, lets continue
            break;
        }
        else if (searchResult.type == RESULT_ENTRY) {
            //We have an opcode, break
            break;
        }
        else if (searchResult.type == RESULT_INDEX){
            //This may be a prefix or an opcode, we have to search again, but using the current index
            section->readChar(section, (unsigned char**) &pointer);
            r = self->db->search(self->db, searchResult.data, *(unsigned char *) pointer, &searchResult);
            if (r == false)
            {
                printf("Error searching!\n");
                return false;
            }
            if (searchResult.type == NOT_FOUND)
            {
                //This is a prefix, store it and continue. Lower one to re read the last char.
                result.prefixes[i] = *(unsigned char*)pointer;
                result.opcode_offset++;
                section->cursor--;
                continue;
            }
            else if (searchResult.type == RESULT_ENTRY) {
                //There is an entry, probably we have a 2 byte opcode, break.
                break;
            }
            else if (searchResult.type == RESULT_INDEX) {
                //Another index, probably a 3 byte opcode, break.
                break;
            }
        }
    }
    //We have read all the prefixes, lets go to the position next to the prefixes.
    section->cursor = result.offset + result.opcode_offset;
    
    //Now read the first byte and try to find an opcode, max size should be 3 bytes.
    searchResult.data = NULL;
    searchResult.count = 0;
    
    for (i = 0; i < 3; i++)
    {
        section->readChar(section, (unsigned char**) &pointer);
        r = self->db->search(self->db, (struct opdb_index *) searchResult.data, *(unsigned char *) pointer, &searchResult);
        if (r == false)
        {
            printf("Error searching!\n");
            return false;
        }
        
        if (searchResult.type == NOT_FOUND)
        {
            //If we dont have a result, this may mean that we have a "weird" opcode with a register encoded. Try again with the clean opcode.
            debug_print("Cannot find opcode, lets try cleaning up\n");
            cleanopcode = *(unsigned char *) pointer & 248; //248 is to remove the 3 least significant bits
            r = self->db->search(self->db, (struct opdb_index *) searchResult.data, cleanopcode, &searchResult);
            if (r == false)
            {
                printf("Error searching!\n");
                return false;
            }
            if (searchResult.type == NOT_FOUND)
            {
                //not found, again, this is bad.
                break;
            }
            else if (searchResult.type == RESULT_ENTRY) {
                //hey!, an entry, hopefully its unique
                if (searchResult.count == 1) {
                    //We have found the opcode, finally!
                    debug_print("Direct opcode\n");
                    result.opcode_data = (struct opdb_entry*)searchResult.data;
                }
                //We found a list of possible opcodes, we need to narrow it down. Lets leave that for another step.
                break;
            }
            else if (searchResult.type == RESULT_INDEX) {
                //An index, maybe a 2 byte opcode? thats weird as a prefix...
                continue;
            }
        }
        else if (searchResult.type == RESULT_ENTRY) {
            if (searchResult.count == 1) {
                //We have found the opcode, finally!
                debug_print("Direct opcode\n");
                result.opcode_data = (struct opdb_entry*)searchResult.data;
            }
            //We found a list of possible opcodes, we need to narrow it down. Lets leave that for another step.
            break;
        }
        else if (searchResult.type == RESULT_INDEX) {
            //Another index, probably a multibyte opcode, continue as is.
            continue;
        }
        //We should not be here :S
        return false;
    }
    
    if (searchResult.type == NOT_FOUND) {
        //No opcode found, and no way to go, sniff sniff :(
        printf("No opcode found\n");
        return false;
    }
    
    //Ok, up to here we have either an opcode selected, or a list of opcodes to play with. Lets begin by narrowing down the options.
    if (searchResult.count > 1)
    {
        unsigned char opcodext = 0;
        struct opdb_entry *current;
        
        debug_print("Found a list of opcodes, searching\n");
        
        //Lets assume we need a mod/rm byte
        section->readChar(section, (unsigned char**) &pointer);
        opcodext = ((*(unsigned char*)pointer) & 56) >> 3;
        
        //Store the position of the mod/rm byte
        result.modrm_offset =  section->cursor - result.offset - 1;
        
        for (c = 0; c < searchResult.count; c++)
        {
            current = ((struct opdb_entry *)searchResult.data) + c;
            debug_print("%s\n", current->mnemonic);
            if (current->opcodeext == 0xff) {
                debug_print("Entry has no extension!\n");
                continue;
            }
            if (opcodext == current->opcodeext)
            {
                //Found a matching extension!
                result.opcode_data = current;
                break;
            }
        }
    }
    
    //This good! we should have opcode data at this point. If we dont have it, then I have no idea what happened.
    if (result.opcode_data == NULL) {
        //If we are here, and we dont have opcode data, we in big trouble.
        printf("No opcode found\n");
        return false;
    }
    
    debug_print("Selected opcode is: %s\n", result.opcode_data->mnemonic);
    
    //Now lets parse each operand.
    for (i = 0; i < 4; i ++)
    {
        if (result.opcode_data->operand_methods[i] == NO_METHOD)
        {
            debug_print("No more operands, finishing.\n");
            result.operand_count = i+1;
            break;
        }
        debug_print("Looking at operand %d\n", i);
        if (parseOperandFunctions[result.opcode_data->operand_methods[i]](section, &result, i, result.opcode_data->operand_methods[i]) == false)
        {
            printf("Cannot parse operand: %d\n", i);
            return false;
        }
    }
    
    result.size = section->cursor - result.offset;
    debug_print("Found instruction of size: %d\n",result.size);
    debug_print("Found instruction with opcode at: %d\n",result.opcode_offset);
    debug_print("Found instruction with modrm at: %d\n",result.modrm_offset);
    self->printer->printOpcode(self->printer, &result);
    return true;
}

bool disassemblerDisassembleSection(IN struct dis_disassembler *self, IN dis_section *section) {
    bool r = true;
    
    //Print section data
    self->printer->printSection(self->printer,section);
    //If the code is not a data section, continue
    if (!section->codeSection)
    {
        return true;
    }
    
    while (r == true && section->cursor < section->size){
        r = self->disassembleNext(self, section);
    }
    return r;
}

dis_dissassembler defaultDisassembler = {.initialize = &disassemblerInitialize, .clean = &disassemblerCleanup, .disassembleSection = &disassemblerDisassembleSection, .disassembleNext = &disassemblerDisassembleNext};