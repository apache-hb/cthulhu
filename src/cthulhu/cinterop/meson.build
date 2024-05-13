cinterop_api = configure_file(
    input : api_header,
    output : 'ctu_cinterop_api.h',
    configuration : { 'MOD': 'CINTEROP' }
)

cinterop_include = include_directories('.', 'include')

src = [
    'src/cinterop.c',
    'src/writer.c'
]

libcinterop = library('cinterop', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_CINTEROP_BUILD=1' ],
    include_directories : cinterop_include,
    dependencies : [ base, std, tree, arena, io ]
)

cinterop = declare_dependency(
    link_with : libcinterop,
    include_directories : cinterop_include,
    dependencies : [ base ]
)

install_headers(cinterop_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
