types = [
    [ 'types.alias1', true ],
    [ 'types.alias2', false ],
    [ 'types.alias3', false ],
    [ 'types.alias4', true ],

    [ 'types.recurse1', false ],
    [ 'types.recurse2', true ],
    [ 'types.recurse3', false ],
    [ 'types.recurse4', false ],
    [ 'types.recurse5', false ],

    [ 'types.order1', true ],

    [ 'types.variant1', true ],
    [ 'types.variant2', true ],
    [ 'types.variant3', true ],
    [ 'types.variant4', true ],
    [ 'types.variant5', false ],
    [ 'types.variant6', false ],
    [ 'types.variant7', false ],
    [ 'types.variant8', true ],
    [ 'types.variant9', false ],

    [ 'types.define1', false ],
    [ 'types.define2', false ],
    [ 'types.define3', true ],
    [ 'types.define4', true ],

    [ 'types.field1', false ],
    [ 'types.field2', false ],

    [ 'types.closure1', true ],
    [ 'types.closure2', false ],
    [ 'types.closure3', false ],

    [ 'types.array1', true ],
    [ 'types.array2', true ],
    [ 'types.array3', false ],
    [ 'types.array4', false ],
    [ 'types.array5', false ],
    [ 'types.array6', false ],
    [ 'types.array7', false ],
]

funcs = [
    [ 'funcs.func1', true ],
    [ 'funcs.func2', true ],
    [ 'funcs.func3', false ],
    [ 'funcs.func4', true ],
    [ 'funcs.func5', true ],
    [ 'funcs.func6', true ],
    [ 'funcs.func7', true ],
    [ 'funcs.func8', true ],
    [ 'funcs.func9', true ],
]

tests = types #+ funcs

foreach item : tests
    name = item[0]
    pass = item[1]

    path = meson.current_source_dir() / 'cases' / '/'.join(name.split('.')) + '.ct'

    test(name, ctc, args : [ path ], should_fail : not pass)
endforeach
