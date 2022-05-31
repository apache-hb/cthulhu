src = [
    'interface/cmd.c',
    lex.process('interface/cmd.l'),
    parse.process('interface/cmd.y')
]

links = host != 'windows' ? [] : [ 'setargv.obj' ]

libcmd = library('cli-interface', src,
    dependencies : [ generic, deps ]
)

cmd = declare_dependency(
    link_with : libcmd,
    include_directories : '.',
    dependencies : [ generic, deps ]
)

foreach name, lib : langs
    executable(name, 'interface/main.c',
        dependencies : [ cmd, lib ]
    )
endforeach
