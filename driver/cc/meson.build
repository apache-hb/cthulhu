src = [ 
    'cc/main.c', 
    'cc/scan.c',
    lex.process('cc/cc.l'), 
    parse.process('cc/cc.y') 
]

cc = library('cc', src,
    dependencies : [ generic, interop ],
    name_suffix: lib_suffix
)

langs += { 'cc': declare_dependency(link_with : cc) }
