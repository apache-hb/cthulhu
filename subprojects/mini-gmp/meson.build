project('mini-gmp', 'c',
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
