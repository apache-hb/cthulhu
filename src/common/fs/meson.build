fs_api = configure_file(
    input : api_header,
    output : 'ctu_fs_api.h',
    configuration : { 'MOD': 'FS' }
)

inc = include_directories('.', 'include')
impl_include = include_directories('src')

src = [
    'src/fs.c',
    'src/common.c',
    'src/physical.c',
    'src/virtual.c',
]

fs_args = cc.get_supported_arguments('/wd4324')

libfs = library('fs', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + fs_args + [ '-DCT_FS_BUILD=1' ],
    dependencies : [ arena, std, os, io, base ],
    include_directories : [ inc, impl_include ]
)

fs = declare_dependency(
    link_with : libfs,
    dependencies : [ os ],
    include_directories : inc
)

install_headers(fs_api)
install_subdir('include', install_dir : 'include', strip_directory : true)
