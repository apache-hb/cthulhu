src = [
    'src/main.c',
    'src/scan.c',
    'src/ast.c',
    'src/sema.c',
    lex.process('src/pl0.l'),
    parse.process('src/pl0.y')
]

pl0 = library('pl0', src,
    dependencies : [ mediator, interop, scan, report, tree, util, report ],
    c_args : generated_args,
    include_directories : [ 'src', 'include' ]
)

langs += {
    'pl0': {
        'dep': declare_dependency(link_with : pl0),
        'mod': 'kPl0Module',
        'fuzz_corpus': meson.current_source_dir() / 'fuzz_corpus'
    }
}
