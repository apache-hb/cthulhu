project('mini-gmp', 'c',
    default_options : [
        'warning_level=0'
    ],
    version : '6.0.0',
    license : 'LGPLv3'
)

args = []

if host_machine.system() == 'windows'
    args += [ '/w0' ]
endif

lib = library('gmp', [ 'mini-gmp.c' ],
    include_directories : '.',
    c_args : args
)

gmp = declare_dependency(
    link_with : lib,
    include_directories : '.'
)
