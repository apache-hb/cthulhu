src = [
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

pl0_lang = library('pl0_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

if module_loader.contains('static')
    pl0_static = library('pl0', 'src/main.c',
        link_with : pl0_lang,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', pl0_lang.private_dir_include() ]
    )

    pl0 += {
        'dep': declare_dependency(link_with : pl0_static),
        'mod': 'kPl0Module'
    }
endif

if module_loader.contains('shared')
    pl0_shared = shared_module('pl0', 'src/main.c',
        link_with : pl0_lang,
        dependencies : deps,
        c_args : user_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', pl0_lang.private_dir_include() ]
    )

    pl0 += { 'shared': pl0_shared }
endif

langs += { 'pl0': pl0 }
