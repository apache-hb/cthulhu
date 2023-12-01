src = [ 'src/main.c' ]

ex = library('example', src,
    dependencies : [ mediator, interop, tree ],
    c_args : user_args,
    include_directories : [ 'src' ]
)

langs += {
    'example': {
        'dep': declare_dependency(link_with : ex),
        'mod': 'kExampleModule'
    }
}
