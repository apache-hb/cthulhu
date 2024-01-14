src = [
    'src/node.c',
    'src/scan.c'
]

libscan = library('scan', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ arena, std, io, os ],
    include_directories : 'include'
)

scan = declare_dependency(
    link_with : libscan,
    dependencies : [ core, base, os ],
    include_directories : 'include'
)
