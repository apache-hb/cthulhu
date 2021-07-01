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

#define SECTION_NULL 0
#define SECTION_STRTAB 3

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

static size_t osize(output_t *out) {
    return out->size;
}

static void *oget(output_t *out, size_t offset) {
    return out->bytes + offset;
}

static void owrite(output_t *out, const void *ptr, size_t len) {
    ensure_size(out, out->cursor + len);
    memcpy(out->bytes + out->cursor, ptr, len);
    out->cursor += len;
}

static void omerge(output_t *out, output_t *src) {
    owrite(out, src->bytes, src->size);
}

static output_t onew(void) {
    output_t o = { NULL, 0, 0 };
    return o;
}

typedef struct {
    /* all program and section data */
    output_t contents;

    /* all program headers */
    output_t programs;
    size_t nprograms;

    /* all section headers */
    output_t sections;
    size_t nsections;

    /* strtab contents */
    output_t strings;
} elf_t;

static size_t add_string(elf_t *elf, const char *str) {
    size_t strtab = otell(&elf->strings);
    owrite(&elf->strings, str, strlen(str) + 1);
    return strtab;
}

static size_t add_section(elf_t *elf, uint32_t type, const char *name) {
    size_t strtab = add_string(elf, name);

    elf64_section_t section = {
        .name = strtab,
        .type = type,
        .flags = 0,
        .addr = 0,
        .offset = 0,
        .size = 0,
        .link = 0,
        .info = 0,
        .align = 0,
        .entsize = 0
    };

    owrite(&elf->sections, &section, sizeof(elf64_section_t));
    return elf->nsections++;
}

static elf64_section_t *get_section(elf_t *elf, size_t idx) {
    return oget(&elf->sections, idx * sizeof(elf64_section_t));
}

static size_t elf_data_offset(elf_t *elf, size_t offset) {
    return sizeof(elf64_header_t) 
        + (elf->nprograms * sizeof(elf64_program_t))
        + offset;
}

static elf_t new_elf(void) {
    elf_t elf = {
        onew(), 
        onew(), 0,
        onew(), 0,
        onew()
    };

    char nul[] = "";
    owrite(&elf.strings, nul, sizeof(nul));

    /* the first section header must be null */
    add_section(&elf, SECTION_NULL, ".null");

    return elf;
}

static void build_elf(FILE *out, elf_t *elf) {
    size_t stridx = add_section(elf, SECTION_STRTAB, ".strtab");

    elf64_section_t *str = get_section(elf, stridx);

    str->offset = elf_data_offset(elf, osize(&elf->contents));
    str->size = osize(&elf->strings);
    str->flags = 2;
    str->align = 1;

    omerge(&elf->contents, &elf->strings);

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
        .entry = 0, /* entry point */
        /* absolute offset of program headers */
        .phoffset = elf->nprograms == 0 ? 0 : sizeof(elf64_header_t), 
        /* absolute offset of section headers */
        .shoffset = elf->nsections == 0 ? 0 : elf_data_offset(elf, osize(&elf->contents)), 
        .flags = 0,
        .size = sizeof(elf64_header_t),
        .phsize = sizeof(elf64_program_t),
        .phnum = elf->nprograms,
        .shsize = sizeof(elf64_section_t),
        .shnum = elf->nsections,
        .stridx = stridx
    };

    fwrite(&header, sizeof(elf64_header_t), 1, out);
    fwrite(oget(&elf->programs, 0), sizeof(elf64_program_t), elf->nprograms, out);
    fwrite(oget(&elf->contents, 0), 1, osize(&elf->contents), out);
    fwrite(oget(&elf->sections, 0), sizeof(elf64_section_t), elf->nsections, out);
}

void emit_elf(FILE *file) {
    elf_t elf = new_elf();
    build_elf(file, &elf);
}

#if 0
void emit_elf(FILE *file) {
    elf_t elf = new_elf();
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
#endif