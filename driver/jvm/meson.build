src = [
    'main.c',
    'src/jvm/jvm.c',
]

jvm = library('jvm', src,
    dependencies : [ gmp, mediator, interop, scan, report ],
    c_args : user_args,
    include_directories : [ 'include' ]
)

langs += {
    'jvm': {
        'dep': declare_dependency(link_with : jvm),
        'mod': 'kJvmModule'
    }
}
