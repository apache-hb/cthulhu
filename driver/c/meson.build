src = [ 'main.c', 'scan.c', lex.process('cc.l'), parse.process('cc.y') ]

ctcc = executable('ctcc', src,
    dependencies : generic,
    install : true
)
