src = [
    'src/scan.c',
    'src/ast.c',
    'src/sema.c',
    lex.process('src/pl0.l'),
    parse.process('src/pl0.y'),
    #meta.process('src/pl0.lang')
]

deps = [
    broker, interop, scan,
    notify, tree, util, events,
    config, memory, driver, std
]

pl0 = {
    'fuzz_corpus': meson.current_source_dir() / 'fuzz_corpus'
}

pl0_lang = static_library('pl0_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

if default_library == 'static'
    pl0_static = static_library('pl0_static', 'src/main.c',
        link_with : pl0_lang,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', pl0_lang.private_dir_include() ]
    )

    pl0 += {
        'dep': declare_dependency(link_with : pl0_static),
        'mod': 'kPl0Module',

        'static': pl0_static,
        'module': 'kPl0Module'
    }
elif default_library == 'shared'
    pl0_shared = shared_module('pl0_shared', 'src/main.c',
        build_by_default : not meson.is_subproject(),
        install : not meson.is_subproject(),
        link_with : pl0_lang,
        dependencies : deps,
        c_args : user_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', pl0_lang.private_dir_include() ]
    )

    pl0 += { 'shared': pl0_shared }
endif

langs += { 'pl0': pl0 }
