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
    c_args : c_args,
    c_shared_args : '-DMINI_GMP_EXPORTS'
)

libmpq = library('mpq', 'mini-mpq.c',
    link_with : libgmp,
    include_directories : '.',
    c_args : c_args,
    c_shared_args : '-DMINI_MPQ_EXPORTS'
)

gmp = declare_dependency(
    link_with : [ libgmp, libmpq ],
    include_directories : '.'
)
