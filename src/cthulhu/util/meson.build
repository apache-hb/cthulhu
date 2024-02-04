util_api = configure_file(
    input : api_header,
    output : 'ctu_util_api.h',
    configuration : { 'MOD': 'UTIL' }
)

util_include = include_directories('.', 'include')

src = [
    'src/util.c',
    'src/text.c',
    'src/query.c'
]

libutil = library('util', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_UTIL_BUILD=1' ],
    include_directories : util_include,
    dependencies : [ memory, base, std, scan, events, tree, arena ]
)

util = declare_dependency(
    link_with : libutil,
    include_directories : util_include,
    dependencies : [ base ]
)

install_headers(util_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
