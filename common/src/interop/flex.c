#include "base/macros.h"

#include <string.h>

#define COMPILER_SOURCE 1

#include "interop/flex.h"

void flex_action(where_t *where, const char *text)
{
    where->firstLine = where->lastLine;
    where->firstColumn = where->lastColumn;

    for (int64_t i = 0; text[i]; i++)
    {
        if (text[i] == '\n')
        {
            where->lastLine += 1;
            where->lastColumn = 0;
        }
        else
        {
            where->lastColumn += 1;
        }
    }
}

int flex_input(scan_t scan, char *out, int size)
{
    size_t remainingSize = scan_size(scan) - scan_offset(scan);
    int total = MIN((int)remainingSize, size);

    memcpy(out, scan_text(scan) + scan_offset(scan), total);

    scan_advance(scan, total);

    return total;
}

void flex_init(where_t *where)
{
    where->firstLine = 0;
    where->firstColumn = 0;
    where->lastLine = 0;
    where->lastColumn = 0;
}