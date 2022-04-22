project('mini-gmp', 'c',
    version : '6.0.0',
    license : 'LGPLv3'
)

args = []

lib = library('gmp', [ 'mini-gmp.c' ],
    include_directories : '.',
    c_args : args
)

gmp = declare_dependency(
    link_with : lib,
    include_directories : '.'
)
