src = [
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

deps = [
    base, broker, interop,
    scan, tree, util, notify,
    events, memory, driver
]

obr = {}

obr_lang = library('obr_lang', src,
    dependencies : deps,
    c_args : generated_args,
    include_directories : [ 'src', 'include' ],
    override_options : [ 'unity=off' ]
)

if module_loader.contains('static')
    obr_static = library('obr', 'src/main.c',
        link_with : obr_lang,
        dependencies : deps,
        c_args : user_args,
        include_directories : [ 'src', 'include', obr_lang.private_dir_include() ]
    )

    obr += {
        'dep': declare_dependency(link_with : obr_static),
        'mod': 'kOberonModule'
    }
endif

if module_loader.contains('shared')
    obr_shared = shared_module('obr', 'src/main.c',
        link_with : obr_lang,
        dependencies : deps,
        c_args : user_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include', obr_lang.private_dir_include() ]
    )

    obr += { 'shared': obr_shared }
endif

langs += { 'obr': obr }
