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

libsetup = library('setup', src,
    build_by_default : false,
    c_args : user_args + [ '-DCT_SETUP_BUILD=1' ],
    include_directories : setup_include,
    dependencies : [ config, format, argparse, os, io, notify, backtrace, memory, arena ]
)

setup = declare_dependency(
    link_with : libsetup,
    include_directories : setup_include,
    dependencies : [ core ]
)
