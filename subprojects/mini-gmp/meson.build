project('mini-gmp', 'c',
    version : '6.0.0',
    license : 'LGPLv3',
    default_options : [
        'warning_level=1',
        'werror=false'
    ]
)

lib = library('gmp', [ 'mini-gmp.c' ],
    include_directories : '.'
)

gmp = declare_dependency(
    link_with : lib,
    include_directories : '.'
)
