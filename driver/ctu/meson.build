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
    c_args : args,
    include_directories : 'src',
    name_suffix: lib_suffix
)

langs += { 'ctu': declare_dependency(link_with : ctc) }
