src = [
    'src/ast.c',
    'src/main.c',
    'src/scan.c',
    'src/driver.c',
    'src/events.c',

    lex.process('src/cc.l'),
    parse.process('src/cc.y'),
]

libcc = library('cc', src,
    dependencies : [ runtime, interop, scan, notify, tree, util, events, memory ],
    c_args : generated_args,
    include_directories : [ 'src', 'include' ]
)

langs += {
    'cc': {
        'dep': declare_dependency(link_with : libcc),
        'mod': 'kCModule'
    }
}
