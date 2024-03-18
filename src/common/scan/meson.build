scan_api = configure_file(
    input : api_header,
    output : 'ctu_scan_api.h',
    configuration : { 'MOD': 'SCAN' }
)

inc = include_directories('.', 'include')
src = [ 'src/node.c', 'src/scan.c' ]

libscan = library('scan', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_SCAN_BUILD=1' ],
    dependencies : [ arena, io ],
    include_directories : inc
)

scan = declare_dependency(
    link_with : libscan,
    dependencies : [ arena ],
    include_directories : inc
)

install_headers(scan_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
