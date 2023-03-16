project('mini-gmp', 'c',
    version : '6.0.0',
    license : 'LGPLv3',
    default_options : [
        'warning_level=1',
        'werror=false'
    ]
)

libgmp = library('gmp', [ 'mini-gmp.c' ],
    include_directories : '.'
)

libmpq = library('mpq', [ 'mini-mpq.c' ],
    include_directories : '.'
)

gmp = declare_dependency(
    link_with : libgmp,
    include_directories : '.'
)

mpq = declare_dependency(
    link_with : libmpq,
    include_directories : '.'
)
