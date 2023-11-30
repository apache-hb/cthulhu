#include "base/macros.h"
#include "base/panic.h"
#include "base/memory.h"

#include "std/vector.h"

#include "cthulhu/mediator/interface.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/emit.h"

#include "io/io.h"
#include "report/report.h"

#include DRIVER_MOD_HEADER

#define CHECK_REPORTS(msg) \
    do { \
        status_t err = end_reports(cthulhu->reports, msg, reportConfig); \
        if (err != 0) { \
            return err; \
        } \
    } while (0)

static const report_config_t kReportConfig = {
    .limit = SIZE_MAX,
    .warningsAreErrors = false
};

static const version_info_t kVersion = {
    .license = "GPLv3",
    .desc = "AFL fuzzing interface",
    .author = "Elliot Haisley",
    .version = NEW_VERSION(0, 0, 1)
};

int main(int argc, const char **argv)
{
    mediator_t *mediator = mediator_new("example", kVersion);
    lifetime_t *lifetime = lifetime_new(mediator);
    reports_t *reports = lifetime_get_reports(lifetime);

    CTASSERTM(argc == 2, "must provide one argument");

    io_t *io = io_file(argv[1], eAccessRead | eAccessText);
    vector_t *sources = vector_init(io);

}
