src = [
    'src/main.c',
    'src/scan.c',
    'src/ast.c',

    'src/sema/sema.c',
    'src/sema/attribs.c',
    'src/sema/repr.c',
    'src/sema/suffix.c',
    'src/sema/config.c',

    'src/builtin/builtin.c',

    lex.process('src/ctu.l'),
    parse.process('src/ctu.y')
]

ctu = library('ctu', src,
    dependencies : [ mediator, interop, location, hlir ],
    c_args : args,
    include_directories : 'src'
)

langs += {
    'ctu': {
        'dep': declare_dependency(link_with : ctu),
        'mod': 'kCtuModule',
        'data': files('data/core/int.ct')
    }
}
