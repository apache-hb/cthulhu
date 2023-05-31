src = [ 
    'src/main.c', 
    'src/scan.c',
    lex.process('src/cc.l'), 
    parse.process('src/cc.y') 
]

cc = library('cc', src,
    dependencies : [ generic, mediate, argparse, interop ],
    c_args : args,
    include_directories : 'src'
)

langs += { 
    'cc': {
        'dep': declare_dependency(link_with : cc),
        'mod': 'kCModule'
    }
}
