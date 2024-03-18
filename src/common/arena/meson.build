arena_api = configure_file(
    input : api_header,
    output : 'ctu_arena_api.h',
    configuration : { 'MOD': 'ARENA' }
)

inc = include_directories('.', 'include')
src = [ 'src/arena.c' ]
deps = [ base ]

libarena = library('arena', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_ARENA_BUILD=1' ],
    dependencies : deps,
    include_directories : inc
)

arena = declare_dependency(
    link_with : libarena,
    dependencies : deps,
    include_directories : inc
)

install_headers(arena_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
