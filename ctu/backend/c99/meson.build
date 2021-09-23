src = [ 'c99.c' ]

libc99 = library('c99', src,
    include_directories : '.',
    dependencies : generic
)

options += declare_dependency(
    link_with : libc99,
    include_directories : '.'
)
