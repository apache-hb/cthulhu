memory_api = configure_file(
    input : api_header,
    output : 'ctu_memory_api.h',
    configuration : { 'MOD': 'MEMORY' }
)

memory_include = include_directories('.', 'include')

src = [
    'src/memory.c'
]

libmemory = library('memory', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_MEMORY_BUILD=1' ],
    dependencies : [ gmp, arena, base ],
    include_directories : memory_include
)

memory = declare_dependency(
    link_with : libmemory,
    dependencies : [ gmp, core ],
    include_directories : memory_include
)

install_headers(memory_api)
install_subdir('include', install_dir : 'include')
