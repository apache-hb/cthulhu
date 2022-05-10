#pragma once

#include "cthulhu/driver/interface.h"

#define CT_CALLBACKS(id, prefix)                                                                                       \
    static int prefix##_##id##_##init(scan_t *extra, void *scanner)                                                    \
    {                                                                                                                  \
        return prefix##lex_init_extra(extra, scanner);                                                                 \
    }                                                                                                                  \
    static void prefix##_##id##_set_in(FILE *fd, void *scanner)                                                        \
    {                                                                                                                  \
        prefix##set_in(fd, scanner);                                                                                   \
    }                                                                                                                  \
    static int prefix##_##id##_parse(scan_t *extra, void *scanner)                                                     \
    {                                                                                                                  \
        return prefix##parse(scanner, extra);                                                                          \
    }                                                                                                                  \
    static void *prefix##_##id##_scan(const char *text, void *scanner)                                                 \
    {                                                                                                                  \
        return prefix##_scan_string(text, scanner);                                                                    \
    }                                                                                                                  \
    static void prefix##_##id##_delete(void *buffer, void *scanner)                                                    \
    {                                                                                                                  \
        prefix##_delete_buffer(buffer, scanner);                                                                       \
    }                                                                                                                  \
    static void prefix##_##id##_destroy(void *scanner)                                                                 \
    {                                                                                                                  \
        prefix##lex_destroy(scanner);                                                                                  \
    }                                                                                                                  \
    static callbacks_t id = {                                                                                          \
        .init = prefix##_##id##_##init,                                                                                \
        .setIn = prefix##_##id##_set_in,                                                                               \
        .parse = prefix##_##id##_parse,                                                                                \
        .scan = prefix##_##id##_scan,                                                                                  \
        .destroyBuffer = prefix##_##id##_delete,                                                                       \
        .destroy = prefix##_##id##_destroy,                                                                            \
    }

/**
 * @brief initialize the common runtime, always the first function an interface
 * should call
 */
void common_init(void);

/**
 * @brief find a module by name
 *
 * @param runtime the runtime to take the module from
 * @param path the name of the module
 * @return hlir_t* a module if one was found otherwise NULL
 */
hlir_t *find_module(runtime_t *runtime, const char *path);
