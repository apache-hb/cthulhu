src = [
    'src/fs.c',
    'src/common.c',
    'src/physical.c',
    'src/virtual.c',
]

libfs = library('fs', src,
    c_args : user_args,
    dependencies : [ std, os, io, location ],
    include_directories : [ 'include', 'src' ]
)

fs = declare_dependency(
    link_with : libfs,
    dependencies : [ std, os, io ],
    include_directories : 'include'
)
