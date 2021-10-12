#include "retype.h"

#define DIGIT_FITS_CHECK(digit, func, sema, sign, width) \
    if (func(digit)) { return get_cached_digit_type(sema, sign, width); }

static type_t *select_digit_width(sema_t *sema, node_t *node, mpz_t digit) {
    DIGIT_FITS_CHECK(digit, mpz_fits_sshort_p, sema, SIGNED, TY_SHORT);
    DIGIT_FITS_CHECK(digit, mpz_fits_ushort_p, sema, UNSIGNED, TY_SHORT);
    DIGIT_FITS_CHECK(digit, mpz_fits_sint_p, sema, SIGNED, TY_INT);
    DIGIT_FITS_CHECK(digit, mpz_fits_uint_p, sema, UNSIGNED, TY_INT);
    DIGIT_FITS_CHECK(digit, mpz_fits_slong_p, sema, SIGNED, TY_LONG);
    DIGIT_FITS_CHECK(digit, mpz_fits_ulong_p, sema, UNSIGNED, TY_LONG);
    
    report(sema->reports, ERROR, node, "integer `%s` does not fit into any integral type", mpz_get_str(NULL, 10, digit));
    return get_cached_digit_type(sema, SIGNED, TY_LONG);
}

static lir_t *retype_digit(sema_t *sema, const type_t *type, lir_t *lir) {
    if (is_literal(lir_type(lir))) {
        if (is_any(type)) {
            return lir_digit(lir->node, select_digit_width(sema, lir->node, lir->digit), lir->digit);
        }

        return lir_digit(lir->node, type, lir->digit);
    }

    return lir;
}

lir_t *retype_expr(sema_t *sema, const type_t *type, lir_t *lir) {
    switch (lir->leaf) {
    case LIR_DIGIT: return retype_digit(sema, type, lir);

    case LIR_STRING: case LIR_BOOL: case LIR_NAME:
    case LIR_ACCESS: case LIR_CALL:
        return lir;

    default: 
        ctu_assert(sema->reports, "retype-sema unknown leaf %d", lir->leaf);
        return lir_poison(lir->node, "retype-sema unknown leaf");
    }
}
