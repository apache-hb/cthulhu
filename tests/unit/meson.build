unit_api = configure_file(
    input : api_header,
    output : 'ctu_unit_api.h',
    configuration : { 'MOD': 'UNIT' }
)

unit_include = include_directories('.', 'unit')

libunit = library('unit', 'unit/ct-test.c',
    build_by_default : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_UNIT_BUILD=1' ],
    include_directories : unit_include,
    dependencies : [ base, std, memory, os, io, setup, format, backtrace, arena ]
)

unit_args = generated_args

unit_args += cpp.get_supported_arguments(
    # we use setjmp/longjmp in the unit tests
    '/wd4611',

    # the results arent actually ignored
    '-Wno-unused-result',

    '-Wno-clobbered'
)

unit = declare_dependency(
    compile_args : unit_args,
    link_with : libunit,
    include_directories : unit_include,
    dependencies : [ base, std, memory, arena, setup ]
)

cases = [
    'util.str',
    'util.map',
    'util.set',
    'util.bitset'
]

foreach case : cases
    name = case.replace('.', '/')
    path = 'cases'/name + '.c'

    exe = executable(case, path,
        include_directories : '.',
        dependencies : [ unit, memory, std, setup, arena, base ]
    )

    test(case, exe, suite : 'unit')
endforeach

# vfs

vfs_exe = executable('vfs', 'cases/io/vfs.c',
    include_directories : '.',
    dependencies : [ unit, base, scan, fs, setup, arena ]
)

test('vfs', vfs_exe, suite : 'unit')

# fs

fs_exe = executable('fs', 'cases/io/fs.c',
    include_directories : '.',
    dependencies : [ unit, base, std, scan, fs, setup, arena ]
)

test('fs', fs_exe,
    workdir : meson.project_source_root(),
    suite : 'unit'
)

# io

io_exe = executable('io', 'cases/io/io.c',
    include_directories : '.',
    dependencies : [ unit, memory, base, std, config, io, setup, arena ]
)

test('io', io_exe,
    workdir : meson.project_source_root(),
    suite : 'unit'
)

# argparse

argparse_exe = executable('argparse.argparse', 'cases/argparse/argparse.c',
    include_directories : '.',
    dependencies : [ unit, memory, base, std, config, argparse, setup, arena ]
)

test('argparse', argparse_exe, suite : 'unit')

# subdir('tar')
subdir('json')
