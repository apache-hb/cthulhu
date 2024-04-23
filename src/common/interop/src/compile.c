// SPDX-License-Identifier: LGPL-3.0-only

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

USE_DECL
parse_result_t scan_buffer(scan_t *scan, const scan_callbacks_t *callbacks)
{
    CTASSERT(scan != NULL);
    CTASSERT(callbacks != NULL);

    int err = 0;
    void *scanner = NULL;
    void *state = NULL;

    err = callbacks->init(scan, &scanner);
    if (err != 0)
    {
        return parse_error(eParseInitError, err);
    }

    text_view_t text = scan_source(scan);
    state = callbacks->scan(text.text, text.length, scanner);
    if (state == NULL)
    {
        return parse_error(eParseScanError, err);
    }

    err = callbacks->parse(scanner, scan);
    if (err != 0)
    {
        return parse_error(eParseReject, err);
    }

    callbacks->destroy_buffer(state, scanner);
    callbacks->destroy(scanner);

    void *tree = scan_get(scan);
    return parse_value(tree);
}
