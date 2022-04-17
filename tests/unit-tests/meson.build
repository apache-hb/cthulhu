dir = meson.current_source_dir()

all = [ 'util.str' ]

foreach case : all
    where = case.replace('.', '/')
    path = dir/where + '.c'
    it = executable(case, path,
        dependencies : generic,
        include_directories : '.'
    )
    test('test-' + case, it)
endforeach
