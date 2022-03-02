src = [ lex.process('pl0.l'), parse.process('pl0.y'), 'main.c', 'scan.c', 'ast.c', 'sema.c' ]

foreach name, dep : options
    pl0c = executable('pl0c-@0@'.format(name), src,
        dependencies : dep,
        install : true
    )
endforeach
