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
    [ 'types.variant5', false ],

    [ 'types.define1', false ],
    [ 'types.define2', false ],
    # TODO: this test will pass when sema is more developed
    # [ 'types.define3', true ],
    [ 'types.define4', true ],

    [ 'types.field1', false ],
    [ 'types.field2', false ],

    [ 'types.closure1', true ],
    [ 'types.closure2', false ],
    [ 'types.closure3', false ]
]

tests = types

foreach item : tests
    name = item[0]
    pass = item[1]

    path = meson.current_source_dir() / 'cases' / '/'.join(name.split('.')) + '.ct'

    test(name, ctc, args : [ path ], should_fail : not pass)
endforeach
