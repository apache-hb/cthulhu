setup_api = configure_file(
    input : api_header,
    output : 'ctu_setup_api.h',
    configuration : { 'MOD': 'SETUP' }
)

setup_include = include_directories('.', 'include')

src = [
    'src/setup.c',
    'src/memory.c'
]

deps = [
    config, format, argparse, os, io,
    notify, backtrace, memory, arena,
    events
]

libsetup = library('setup', src, setup_api,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_SETUP_BUILD=1' ],
    include_directories : setup_include,
    dependencies : deps
)

setup = declare_dependency(
    link_with : libsetup,
    include_directories : setup_include,
    dependencies : [ format ]
)
