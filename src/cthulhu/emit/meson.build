src = [
    'src/common.c',
    'src/ssa.c',

    'src/c89/emit.c',
    'src/c89/type.c'
]

libemit = library('emit', src,
    include_directories : [ 'include', 'src' ],
    c_args : user_args,
    dependencies : [ arena, base, ssa, std, io, fs, scan, events, interop ]
)

emit = declare_dependency(
    link_with : libemit,
    include_directories : 'include',
    dependencies : [ base, ssa ]
)
