dir = meson.current_source_dir()
pl0c = harness['pl0c']

pass = [
    'chain', 'const', 'consts', 'local',
    'prime', 'procedure', 'square', 'values', 'var',
    'vars', 'module'
]

fail = [
    'case', 'crash', 'dumb', 'loop', 'recurse',
    'space', 'syntax'
]

crashes = {
    '0': false, '1': false, '2': false,
    '3': false, '4': false, '5': false, '6': false,
    '7': false, '8': false, '9': false, '10': false,
    '11': false, '12': false, '13': false, '14': false,
    '15': false, '16': false, '17': false, '18': false,
    '19': false, '20': true, '21': false, '22': false, 
    '23': false, '24': false, '25': false, '26': false,
}

multi = {
    '0': [ 'lib', 'main' ],
    '1': [ 'lhs', 'rhs', 'main' ]
}

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

foreach case, files : multi
    paths = []
    foreach file : files
        paths += [ dir/'multi'/case/file + '.pl0' ]
    endforeach

    test('pass-multi-' + case, pl0c,
        args : paths,
        suite : 'pl0'
    )
endforeach

foreach case, success : crashes
    path = dir/'crashes'/case + '.pl0'
    test('crash-' + case, pl0c,
        args : [ path ],
        suite : 'pl0',
        should_fail : not success
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
