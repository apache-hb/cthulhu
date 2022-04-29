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

# pl0gen = generator(pl0c,
#     output : [ '@BASENAME@.hlir' ],
#     arguments : [ '@INPUT@', '-Benable', '-O', '@OUTPUT@' ]
# )

# test('pass-import-0', pl0c,
#     args : [ dir/'import-0/main.pl0', pl0gen.process(dir/'import-0/lib.pl0') ],
#     suite : 'pl0'
# )

# test('pass-import-1', pl0c,
#     args : [ dir/'import-1/main.pl0', pl0gen.process(dir/'import-1/lib.pl0') ],
#     suite : 'pl0'
# )
