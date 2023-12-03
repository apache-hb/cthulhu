project('mini_gmp', 'c',
    version : '6.0.0',
    license : 'LGPLv3',
    default_options : [
        'warning_level=1',
        'werror=false'
    ]
)

cc = meson.get_compiler('c')
c_args = cc.get_supported_arguments('/wd4146')

libgmp = library('gmp', [ 'mini_gmp.c', 'mini_mpq.c' ],
    include_directories : '.',
    c_args : c_args
)

gmp = declare_dependency(
    link_with : libgmp,
    include_directories : '.'
)
