src = [ 
    'cpp/main.c'
]

cpp = library('cpp', src,
    dependencies : [ generic, interop ],
    name_suffix: lib_suffix
)

langs += { 'cpp': declare_dependency(link_with : cpp) }
