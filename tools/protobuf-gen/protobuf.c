#include "protobuf.h"

#include "base/memory.h"

#include "report/report.h"

#include <gmp.h>

ast_t *pb_new(scan_t *scan, where_t where, astof_t of)
{
    ast_t *ast = ctu_malloc(sizeof(ast_t));
    ast->node = node_new(scan, where);
    ast->kind = of;
    return ast;
}

ast_t *pb_module(scan_t scan, where_t where, vector_t *messages);
ast_t *pb_message(scan_t scan, where_t where, const char *name, vector_t *fields);
ast_t *pb_field(scan_t scan, where_t where, const char *name, uint_least32_t index, const char *type, field_kind_t field);
ast_t *pb_enum(scan_t scan, where_t where, const char *name, vector_t *fields);
ast_t *pb_case(scan_t scan, where_t where, const char *name, uint_least32_t index);
ast_t *pb_oneof(scan_t scan, where_t where, const char *name, vector_t *fields);

static const uint_least32_t kFirstReservedIndex = 19000;
static const uint_least32_t kLastReservedIndex = 19999;

static const uint_least32_t kSmallestIndex = 1;
static const uint_least32_t kLargestIndex = 536870911;

uint_least32_t field_id(scan_t *scan, where_t where, const char *text)
{
    mpz_t digit;
    mpz_init_set_str(digit, text, 10);

    node_t node = node_new(scan, where);
    reports_t *reports = scan_reports(scan);

    // make sure its now below the minimum bound
    if (mpz_cmp_ui(digit, kSmallestIndex) < 0)
    {
        report(reports, eFatal, node, "index '%s' is smaller than the smallest allowed index of '%u'", text, kSmallestIndex);
        return 0;
    }

    // make sure its not above the maximum bound either
    if (mpz_cmp_ui(digit, kLargestIndex) > 0)
    {
        report(reports, eFatal, node, "index '%s' is larger than the largest allowed index of '%u'", text, kLargestIndex);
        return 0;
    }

    // also ensure its not in the reserved range
    if (mpz_cmp_ui(digit, kFirstReservedIndex) >= 0 && mpz_cmp_ui(digit, kLastReservedIndex) <= 0)
    {
        report(reports, eFatal, node, "index '%s' is in the served range of %u to %u", text, kFirstReservedIndex, kLastReservedIndex);
        return 0;
    }

    return mpz_get_ui(digit);
}
