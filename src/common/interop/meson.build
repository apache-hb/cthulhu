interop_api = configure_file(
    input : api_header,
    output : 'ctu_interop_api.h',
    configuration : { 'MOD': 'INTEROP' }
)

interop_include = include_directories('.', 'include')

src = [
    'src/flex.c',
    'src/compile.c'
]

libinterop = library('interop', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_INTEROP_BUILD=1' ],
    dependencies : [ arena, scan ],
    include_directories : interop_include
)

interop = declare_dependency(
    link_with : libinterop,
    dependencies : [ arena, scan ],
    include_directories : interop_include
)

install_headers(interop_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
