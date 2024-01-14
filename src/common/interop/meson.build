src = [
    'src/flex.c',
    'src/compile.c'
]

libinterop = library('interop', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ arena, scan ],
    include_directories : 'include'
)

interop = declare_dependency(
    link_with : libinterop,
    dependencies : [ arena, scan ],
    include_directories : 'include'
)
