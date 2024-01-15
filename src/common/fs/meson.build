fs_api = configure_file(
    input : api_header,
    output : 'ctu_fs_api.h',
    configuration : { 'MOD': 'FS' }
)

fs_include = include_directories('.', 'include')
fs_impl_include = include_directories('src')

src = [
    'src/fs.c',
    'src/common.c',
    'src/physical.c',
    'src/virtual.c',
]

fs_args = cc.get_supported_arguments('/wd4324')

libfs = library('fs', src,
    build_by_default : false,
    c_args : user_args + fs_args + [ '-DCT_FS_BUILD=1' ],
    dependencies : [ arena, std, os, io ],
    include_directories : [ fs_include, fs_impl_include ]
)

fs = declare_dependency(
    link_with : libfs,
    dependencies : [ os ],
    include_directories : fs_include
)
