builtins_api = configure_file(
    input : api_header,
    output : 'ctu_builtins_api.h',
    configuration : { 'MOD': 'BUILTINS' }
)

builtins_include = include_directories('.', 'include')

src = [ 'src/builtins.c' ]

libbuiltins = library('builtins', src,
    build_by_default : false,
    c_args : [ '-DCT_BUILTINS_BUILD=1' ],
    dependencies : [ core, tree, std ],
    include_directories : builtins_include
)

builtins = declare_dependency(
    dependencies : [ core, std ],
    link_with : libbuiltins,
    include_directories : builtins_include
)

install_headers(builtins_api, subdir : 'builtins')
install_subdir('include', install_dir : 'include', strip_directory : true)
