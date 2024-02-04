target_api = configure_file(
    input : api_header,
    output : 'ctu_target_api.h',
    configuration : { 'MOD': 'TARGET' }
)

target_include = include_directories('.', 'include')

src = [ 'src/target.c' ]

libtarget = library('target', src,
    build_by_default : not meson.is_subproject(),
    c_args : [ '-DCT_TARGET_BUILD=1' ],
    dependencies : [ core, broker ],
    include_directories : target_include
)

target = declare_dependency(
    dependencies : [ core, broker ],
    link_with : libtarget,
    include_directories : target_include
)

install_headers(target_api, subdir : 'target')
install_subdir('include', install_dir : 'include', strip_directory : true)
