src = [
    'src/main.c',
    'src/scan.c',
    'src/ast.c',
    'src/sema.c',
    lex.process('src/pl0.l'),
    parse.process('src/pl0.y')
]

deps = [
    broker, interop, scan,
    notify, tree, util, events,
    config, memory, driver
]

pl0 = {
    'fuzz_corpus': meson.current_source_dir() / 'fuzz_corpus'
}

if module_loader.contains('static')
    libpl0 = library('pl0', src,
        dependencies : deps,
        c_args : generated_args,
        include_directories : [ 'src', 'include' ],
        override_options : [ 'unity=off' ]
    )

    pl0 += {
        'dep': declare_dependency(link_with : libpl0),
        'mod': 'kPl0Module'
    }
endif

if module_loader.contains('shared')
    pl0so = shared_module('pl0', src,
        dependencies : deps,
        c_args : generated_args,
        include_directories : [ 'src', 'include' ],
        override_options : [ 'unity=off' ]
    )

    pl0 += { 'shared': pl0so }
endif

langs += { 'pl0': pl0 }
