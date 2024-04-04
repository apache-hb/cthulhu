base_api = configure_file(
    input : api_header,
    output : 'ctu_base_api.h',
    configuration : { 'MOD': 'BASE' }
)

inc = include_directories('.', 'include')

src = [
    'src/util.c',
    'src/panic.c',
    'src/log.c',
    'src/bitset.c'
]

deps = [ core ]

libbase = library('base', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_BASE_BUILD=1', '-D_CRT_SECURE_NO_WARNINGS' ],
    dependencies : deps,
    include_directories : inc
)

base = declare_dependency(
    link_with : libbase,
    dependencies : deps,
    include_directories : inc
)

install_headers(base_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
