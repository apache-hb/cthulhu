#include "elf.h"

#include <stdlib.h>
#include <string.h>

typedef struct {
    uint32_t magic;
    uint8_t cls;
    uint8_t endian;
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

    uint64_t entry;
    uint64_t phoffset;
    uint64_t shoffset;

    uint32_t flags;
    uint16_t size;
    
    uint16_t phsize;
    uint16_t phnum;

    uint16_t shsize;
    uint16_t shnum;

    uint16_t stridx;
} elf64_header_t;

typedef struct {
    uint32_t type;
    uint32_t flags;
    uint64_t offset;
    uint64_t vaddr;
    uint64_t paddr;
    uint64_t filesize;
    uint64_t memsize;
    uint64_t align;
} elf64_program_t;

typedef struct {
    uint32_t name;
    uint32_t type;

    uint64_t flags;
    uint64_t addr;
    uint64_t offset;
    uint64_t size;
    
    uint32_t link;
    uint32_t info;
    
    uint64_t align;
    uint64_t entsize;
} elf64_section_t;

typedef struct {
    uint8_t *bytes;
    size_t cursor, size;
} output_t;

static void ensure_size(output_t *out, size_t size) {
    if (size > out->size) {
        out->size = size;
        out->bytes = realloc(out->bytes, size);
    }
}

static size_t otell(output_t *out) {
    return out->cursor;
}

static void oseek(output_t *out, size_t offset) {
    ensure_size(out, offset);
    out->cursor = offset;
}

static void owrite(output_t *out, const void *ptr, size_t len) {
    ensure_size(out, out->cursor + len);
    memcpy(out->bytes + out->cursor, ptr, len);
    out->cursor += len;
}

static void odump(output_t *out, FILE *fd) {
    fwrite(out->bytes, 1, out->size, fd);
}

void emit_elf(FILE *file) {
    output_t o = { NULL, 0, 0 };
    elf64_header_t header = {
        .ident = {
            .magic = 0x464C457F, /* 0x7F ELF */
            .cls = 0x2, /* 64 bit */
            .endian = 1, /* little endian */
            .version = 1, /* must be 1 */
            .abi = 0, /* sysv abi */
            .abiversion = 0, /* ignored mostly */
            .pad = { 0 }
        },
        .type = 0x2, /* exec */
        .machine = 0x3E, /* x64 */
        .version = 1, /* must be 1 */
        .entry = 0,
        .phoffset = 0,
        .shoffset = 0,
        .flags = 0,
        .size = 64,
        .phsize = sizeof(elf64_program_t),
        .phnum = 0,
        .shsize = sizeof(elf64_section_t),
        .shnum = 2,
        .stridx = 1
    };
    owrite(&o, &header, sizeof(elf64_header_t));

    char name[] = "\0.null\0.shstrtab";
    size_t name_offset = otell(&o);
    owrite(&o, name, sizeof(name));
    size_t sh_offset = otell(&o);

    elf64_section_t strtab = {
        .name = 7,
        .type = 3,
        .flags = 2,
        .addr = 0,
        .offset = name_offset,
        .size = sizeof(name),
        .link = 0,
        .info = 0,
        .align = 1,
        .entsize = 0
    };

    elf64_section_t undefsec = {
        .name = 1,
        .type = 0,
        .flags = 0,
        .addr = 0,
        .offset = 0,
        .size = 0,
        .link = 0,
        .info = 0,
        .align = 1,
        .entsize = 0
    };

    owrite(&o, &undefsec, sizeof(elf64_section_t));
    owrite(&o, &strtab, sizeof(elf64_section_t));

    oseek(&o, 0);
    header.shoffset = sh_offset;
    owrite(&o, &header, sizeof(elf64_header_t));

    odump(&o, file);
}
