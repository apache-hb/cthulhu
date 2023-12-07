src = [
    'src/flex.c',
    'src/compile.c'
]

libinterop = library('interop', src,
    c_args : user_args,
    dependencies : [ memory, scan, report ],
    include_directories : 'include'
)

interop = declare_dependency(
    link_with : libinterop,
    dependencies : [ memory, scan ],
    include_directories : 'include'
)
