types = [
    ("char", "INTEGER_CHAR", True, "t"),
    ("uchar", "INTEGER_CHAR", False, "ut"),
    ("short", "INTEGER_SHORT", True, "s"),
    ("ushort", "INTEGER_SHORT", False, "us"),
    ("int", "INTEGER_INT", True, "i"),
    ("uint", "INTEGER_INT", False, "ui"),
    ("long", "INTEGER_LONG", True, "l"),
    ("ulong", "INTEGER_LONG", False, "ul"),
    ("isize", "INTEGER_SIZE", True, "z"),
    ("usize", "INTEGER_SIZE", False, "uz"),
    ("intptr", "INTEGER_INTPTR", True, "p"),
    ("uintptr", "INTEGER_INTPTR", False, "up"),
    ("intmax", "INTEGER_INTMAX", True, "m"),
    ("uintmax", "INTEGER_INTMAX", False, "um")
]

def gen_test(it):
    return f'''
// unit test for {it[0]} suffix

#include <inttypes.h>

#include "ctu/ast/ast.h"
#include "ctu/util/report.h"
#include "ctu/util/str.h"

int main(void) {{
    report_begin(20, true);

    node_t *base10 = ast_digit(NULL, NOWHERE, ctu_strdup("100{it[3]}"), 10);
    node_t *base2 = ast_digit(NULL, NOWHERE, ctu_strdup("100{it[3]}"), 2);
    node_t *base16 = ast_digit(NULL, NOWHERE, ctu_strdup("100{it[3]}"), 16);

    ASSERT(base10->digit == 100)("base10 digit parse failed, got `%" PRIu64 "`", base10->digit);
    ASSERT(base10->sign == {str(it[2]).lower()})("base10 sign incorrect, got %s", base10->sign ? "true" : "false");
    ASSERT(base10->integer == {it[1]})("base10 type incorrect, got `%d`", base10->integer);

    ASSERT(base2->digit == 0b100)("base2 digit parse failed, got `%" PRIu64 "`", base2->digit);
    ASSERT(base2->sign == {str(it[2]).lower()})("base2 sign incorrect, got %s", base2->sign ? "true" : "false");
    ASSERT(base2->integer == {it[1]})("base2 type incorrect, got `%d`", base2->integer);

    ASSERT(base16->digit == 0x100)("base16 digit parse failed, got `%" PRIu64 "`", base16->digit);
    ASSERT(base16->sign == {str(it[2]).lower()})("base16 sign incorrect, got %s", base16->sign ? "true" : "false");
    ASSERT(base16->integer == {it[1]})("base16 type incorrect, got `%d`", base16->integer);

    return report_end("test");
}}
    '''

for each in types:
    name = f'tests/unit/suffix/test-{each[0]}.c'
    print(f'generating test {name}')
    with open(name, 'w') as f:
        f.write(gen_test(each))
