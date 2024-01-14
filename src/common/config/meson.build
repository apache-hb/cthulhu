src = [
    'src/config.c',
    'src/reflect.c',
    'src/update.c'
]

libconfig = library('config', src,
    build_by_default : false,
    include_directories : [ 'include' ],
    dependencies : [ base, std, arena, io ]
)

config = declare_dependency(
    link_with : libconfig,
    include_directories : [ 'include' ],
    dependencies : [ base, std ]
)
