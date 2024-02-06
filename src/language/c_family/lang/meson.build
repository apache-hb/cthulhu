src = [
    'src/ast.c',
    'src/scan.c',
    'src/driver.c',
    'src/events.c',

    lex.process('src/cc.l'),
    parse.process('src/cc.y'),
]

deps = [
    broker, interop, scan,
    notify, tree, util, events,
    memory, driver
]

cc_lang = library('cc_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

c = {}

if module_loader.contains('static')
    cc_static = library('cc', 'src/main.c',
        link_with : cc_lang,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', cc_lang.private_dir_include() ]
    )

    c += {
        'dep': declare_dependency(link_with : cc_static),
        'mod': 'kCModule'
    }
endif

if module_loader.contains('shared')
    cc_shared = shared_module('cc', 'src/main.c',
        link_with : cc_lang,
        dependencies : deps,
        c_args : user_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', cc_lang.private_dir_include() ]
    )

    c += { 'shared': cc_shared }
endif

langs += {
    'cc': c
}
