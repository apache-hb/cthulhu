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
    dependencies : [ mediator, interop, location, hlir, util ],
    c_args : args,
    include_directories : [ 'src', 'include' ]
)

langs += {
    'obr': {
        'dep': declare_dependency(link_with : obr),
        'mod': 'kOberonModule'
    }
}
