src = [ 
    'src/main.c', 
    'src/scan.c', 
    'src/ast.c', 
    'src/lang/sema.c', 
    'src/lang/attribs.c',
    'src/lang/repr.c',
    'src/lang/suffix.c',
    lex.process('src/ctu.l'), 
    parse.process('src/ctu.y') 
]

ctc = library('ctc', src,
    dependencies : [ generic, interop ],
    include_directories : 'src',
    install : true
)

langs += { 'ctu': declare_dependency(link_with : ctc) }
