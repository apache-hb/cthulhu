#pragma once

#include <stdint.h>

#include "ctu/util/util.h"

/**
 * file generation for elf files
 */

typedef struct {
    uint32_t magic;
    uint8_t cls;
    uint8_t data;
    uint8_t version;
    uint8_t abi;
    uint8_t abiversion;
    uint8_t pad[7];
} elf_ident_t;

typedef struct {
    elf_ident_t ident;
    uint16_t type;
    uint16_t machine;
    uint32_t version;
    uint32_t entry;
    uint32_t phoffset;
    uint32_t shoffset;
    uint16_t flags;
    uint16_t size;
    
    uint16_t phsize;
    uint16_t phnum;
    
    uint16_t shsize;
    uint16_t shnum;

    uint16_t stridx;
} elf32_header_t;

typedef struct {
    elf_ident_t ident;
} elf64_header_t;
