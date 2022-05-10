src = [
    # command line
    'interface/driver.c',
    'interface/cmd.c',
    'interface/plugins.c',
    'interface/switch.c',
    lex.process('interface/cmd.l'),
    parse.process('interface/cmd.y')
]

links = host != 'windows' ? [] : [ 'setargv.obj' ]

foreach name, lib : langs
    interface = executable(name, src,
        export_dynamic : true,
        dependencies : [ generic, lib ]
    )
endforeach
