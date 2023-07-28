src = [
    'src/main.c',
    'src/ast.c',
    'src/scan.c',

    lex.process('src/obr.l'),
    parse.process('src/obr.y')
]

obr = library('obr', src,
    dependencies : [ mediator, interop, location, hlir ],
    c_args : args,
    include_directories : 'src'
)

langs += {
    'obr': {
        'dep': declare_dependency(link_with : obr),
        'mod': 'kOberonModule'
    }
}
