project('ct-std', 'c')

src = [
    'src/map.c',
    'src/optimal.c',
    'src/set.c',
    'src/str.c',
    'src/vector.c'
]

base = subproject('ct-base').get_variable('base')

libstd = library('std', src,
    include_directories : 'include',
    dependencies : base
)

std = declare_dependency(
    link_with : libstd,
    include_directories : 'include',
    dependencies : base
)
