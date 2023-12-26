src = [
    'src/config.c',
    'src/reflect.c',
    'src/update.c'
]

libconfig = library('config', src,
    include_directories : [ 'include' ],
    dependencies : [ base, std, memory, io ]
)

config = declare_dependency(
    link_with : libconfig,
    include_directories : [ 'include' ],
    dependencies : [ base, std, memory ]
)
