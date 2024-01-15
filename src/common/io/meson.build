io_api = configure_file(
    input : api_header,
    output : 'ctu_io_api.h',
    configuration : { 'MOD': 'IO' }
)

io_include = include_directories('.', 'include')

src = [
    'src/io.c',
    'src/common.c',
    'src/view.c',
    'src/buffer.c',
    'src/file.c',
    'src/console.c'
]

# TODO: make src/console.c optional to allow for platforms without stdio.h

libio = library('io', src,
    build_by_default : false,
    c_args : user_args + [ '-DCT_IO_BUILD=1' ],
    dependencies : [ std, os, arena ],
    include_directories : io_include
)

io = declare_dependency(
    link_with : libio,
    dependencies : [ os ],
    include_directories : io_include
)
