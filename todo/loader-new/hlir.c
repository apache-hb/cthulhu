#include "loader.h"

#include "cthulhu/hlir/hlir.h"

#define INDICES(span, ...)                                                                                             \
    enum                                                                                                               \
    {                                                                                                                  \
        span,                                                                                                          \
        __VA_ARGS__                                                                                                    \
    };

typedef enum
{
    INDEX_SPAN = HLIR_TOTAL,
    INDEX_ATTRIBS,
    INDEX_SCANNER,
    INDEX_TOTAL
} index_t;
