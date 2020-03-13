project('ctulang', 'c',
    default_options : [ 'c_std=c11' ]
)

sources = [
    'ctu/ctu.c',
    'ctu/keyword.c',
    'utils/string/buffer.c',
    'utils/vec/vec.c',
    'utils/map/map.c'
]

libctu = library('ctu', sources)


ctu = executable('ctu', 'cli/cli.c',
    link_with : libctu
)