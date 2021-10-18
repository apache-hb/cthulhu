dir = meson.current_source_dir()

pass = [
    'chain', 'const', 'consts', 'local',
    'prime', 'procedure', 'square', 'values', 'var',
    'vars'
]

fail = [
    'case', 'crash', 'dumb', 'loop', 'recurse',
    'space'
]

foreach case : pass
    path = dir/case + '.pl0'
    test('pass-' + case, pl0, 
        args : [ '-V', path, '-gen', 'c99' ], 
        suite : 'pl0'
    )
endforeach

foreach case : fail
    path = dir/case + '.pl0'
    test('fail-' + case, pl0, 
        args : [ '-V', path, '-gen', 'c99' ], 
        suite : 'pl0',
        should_fail : true
    )
endforeach
