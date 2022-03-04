src = [ 'main.c' ]

foreach name, dep : options
    bfc = executable('bfc@0@'.format(name), src,
        dependencies : dep,
        install : true
    )
endforeach
