src = [ 'main.c', 'scan.c', 'ast.c', 'sema.c', lex.process('ctu.l'), parse.process('ctu.y') ]

foreach name, dep : options
    ctc = executable('ctc@0@'.format(name), src,
        dependencies : dep,
        install : true
    )
endforeach
