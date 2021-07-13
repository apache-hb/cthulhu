ast_digit_test = executable('ast-digit-test', 'ast-digit.c',
    dependencies : ctu
)

ops_test = executable('ops-test', 'ops.c',
    dependencies : ctu
)

get_type_test = executable('get-type-test', 'get-type.c',
    dependencies : ctu
)

str_compile_test = executable('str-compile-test', 'str-compile.c',
    dependencies : ctu
)


test('ast-digit-test', ast_digit_test)
test('ops-test', ops_test)
test('get-type-test', get_type_test)
test('str-compile-test', str_compile_test)

decl_name_test = executable('decl-name-test', 'decl-name.c',
    dependencies : ctu
)

resolved_name_test = executable('resolved-name-test', 'resolved-name.c',
    dependencies : ctu
)

symbol_name_test = executable('symbol-name-test', 'symbol-name.c',
    dependencies : ctu
)

test('decl-name-test', decl_name_test, should_fail : true)
test('resolved-name-test', resolved_name_test, should_fail : true)
test('symbol-name-test', symbol_name_test, should_fail : true)
