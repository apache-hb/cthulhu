#include "argparse/argparse.h"

#include "std/str.h"

#include "cthulhu/interface/interface.h"

static group_t *codegen_group(void)
{
    vector_t *params = vector_new(32);
    ADD_FLAG(params, PARAM_STRING, "output file name", { "-o", "--output" });
    return new_group("codegen", "code generation options", params);
}

static vector_t *build_groups(void)
{
    vector_t *result = vector_new(64);
    vector_push(&result, codegen_group());
    return result;
}

int main(int argc, const char **argv)
{
    common_init();

    driver_t driver = get_driver();

    reports_t *reports = begin_reports();

    arg_parse_config_t config = {
        .argc = argc,
        .argv = argv,

        .description = format("%s command line interface", driver.name),
        .version = driver.version,

        .reports = reports,

        .groups = build_groups(),
    };

    arg_parse_result_t result = arg_parse(&config);

    if (result.exitCode != EXIT_OK)
    {
        return result.exitCode;
    }
}
