src = [ 
    'src/main.c',
    'src/bytecode.c',
    'src/jvm.c',
    'src/attribs/attrib.c'
]

jvm = library('jvm', src,
    dependencies : [ gmp, generic, mediator, interop, location ],
    c_args : args,
    include_directories : [ 'include' ]
)

langs += { 
    'jvm': {
        'dep': declare_dependency(link_with : jvm),
        'mod': 'kJvmModule'
    }
}
