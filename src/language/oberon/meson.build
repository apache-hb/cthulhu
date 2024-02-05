src = [
    'src/main.c',

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

if module_loader.contains('static')
    libobr = library('obr', src,
        dependencies : deps,
        c_args : generated_args,
        include_directories : [ 'src', 'include' ],
        override_options : [ 'unity=off' ]
    )

    obr += {
        'dep': declare_dependency(link_with : libobr),
        'mod': 'kOberonModule'
    }
endif

if module_loader.contains('shared')
    obrso = shared_library('obr', src,
        dependencies : deps,
        c_args : generated_args + [ '-DCTU_DRIVER_SHARED=1' ],
        include_directories : [ 'src', 'include' ],
        override_options : [ 'unity=off' ]
    )

    obr += { 'shared': obrso }
endif

langs += { 'obr': obr }
