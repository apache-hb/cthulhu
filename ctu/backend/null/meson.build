src = [ 'null.c' ]

libnull = library('null', src,
    include_directories : '.',
    dependencies : generic
)

options += declare_dependency(
    link_with : libnull,
    include_directories : '.'
)
