dir = meson.current_source_dir()

pass = [
    'chain', 'const', 'consts', 'local',
    'prime', 'procedure', 'square', 'values', 'var',
    'vars', 'module'
]

fail = [
    'case', 'crash', 'dumb', 'loop', 'recurse',
    'space', 'syntax'
]

foreach case : pass
    path = dir/case + '.pl0'
    test('pass-' + case, pl0c, 
        args : [ path ], 
        suite : 'pl0'
    )
endforeach

foreach case : fail
    path = dir/case + '.pl0'
    test('fail-' + case, pl0c, 
        args : [ path ], 
        suite : 'pl0',
        should_fail : true
    )
endforeach

imp0 = custom_target('import-0-lib',
    input : dir/'import-0/lib.pl0',
    output : 'lib.hlir',
    command : [ pl0c, '@INPUT@', '--bytecode', '--output', '@OUTPUT@' ]
)

test('pass-import-0', pl0c,
    args : [ dir/'import-0/main.pl0', imp0 ],
    suite : 'pl0'
)

imp1 = custom_target('import-1-lib',
    input : dir/'import-1/constant.pl0',
    output : 'constant.hlir',
    command : [ pl0c, '@INPUT@', '--bytecode', '--output', '@OUTPUT@' ]
)

test('pass-import-1', pl0c,
    args : [ dir/'import-1/main.pl0', imp1 ],
    suite : 'pl0',
    should_fail : true
)
