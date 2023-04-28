dir = meson.current_source_dir()

all = [ 'util.str', 'util.set', 'util.map', 'argparse.argparse', 'argparse2.ap' ]

foreach case : all
    where = case.replace('.', '/')
    path = dir/where + '.c'

    it = executable(case, path,
        include_directories : '.',
        dependencies : [ generic, argparse, argparse2 ]
    )

    test('test-' + case, it, suite : 'unit-tests')
endforeach

# currently disabled, meson has no way of detecting that a test
# has exiting with a specific exit code
# segfault = executable('segfault', dir/'platform/segfault.c',
#     dependencies : [ platform ]
# )

# test('test-segfault', segfault, 
#     should_fail : true,
#     suite : 'unit-tests'
# )
