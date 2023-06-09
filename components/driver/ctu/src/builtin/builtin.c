#include "builtin/builtin.h"

#include "cthulhu/hlir/hlir.h"
#include "cthulhu/hlir/type.h"

#include "sema/sema.h"

// __builtin module

static const builtin_digit_t kDigits[] = {
    { "int8", eDigit8, eSigned },
    { "int16", eDigit16, eSigned },
    { "int32", eDigit32, eSigned },
    { "int64", eDigit64, eSigned },

    { "uint8", eDigit8, eUnsigned },
    { "uint16", eDigit16, eUnsigned },
    { "uint32", eDigit32, eUnsigned },
    { "uint64", eDigit64, eUnsigned },

    { "fast8", eDigitFast8, eSigned },
    { "fast16", eDigitFast16, eSigned },
    { "fast32", eDigitFast32, eSigned },
    { "fast64", eDigitFast64, eSigned },

    { "ufast8", eDigitFast8, eUnsigned },
    { "ufast16", eDigitFast16, eUnsigned },
    { "ufast32", eDigitFast32, eUnsigned },
    { "ufast64", eDigitFast64, eUnsigned },

    { "least8", eDigitLeast8, eSigned },
    { "least16", eDigitLeast16, eSigned },
    { "least32", eDigitLeast32, eSigned },
    { "least64", eDigitLeast64, eSigned },

    { "uleast8", eDigitLeast8, eUnsigned },
    { "uleast16", eDigitLeast16, eUnsigned },
    { "uleast32", eDigitLeast32, eUnsigned },
    { "uleast64", eDigitLeast64, eUnsigned },
};

#define TOTAL_BUILTIN_DIGITS (sizeof(kDigits) / sizeof(builtin_digit_t))

sema_t *get_builtin_sema(sema_t *root)
{
    size_t sizes[eTagTotal] = {
        [eSemaValues] = 1,
        [eSemaProcs] = 1,
        [eSemaTypes] = TOTAL_BUILTIN_DIGITS,
        [eSemaModules] = 1,
        [eTagAttribs] = 1,
        [eTagSuffix] = 1
    };

    sema_t *sema = begin_sema(root, sizes);
    node_t *node = node_builtin();
    for (size_t i = 0; i < TOTAL_BUILTIN_DIGITS; ++i) 
    {
        const builtin_digit_t *digit = kDigits + i;
        hlir_t *hlir = hlir_digit(node, digit->name, digit->digit, digit->sign);
        add_decl(sema, eSemaTypes, digit->name, hlir);
    }

    return sema;
}
