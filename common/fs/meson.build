src = [
    'src/fs.c',
    'src/common.c',
    'src/physical.c',
    'src/virtual.c',
]

libfs = library('fs', src,
    c_args : user_args,
    dependencies : [ memory, std, os, io, scan, report ],
    include_directories : [ 'include', 'src' ]
)

fs = declare_dependency(
    link_with : libfs,
    dependencies : [ io ],
    include_directories : 'include'
)
