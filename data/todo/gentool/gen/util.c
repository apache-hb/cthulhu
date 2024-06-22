#include "util.h"

#include "report/report-ext.h"
#include "std/str.h"

file_t check_open(reports_t *reports, const char *path, file_flags_t mode)
{
    cerror_t err = 0;
    file_t result = file_open(path, mode, &err);

    if (err != 0)
    {
        report_errno(reports, format("failed to open '%s'", path), err);
    }

    return result;
}
