src = [ 'src/main.c' ]

ex = static_library('example', src,
    dependencies : [ broker, interop, tree, notify, memory, driver ],
    c_args : user_args,
    include_directories : [ 'src' ],
    kwargs : libkwargs
)

langs += {
    'example': {
        'dep': declare_dependency(link_with : ex),
        'mod': 'kExampleModule'
    }
}
