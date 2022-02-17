dir = meson.current_source_dir()

pass = [
    'multi', 'module', 'hello',
    'simple', 'vars', 'empty',

    'inttypes/all',
    'inttypes/simple',
    'inttypes/char',
    'inttypes/short',
    'inttypes/int',
    'inttypes/long',
    'inttypes/longlong',
    'inttypes/sign',

    'integral-types/bool'
]

fail = [
    'error/mismatch', 'integral-types/void'
]

foreach case : pass
    path = dir/case + '.c'
    test('pass-' + case, ctcc, 
        args : [ path ], 
        suite : 'cc'
    )
endforeach

foreach case : fail
    path = dir/case + '.c'
    test('fail-' + case, ctcc, 
        args : [ path ], 
        suite : 'cc',
        should_fail : true
    )
endforeach
