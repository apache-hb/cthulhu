libunit = library('unit', 'unit/ct-test.cpp',
    include_directories : include_directories('unit'),
    dependencies : [ base, std ]
)

unit = declare_dependency(link_with : libunit)

cases = [
    'util.str',
    'util.map',
    'util.set'
]

foreach case : cases
    name = case.replace('.', '/')
    path = 'cases'/name + '.cpp'

    exe = executable(case, path,
        include_directories : '.',
        dependencies : [ unit, base ]
    )

    test(case, exe, suite : 'unit')
endforeach

# vfs

vfs_exe = executable('vfs', 'cases/io/vfs.cpp',
    include_directories : '.',
    dependencies : [ unit, base, location, io ]
)

test('vfs', vfs_exe, suite : 'unit')

# fs

fs_exe = executable('fs', 'cases/io/fs.cpp',
    include_directories : '.',
    dependencies : [ unit, base, location, io ]
)

test('fs', fs_exe,
    workdir : meson.project_source_root(),
    suite : 'unit'
)

# argparse

argparse_exe = executable('argparse.argparse', 'cases/argparse/argparse.cpp',
    include_directories : '.',
    dependencies : [ unit, base, argparse ]
)

test('argparse', argparse_exe, suite : 'unit')
