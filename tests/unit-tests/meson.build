dir = meson.current_source_dir()

all = [ 'util.str', 'util.set', 'util.map', 'driver.cmd' ]

foreach case : all
    where = case.replace('.', '/')
    path = dir/where + '.c'
    it = executable(case, path, dependencies : generic)
    test('test-' + case, it, suite : 'unit-tests')
endforeach
