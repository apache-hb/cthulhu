config_api = configure_file(
    input : api_header,
    output : 'ctu_config_api.h',
    configuration : { 'MOD': 'CONFIG' }
)

inc = include_directories('.', 'include')

src = [
    'src/config.c',
    'src/reflect.c',
    'src/update.c'
]

libconfig = library('config', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_CONFIG_BUILD=1' ],
    dependencies : [ base, std, arena, io ],
    include_directories : inc
)

config = declare_dependency(
    link_with : libconfig,
    include_directories : inc,
    dependencies : [ base, std ]
)

install_headers(config_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
