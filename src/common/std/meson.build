std_api = configure_file(
    input : api_header,
    output : 'ctu_std_api.h',
    configuration : { 'MOD': 'STD' }
)

inc = include_directories('.', 'include')

src = [
    'src/map.c',
    'src/optimal.c',
    'src/set.c',
    'src/str.c',
    'src/vector.c',

    'src/typed/vector.c'
]

libstd = library('std', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_STD_BUILD=1' ],
    dependencies : [ arena, base, stb_sprintf ],
    include_directories : inc
)

std = declare_dependency(
    link_with : libstd,
    dependencies : base,
    include_directories : inc
)

install_headers(std_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
