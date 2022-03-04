src = [ 'main.c' ]

foreach name, dep : options
    ctcc = executable('ctcc@0@'.format(name), src,
        dependencies : dep,
        install : true
    )
endforeach