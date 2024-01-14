src = [
    'src/fs.c',
    'src/common.c',
    'src/physical.c',
    'src/virtual.c',
]

fs_args = cc.get_supported_arguments('/wd4324')

libfs = library('fs', src,
    build_by_default : false,
    c_args : user_args + fs_args,
    dependencies : [ arena, std, os, io ],
    include_directories : [ 'include', 'src' ]
)

fs = declare_dependency(
    link_with : libfs,
    dependencies : [ os ],
    include_directories : 'include'
)
