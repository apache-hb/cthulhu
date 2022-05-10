dir = meson.current_source_dir()

all = [ 'util.str', 'util.set', 'util.map' ]

foreach case : all
    where = case.replace('.', '/')
    path = dir/where + '.c'

    it = executable(case, path,
        dependencies : [ generic, cttest ]
    )

    test('test-' + case, it, suite : 'unit-tests')
endforeach
