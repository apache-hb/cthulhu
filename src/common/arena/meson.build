arena_api = configure_file(
    input : api_header,
    output : 'ctu_arena_api.h',
    configuration : { 'MOD': 'ARENA' }
)

arena_include = include_directories('.', 'include')

src = [ 'src/arena.c' ]

libarena = library('arena', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_ARENA_BUILD=1' ],
    dependencies : [ base ],
    include_directories : arena_include
)

arena = declare_dependency(
    link_with : libarena,
    dependencies : [ core ],
    include_directories : arena_include
)

install_headers(arena_api)
install_subdir('include', install_dir : 'include')
