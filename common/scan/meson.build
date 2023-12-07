src = [
    'src/node.c',
    'src/scan.c'
]

libscan = library('scan', src,
    c_args : user_args,
    dependencies : [ memory, std, io, os ],
    include_directories : 'include'
)

scan = declare_dependency(
    link_with : libscan,
    dependencies : [ core, base, os ],
    include_directories : 'include'
)
