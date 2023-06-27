dir = meson.current_source_dir()

all = [ 
    'util.str', 'util.set', 'util.map', 
    'argparse.ap', 
    'io.vfs', 'io.fs' 
]

foreach case : all
    where = case.replace('.', '/')
    path = dir/where + '.c'

    it = executable(case, path,
        include_directories : '.',
        dependencies : [ mediator, argparse ]
    )

    test('test-' + case, it,
        workdir : meson.project_source_root(), 
        suite : 'unit-tests'
    )
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
