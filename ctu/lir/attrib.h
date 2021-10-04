#pragma once

typedef enum {
    IMPORTED,
    PUBLIC,
    PRIVATE,
    ENTRYPOINT
} visibility_t;

/* all attributes a declaration can have */
typedef struct {
    visibility_t visibility; /* the visibility of this symbol */
    const char *mangle; /* what is the mangled name of this symbol */
} attrib_t;
