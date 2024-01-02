#include "cpp/cpp.h"
#include "base/log.h"
#include "base/panic.h"

#include "cpp/scan.h"

#include "cpp_bison.h" // IWYU pragma: keep
#include "cpp_flex.h" // IWYU pragma: keep

#include "io/io.h"
#include "std/typed/vector.h"

io_t *cpp_preprocess(cpp_config_t config, scan_t *input)
{
    CTASSERT(config.arena != NULL);
    CTASSERT(config.logger != NULL);
    CTASSERT(config.include_directories != NULL);
    CTASSERT(config.defines != NULL);
    CTASSERT(input != NULL);

    cpp_extra_t extra = { 0 };

    int init_err = cpp_extra_init(config, &extra);
    CTASSERTF(init_err == 0, "Failed to initialize preprocessor: %d", init_err);

    cpp_file_t *file = cpp_file_from_scan(&extra, input);
    cpp_set_current_file(&extra, file);

    int parse_err = cpp_parse(&extra);
    if (parse_err != 0)
    {
        ctu_log("failed to parse: %d", parse_err);
    }

    // TODO: properly cleanup

    const char *text = typevec_data(extra.result);
    size_t size = typevec_len(extra.result);

    return io_view("out", text, size, config.arena);
}
