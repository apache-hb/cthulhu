src = [
    'src/main.c',
    'src/driver.c',
    'src/events.c',

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

if module_loader.contains('static')
    libctu = library('ctu', src,
        c_args : generated_args,
        dependencies : deps,
        include_directories : [ 'src', 'include' ],
        override_options : [ 'unity=off' ]
    )

    ctu += {
        'dep': declare_dependency(link_with : libctu),
        'mod': 'kCtuModule'
    }
endif

if module_loader.contains('shared')
    ctuso = shared_module('ctu', src,
        c_args : generated_args,
        dependencies : deps,
        include_directories : [ 'src', 'include' ],
        override_options : [ 'unity=off' ]
    )

    ctu += {
        'shared': ctuso,
    }
endif

langs += {
    'ctu': ctu
}
