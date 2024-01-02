
src = [
    'src/map.c',
    'src/optimal.c',
    'src/set.c',
    'src/str.c',
    'src/vector.c',
    'src/colour.c',

    'src/typed/vector.c'
]

libstd = library('std', src,
    c_args : user_args,
    dependencies : [ memory, base ],
    include_directories : 'include'
)

std = declare_dependency(
    link_with : libstd,
    dependencies : base,
    include_directories : 'include'
)
