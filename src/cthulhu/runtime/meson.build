src = [
    'src/interface.c',
    'src/driver.c'
]

libruntime = library('runtime', src,
    include_directories : 'include',
    c_args : user_args,
    dependencies : [
        memory, base, std,
        scan, events, tree, interop, notify
    ]
)

runtime = declare_dependency(
    link_with : libruntime,
    include_directories : [ 'include' ],
    dependencies : [ base, std, notify ]
)
