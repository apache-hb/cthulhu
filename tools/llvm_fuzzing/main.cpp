#include "core/macros.h"
#include "base/panic.h"
#include "base/memory.h"

#include "io/fs.h"
#include "stacktrace/stacktrace.h"
#include "std/str.h"
#include "std/vector.h"

#include "io/io.h"
#include "report/report.h"

#include "argparse/argparse.h"
#include "cthulhu/mediator/interface.h"

#include "cthulhu/ssa/ssa.h"
#include "cthulhu/emit/emit.h"
#include <stdio.h>

#include DRIVER_MOD_HEADER

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

#define CHECK_REPORTS(reports, msg, error) \
    do { \
        status_t err = end_reports(reports, msg, kReportConfig); \
        if (err != 0) { \
            return error; \
        } \
    } while (0)

static mediator_t *gMediator = NULL;

struct fuzz_exception_t {
    panic_t panic;
    char *msg;
};

static void fuzzing_panic_handler(panic_t panic, const char *fmt, va_list args) {
    fuzz_exception_t ex = { panic, formatv(fmt, args) };
    stacktrace_print(stdout);
    throw fuzz_exception_t{ ex };
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv) {
    CTU_UNUSED(argc);
    CTU_UNUSED(argv);

    gPanicHandler = fuzzing_panic_handler;

    gMediator = mediator_new("llvm_fuzzing", kVersion);

    return 0;
}

enum fuzz_result_t {
    eFuzzUseless, ///< the input didnt parse
    eFuzzParsedOk, ///< the input parsed but didnt compile
    eFuzzSuccess, ///< the input compiled
    eFuzzEmitError, ///< the input compiled but failed to emit
    eFuzzValid ///< the input compiled and emitted
};

static fuzz_result_t do_fuzzing(const uint8_t *data, size_t size) {
    // initial setup
    lifetime_t *lifetime = lifetime_new(gMediator);
    ap_t *ap = ap_new("llvm_fuzzing", NEW_VERSION(1, 0, 0));

    const language_t *driver = &AFL_DRIVER;

    lifetime_config_language(lifetime, ap, driver);
    lifetime_add_language(lifetime, driver);

    reports_t *reports = lifetime_get_reports(lifetime);

    // parsing
    io_t *io = io_view("fuzzing_input", data, size);
    lifetime_parse(lifetime, &AFL_DRIVER, io);

    CHECK_REPORTS(reports, "parsing source", eFuzzUseless);

    // compilation passes
    for (size_t idx = 0; idx < eStageTotal; idx++)
    {
        compile_stage_t stage = compile_stage_t(idx);
        lifetime_run_stage(lifetime, stage);

        char *msg = format("running stage %s", stage_to_string(stage));
        CHECK_REPORTS(reports, msg, eFuzzParsedOk);
    }

    lifetime_resolve(lifetime);
    CHECK_REPORTS(reports, "resolving symbols", eFuzzSuccess);

    map_t *modmap = lifetime_get_modules(lifetime);

    // emit
    ssa_result_t ssa = ssa_compile(modmap);
    CHECK_REPORTS(reports, "generating ssa", eFuzzEmitError);

    ssa_opt(reports, ssa);
    CHECK_REPORTS(reports, "optimizing ssa", eFuzzEmitError);

    fs_t *fs = fs_virtual(reports, "out");

    emit_options_t base_emit = {
        .reports = reports,
        .fs = fs,

        .modules = ssa.modules,
        .deps = ssa.deps,
    };

    ssa_emit_options_t ssa_emit = {
        .opts = base_emit
    };

    ssa_emit_result_t ssa_result = emit_ssa(&ssa_emit);
    CHECK_REPORTS(reports, "emitting ssa", eFuzzEmitError);

    CTU_UNUSED(ssa_result);

    c89_emit_options_t c89_emit = {
        .opts = base_emit
    };

    c89_emit_result_t c89_result = emit_c89(&c89_emit);
    CHECK_REPORTS(reports, "emitting c89", eFuzzEmitError);

    CTU_UNUSED(c89_result);

    return eFuzzValid;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    try
    {
        fuzz_result_t result = do_fuzzing(data, size);
        if (result == eFuzzUseless)
        {
            return 0;
        }
        else
        {
            return -1;
        }
    }
    catch (fuzz_exception_t ex)
    {
        printf("panic: %s\n", ex.msg);
        return 99;
    }
}
