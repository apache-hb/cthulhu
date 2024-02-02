src = [
    'src/main.c',

    'src/driver.c',

    'src/sema/sema.c',
    'src/sema/type.c',
    'src/sema/decl.c',
    'src/sema/expr.c',

    'src/ast.c',
    'src/scan.c',
    lex.process('src/obr.l'),
    parse.process('src/obr.y')
]

obr = library('obr', src,
    dependencies : [
        base, runtime, interop,
        scan, tree, util, notify,
        events, memory, driver
    ],
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

langs += {
    'obr': {
        'dep': declare_dependency(link_with : obr),
        'mod': 'kOberonModule'
    }
}
