src = [ 'cir.c' ]

libcir = library('cir', src,
    include_directories : '.',
    dependencies : generic
)

options += declare_dependency(
    link_with : libcir,
    include_directories : '.'
)
