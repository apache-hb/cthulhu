base_api = configure_file(
    input : api_header,
    output : 'ctu_base_api.h',
    configuration : { 'MOD': 'BASE' }
)

base_include = include_directories('.', 'include')

src = [
    'src/util.c',
    'src/panic.c',
    'src/log.c'
]

libbase = library('base', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_BASE_BUILD=1' ],
    dependencies : [ core ],
    include_directories : base_include
)

base = declare_dependency(
    link_with : libbase,
    dependencies : [ core ],
    include_directories : base_include
)

install_headers(base_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
