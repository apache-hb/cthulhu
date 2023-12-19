#include "interop/compile.h"

static parse_result_t parse_error(parse_error_t result, int error)
{
    parse_result_t res = {
        .result = result,
        .error = error,
    };

    return res;
}

static parse_result_t parse_value(void *tree)
{
    parse_result_t res = {
        .result = eParseOk,
        .tree = tree,
    };

    return res;
}

parse_result_t compile_scanner(scan_t *extra, callbacks_t *callbacks)
{
    int err = 0;
    void *scanner = NULL;
    void *state = NULL;

    err = callbacks->init(extra, &scanner);
    if (err != 0)
    {
        return parse_error(eParseInitFailed, err);
    }

    state = callbacks->scan(scan_text(extra), scan_size(extra), scanner);
    if (state == NULL)
    {
        return parse_error(eParseScanFailed, err);
    }

    err = callbacks->parse(scanner, extra);
    if (err != 0)
    {
        return parse_error(eParseFailed, err);
    }

    callbacks->destroy_buffer(state, scanner);
    callbacks->destroy(scanner);

    void *tree = scan_get(extra);
    return parse_value(tree);
}
