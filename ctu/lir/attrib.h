#pragma once

/* all attributes a declaration can have */
typedef struct {
    bool exported; /* is this symbol exported */
    bool entry; /* is this an entry point */
    const char *mangle; /* what is the mangled name of this symbol */
} attrib_t;
