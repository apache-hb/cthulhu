src = [ 
    'cc/main.c', 
    'cc/scan.c',
    lex.process('cc/cc.l'), 
    parse.process('cc/cc.y') 
]

cc = shared_library('cc', src,
    dependencies : [ generic, interop ]
)

langs += { 'cc': declare_dependency(link_with : cc) }
