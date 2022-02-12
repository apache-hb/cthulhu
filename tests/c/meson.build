dir = meson.current_source_dir()

pass = [
    'lang/multi', 'lang/module', 'lang/hello',
    'lang/simple', 'lang/vars', 'lang/empty',

    'lang/inttypes/all',
    'lang/inttypes/simple',
    'lang/inttypes/char',
    'lang/inttypes/short',
    'lang/inttypes/int',
    'lang/inttypes/long',
    'lang/inttypes/longlong',
    'lang/inttypes/sign',

    'lang/integral-types/bool'
]

fail = [
    'lang/error/mismatch', 'lang/integral-types/void'
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
