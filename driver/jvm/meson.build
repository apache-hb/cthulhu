src = [ 
    'src/main.c',
    'src/jvm/jvm.c',
    'src/bytecode.c'
]

jvm = library('jvm', src,
    dependencies : [ gmp, generic, mediator, interop, location ],
    c_args : args,
    include_directories : [ 'src' ]
)

langs += { 
    'jvm': {
        'dep': declare_dependency(link_with : jvm),
        'mod': 'kJvmModule'
    }
}
