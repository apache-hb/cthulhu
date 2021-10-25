project('mini-gmp', 'c',
    license : 'LGPLv3'
)

lib = library('gmp', [ 'mini-gmp.c' ],
    include_directories : '.'
)

gmp = declare_dependency(
    link_with : lib,
    include_directories : '.'
)
