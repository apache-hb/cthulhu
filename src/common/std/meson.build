std_api = configure_file(
    input : api_header,
    output : 'ctu_std_api.h',
    configuration : { 'MOD': 'STD' }
)

std_include = include_directories('.', 'include')

src = [
    'src/map.c',
    'src/optimal.c',
    'src/set.c',
    'src/str.c',
    'src/vector.c',

    'src/typed/vector.c',
    'src/typed/info.c'
]

libstd = library('std', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_STD_BUILD=1' ],
    dependencies : [ arena, base ],
    include_directories : std_include
)

std = declare_dependency(
    link_with : libstd,
    dependencies : base,
    include_directories : std_include
)

install_headers(std_api)
install_subdir('include', install_dir : 'include')
