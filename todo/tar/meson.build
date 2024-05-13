tar_api = configure_file(
    input : api_header,
    output : 'ctu_tar_api.h',
    configuration : { 'MOD': 'TAR' }
)

tar_include = include_directories('.', 'include')

src = [ 'src/tar.c' ]

deps = [ core, arena, fs, io, std ]

libtar = library('tar', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : [ '-DCT_TAR_BUILD=1' ],
    dependencies : deps,
    include_directories : tar_include
)

tar = declare_dependency(
    dependencies : [ core ],
    link_with : libtar,
    include_directories : tar_include
)

install_headers(tar_api, subdir : 'tar')
install_subdir('include', install_dir : 'include', strip_directory : true)
