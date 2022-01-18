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
