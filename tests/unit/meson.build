libunit = library('unit', 'unit/ct-test.c',
    include_directories : include_directories('unit'),
    dependencies : [ stacktrace, base, std, memory ]
)

unit = declare_dependency(
    compile_args : cc.get_supported_arguments('-Wno-unused-result', '-Wno-unused-value'),
    link_with : libunit,
    dependencies : [ base, std ]
)

cases = [
    'util.str',
    'util.map',
    'util.set'
]

foreach case : cases
    name = case.replace('.', '/')
    path = 'cases'/name + '.c'

    exe = executable(case, path,
        include_directories : '.',
        dependencies : [ unit, memory, std ]
    )

    test(case, exe, suite : 'unit')
endforeach

# vfs

vfs_exe = executable('vfs', 'cases/io/vfs.c',
    include_directories : '.',
    dependencies : [ unit, base, scan, report, fs ]
)

test('vfs', vfs_exe, suite : 'unit')

# fs

fs_exe = executable('fs', 'cases/io/fs.c',
    include_directories : '.',
    dependencies : [ unit, base, std, scan, report, fs ]
)

test('fs', fs_exe,
    workdir : meson.project_source_root(),
    suite : 'unit'
)

# argparse

argparse_exe = executable('argparse.argparse', 'cases/argparse/argparse.c',
    include_directories : '.',
    dependencies : [ unit, memory, base, std, scan, report, argparse ]
)

test('argparse', argparse_exe, suite : 'unit')
