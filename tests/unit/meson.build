libunit = library('unit', 'unit/ct-test.c',
    include_directories : include_directories('unit'),
    dependencies : [ stacktrace, base, std ]
)

unit = declare_dependency(
    compile_args : cc.get_supported_arguments('-Wno-unused-result'),
    link_with : libunit,
    dependencies : [ base, std ]
)

cases = [
    'util.str',
    'util.map',
    'util.set'
]

# gcc doesnt let me cast to void to suppress unused result warnings
if cc.get_id() == 'gcc'
    test_cpp_args = cpp.get_supported_arguments(
        '-Wno-unused-result',
        '-Wno-unused-value'
    )
else
    test_cpp_args = []
endif

foreach case : cases
    name = case.replace('.', '/')
    path = 'cases'/name + '.c'

    exe = executable(case, path,
        include_directories : '.',
        cpp_args : test_cpp_args,
        dependencies : [ unit, memory, std ]
    )

    test(case, exe, suite : 'unit')
endforeach

# vfs

vfs_exe = executable('vfs', 'cases/io/vfs.c',
    include_directories : '.',
    dependencies : [ unit, base, location, fs ]
)

test('vfs', vfs_exe, suite : 'unit')

# fs

fs_exe = executable('fs', 'cases/io/fs.c',
    include_directories : '.',
    dependencies : [ unit, base, std, location, fs ]
)

test('fs', fs_exe,
    workdir : meson.project_source_root(),
    suite : 'unit'
)

# argparse

argparse_exe = executable('argparse.argparse', 'cases/argparse/argparse.c',
    include_directories : '.',
    dependencies : [ unit, memory, base, std, location, argparse ]
)

test('argparse', argparse_exe, suite : 'unit')
