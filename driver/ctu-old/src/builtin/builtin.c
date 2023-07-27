#include "builtin/builtin.h"

#include "cthulhu/hlir/h2.h"

#include "sema/sema.h"

#include "scan/node.h"

// __builtin module

static const builtin_digit_t kDigits[] = {
    { "int8", eDigit8, eSignSigned },
    { "int16", eDigit16, eSignSigned },
    { "int32", eDigit32, eSignSigned },
    { "int64", eDigit64, eSignSigned },

    { "uint8", eDigit8, eSignUnsigned },
    { "uint16", eDigit16, eSignUnsigned },
    { "uint32", eDigit32, eSignUnsigned },
    { "uint64", eDigit64, eSignUnsigned },

    { "fast8", eDigitFast8, eSignSigned },
    { "fast16", eDigitFast16, eSignSigned },
    { "fast32", eDigitFast32, eSignSigned },
    { "fast64", eDigitFast64, eSignSigned },

    { "ufast8", eDigitFast8, eSignUnsigned },
    { "ufast16", eDigitFast16, eSignUnsigned },
    { "ufast32", eDigitFast32, eSignUnsigned },
    { "ufast64", eDigitFast64, eSignUnsigned },

    { "least8", eDigitLeast8, eSignSigned },
    { "least16", eDigitLeast16, eSignSigned },
    { "least32", eDigitLeast32, eSignSigned },
    { "least64", eDigitLeast64, eSignSigned },

    { "uleast8", eDigitLeast8, eSignUnsigned },
    { "uleast16", eDigitLeast16, eSignUnsigned },
    { "uleast32", eDigitLeast32, eSignUnsigned },
    { "uleast64", eDigitLeast64, eSignUnsigned },
};

#define TOTAL_BUILTIN_DIGITS (sizeof(kDigits) / sizeof(builtin_digit_t))

h2_t *get_builtin_sema(h2_t *root)
{
    size_t sizes[eTagTotal] = {
        [eTagValues] = 1,
        [eTagProcs] = 1,
        [eTagTypes] = TOTAL_BUILTIN_DIGITS,
        [eTagModules] = 1,
        [eTagAttribs] = 1,
        [eTagSuffix] = 1
    };

    h2_t *sema = begin_sema(root, sizes);
    node_t *node = node_builtin();
    for (size_t i = 0; i < TOTAL_BUILTIN_DIGITS; ++i)
    {
        const builtin_digit_t *digit = kDigits + i;
        h2_t *hlir = h2_type_digit(node, digit->name, digit->digit, digit->sign);
        add_decl(sema, eTagTypes, digit->name, hlir);
    }

    return sema;
}
