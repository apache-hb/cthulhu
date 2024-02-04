broker_api = configure_file(
    input : api_header,
    output : 'ctu_broker_api.h',
    configuration : { 'MOD': 'BROKER' }
)

broker_include = include_directories('.', 'include')

src = [ 'src/broker.c' ]

libbroker = library('broker', src,
    build_by_default : not meson.is_subproject(),
    c_args : [ '-DCT_BROKER_BUILD=1' ],
    dependencies : [ core, notify, tree ],
    include_directories : broker_include
)

broker = declare_dependency(
    dependencies : [ core ],
    link_with : libbroker,
    include_directories : broker_include
)

install_headers(broker_api, subdir : 'broker')
install_subdir('include', install_dir : 'include', strip_directory : true)
