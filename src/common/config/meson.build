config_api = configure_file(
    input : api_header,
    output : 'ctu_config_api.h',
    configuration : { 'MOD': 'CONFIG' }
)

config_include = include_directories('.', 'include')

src = [
    'src/config.c',
    'src/reflect.c',
    'src/update.c'
]

libconfig = library('config', src,
    c_args : user_args + [ '-DCT_CONFIG_BUILD=1' ],
    build_by_default : false,
    include_directories : config_include,
    dependencies : [ base, std, arena, io ]
)

config = declare_dependency(
    link_with : libconfig,
    include_directories : config_include,
    dependencies : [ base, std ]
)
