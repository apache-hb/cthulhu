types = [
    [ 'types.recurse1', false ],
    [ 'types.recurse2', true ],
    [ 'types.recurse3', false ],
    [ 'types.recurse4', false ],

    [ 'types.order1', true ],

    [ 'types.variant1', true ],
    [ 'types.variant2', true ],
    [ 'types.variant3', true ],
    [ 'types.variant4', true ],
    [ 'types.variant5', false ]
]

tests = types

foreach item : tests
    name = item[0]
    pass = item[1]

    path = 'cases/' + '/'.join(name.split('.')) + '.ct'

    test(name, ctc, args : [ path ], should_fail : not pass, workdir : meson.current_source_dir())
endforeach
