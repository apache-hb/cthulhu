project('miniz', 'c')

src = [ 'miniz/miniz.c' ]

libzip = library('miniz', src,
    c_args : [ '-D_LARGEFILE64_SOURCE=1' ]
)

miniz = declare_dependency(
    link_with : libzip,
    include_directories : '.'
)
