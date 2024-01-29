scan_api = configure_file(
    input : api_header,
    output : 'ctu_scan_api.h',
    configuration : { 'MOD': 'SCAN' }
)

scan_include = include_directories('.', 'include')

src = [
    'src/node.c',
    'src/scan.c'
]

libscan = library('scan', src,
    build_by_default : false,
    install : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_SCAN_BUILD=1' ],
    dependencies : [ arena, std, io, os ],
    include_directories : scan_include
)

scan = declare_dependency(
    link_with : libscan,
    dependencies : [ core, base, os ],
    include_directories : scan_include
)

install_headers(scan_api)
install_subdir('include', install_dir : 'include')
