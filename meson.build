project('cthulhu', 'c', default_options : [ 'c_std=c89' ])

libctu = static_library('ctu', [ 
    'ctulang/ctu.c',
    'thirdparty/vec/vec.c'
])

cli = executable('cli', 'cli/cli.c',
    link_with : libctu
)