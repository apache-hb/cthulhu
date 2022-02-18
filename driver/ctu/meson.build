src = [ 'main.c', 'scan.c', 'ast.c', 'sema.c', lex.process('ctu.l'), parse.process('ctu.y') ]

ctc = executable('ctc', src,
    dependencies : generic,
    install : true
)
