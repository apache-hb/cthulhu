cli = executable('ctc-cli', 'src/main.c',
    link_args : host != 'windows' ? [] : [ 'setargv.obj' ],
    dependencies : [ generic, mediate, argparse2, ssa, emit ]
)

foreach name, lib : langs
    message('Generating ' + lib.name() + '-cli')
    config = { 
        'lang': lib.full_path(),
        'cmd': cli.full_path()
    }
    configure_file(
        input : 'data/lang-cli.sh',
        output : name + '-cli',
        configuration : config
    )
endforeach
