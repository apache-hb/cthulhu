src = [ 
    'cpp/main.c'
]

cpp = library('cpp', src,
    dependencies : [ generic, interop ]
)

langs += { 
    'cpp': declare_dependency(link_with : cpp) 
}
