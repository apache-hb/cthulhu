src = [ lex.process('pl0.l'), parse.process('pl0.y'), 'main.c', 'scan.c', 'ast.c', 'sema.c' ]

pl0c = executable('pl0c', src,
    dependencies : generic,
    install : true
)
