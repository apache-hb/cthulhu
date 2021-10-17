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
    test('pl0-pass-' + case, pl0, args : [ '-V', path ])
endforeach

foreach case : fail
    path = dir/case + '.pl0'
    test('pl0-fail-' + case, pl0, args : [ '-V', path ], should_fail : true)
endforeach
