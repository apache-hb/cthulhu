src = [
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

deps = [
    base, memory, std, broker,
    interop, scan, notify, tree,
    util, driver, events
]

ctu = {
    'fuzz_corpus': meson.current_source_dir() / 'fuzz_corpus'
}

ctu_lang = library('ctu_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

if module_loader.contains('static')
    ctu_static = library('ctu', 'src/main.c',
        link_with : ctu_lang,
        c_args : generated_args,
        dependencies : deps,
        include_directories : [ 'src', 'include', ctu_lang.private_dir_include() ]
    )

    ctu += {
        'dep': declare_dependency(link_with : ctu_static),
        'mod': 'kCtuModule'
    }
endif

if module_loader.contains('shared')
    ctu_shared = shared_module('ctu', 'src/main.c',
        link_with : ctu_lang,
        dependencies : deps,
        c_args : generated_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', ctu_lang.private_dir_include() ],
        override_options : [ 'unity=off' ]
    )

    ctu += {
        'shared': ctu_shared,
    }
endif

langs += {
    'ctu': ctu
}
