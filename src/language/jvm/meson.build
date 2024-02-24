src = [
    'main.c',
    'src/jvm/jvm.c',
]

jvm = static_library('jvm', src,
    dependencies : [ gmp, broker, interop, scan, notify ],
    c_args : user_args,
    include_directories : [ 'include' ]
)

langs += {
    'jvm': {
        'dep': declare_dependency(link_with : jvm),
        'mod': 'kJvmModule'
    }
}
