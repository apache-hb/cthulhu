src = [
    'src/util.c',
    'src/text.c',
    'src/query.c'
]

libutil = library('util', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [ memory, base, std, scan, events, tree ]
)

util = declare_dependency(
    link_with : libutil,
    include_directories : 'include',
    dependencies : [ base ]
)
