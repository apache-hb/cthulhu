src = [
    # command line
    'interface/cmd.c',
    'interface/plugins.c',
    'interface/switch.c',
    lex.process('interface/cmd.l'),
    parse.process('interface/cmd.y')
]

links = host != 'windows' ? [] : [ 'setargv.obj' ]

libcmd = library('cmd-interface', src,
    dependencies : generic
)

cmd = declare_dependency(
    link_with : libcmd,
    include_directories : '.',
    dependencies : generic
)

foreach name, lib : langs
    interface = executable(name, 'interface/driver.c',
        export_dynamic : true,
        dependencies : [ cmd, lib ]
    )
endforeach
