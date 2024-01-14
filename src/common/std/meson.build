
src = [
    'src/map.c',
    'src/optimal.c',
    'src/set.c',
    'src/str.c',
    'src/vector.c',

    'src/typed/vector.c',
    'src/typed/info.c'
]

libstd = library('std', src,
    build_by_default : false,
    c_args : user_args,
    dependencies : [ arena, base ],
    include_directories : 'include'
)

std = declare_dependency(
    link_with : libstd,
    dependencies : base,
    include_directories : 'include'
)
