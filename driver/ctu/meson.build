src = [
    'src/main.c',
    'src/driver.c',

    'src/sema/type.c',
    'src/sema/sema.c',
    'src/sema/decl.c',
    'src/sema/expr.c',
    'src/sema/attrib.c',

    'src/sema/decl/resolve.c',
    'src/sema/decl/function.c',

    'src/ast.c',
    'src/scan.c',
    lex.process('src/ctu.l'),
    parse.process('src/ctu.y')
]

ctu = library('ctu', src,
    dependencies : [ base, std, mediator, interop, location, tree, util ],
    c_args : generated_args,
    include_directories : [ 'src', 'include' ]
)

langs += {
    'ctu': {
        'dep': declare_dependency(link_with : ctu),
        'mod': 'kCtuModule'
    }
}
