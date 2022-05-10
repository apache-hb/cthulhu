src = [ 
    'main.c', 
    'scan.c', 
    'ast.c', 
    'sema.c', 
    lex.process('ctu.l'), 
    parse.process('ctu.y') 
]

ctc = library('ctc', src,
    dependencies : generic,
    install : true
)

langs += { 'ctc': declare_dependency(link_with : ctc) }
