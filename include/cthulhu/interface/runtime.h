#pragma once

#include "cthulhu/interface/interface.h"

///
/// code required by drivers
/// this is provided by the generic framework
///

#define CT_CALLBACKS(id, prefix)                                                                                       \
    static int prefix##_##id##_##init(scan_t *extra, void *scanner)                                                    \
    {                                                                                                                  \
        return prefix##lex_init_extra(extra, scanner);                                                                 \
    }                                                                                                                  \
    static int prefix##_##id##_parse(void *scanner, scan_t *extra)                                                     \
    {                                                                                                                  \
        return prefix##parse(scanner, extra);                                                                          \
    }                                                                                                                  \
    static void *prefix##_##id##_scan(const char *text, size_t size, void *scanner)                                    \
    {                                                                                                                  \
        return prefix##_scan_bytes(text, (int)size, scanner);                                                          \
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
        .parse = prefix##_##id##_parse,                                                                                \
        .scan = prefix##_##id##_scan,                                                                                  \
        .destroyBuffer = prefix##_##id##_delete,                                                                       \
        .destroy = prefix##_##id##_destroy,                                                                            \
    }

/**
 * @brief find a module by name
 *
 * @param runtime the runtime to take the module from
 * @param path the name of the module
 * @return sema_t* a module if one was found otherwise NULL
 */
sema_t *find_module(runtime_t *runtime, const char *path);

void add_module(runtime_t *runtime, const char *name, sema_t *sema);
