src = [ 'src/flex.c' ]

libinterop = library('interop', src,
    c_args : user_args,
    dependencies : location,
    include_directories : 'include'
)

interop = declare_dependency(
    link_with : libinterop,
    dependencies : location,
    include_directories : 'include'
)
