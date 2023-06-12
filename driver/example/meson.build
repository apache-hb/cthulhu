src = [ 'src/main.c' ]

ex = library('example', src,
    dependencies : [ generic, mediator, interop ],
    c_args : args,
    include_directories : [ 'src', versiondir ]
)

langs += { 
    'example': {
        'dep': declare_dependency(link_with : ex),
        'mod': 'kExampleModule'
    }
}
