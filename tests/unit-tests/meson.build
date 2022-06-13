dir = meson.current_source_dir()

all = [ 'util.str', 'util.set', 'util.map', 'argparse.argparse' ]

cttest = subproject('ct-test').get_variable('cttest')

foreach case : all
    where = case.replace('.', '/')
    path = dir/where + '.c'

    it = executable(case, path,
        dependencies : [ generic, argparse, cttest ]
    )

    test('test-' + case, it, suite : 'unit-tests')
endforeach

subdir('gentool')

# currently disabled, meson has no way of detecting that a test
# has exiting with a specific exit code
# segfault = executable('segfault', dir/'platform/segfault.c',
#     dependencies : [ platform ]
# )

# test('test-segfault', segfault, 
#     should_fail : true,
#     suite : 'unit-tests'
# )
