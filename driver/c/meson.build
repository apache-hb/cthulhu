src = [ lex.process('cc.l'), parse.process('cc.y'), 'main.c', 'scan.c', 'sema.c', 'ast.c' ]

ctcc = executable('ctcc', src,
    dependencies : generic,
    install : true
)
