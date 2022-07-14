src = [ 
    'main.c', 
    'scan.c', 
    'ast.c', 
    'lang/sema.c', 
    'lang/attribs.c',
    'lang/repr.c',
    'lang/suffix.c',
    lex.process('ctu.l'), 
    parse.process('ctu.y') 
]

ctc = library('ctc', src,
    dependencies : [ generic, interop ],
    install : true
)

langs += { 'ctu': declare_dependency(link_with : ctc) }
