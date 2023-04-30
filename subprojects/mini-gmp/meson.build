project('mini-gmp', 'c',
    version : '6.0.0',
    license : 'LGPLv3',
    default_options : [
        'warning_level=1',
        'werror=false'
    ]
)

c_args = []

if meson.get_compiler('c').get_id() == 'msvc'
    c_args = [ '/wd4146' ]
endif

libgmp = library('gmp', 'mini-gmp.c',
    include_directories : '.',
    c_args : c_args
)

libmpq = library('mpq', 'mini-mpq.c',
    include_directories : '.',
    c_args : c_args
)

gmp = declare_dependency(
    link_with : [ libgmp, libmpq ],
    include_directories : '.'
)
