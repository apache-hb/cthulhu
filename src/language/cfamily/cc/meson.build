src = [
    'src/ast.c',
    'src/lexer.c',
    'src/parser.c'
]

deps = [ std ]

cc_lang = static_library('cc_lang', src,
    include_directories : 'include',
    dependencies : deps
)

if default_library == 'static'
    cc_static = static_library('cc_static', 'src/main.c',
        include_directories : 'include',
        dependencies : deps,
        link_with : cc_lang
    )

    langs += {
        'cc': {
            'dep': declare_dependency(link_with : cc_static),
            'mod': 'kCModule',

            'static': cc_static,
            'module': 'kCModule'
        }
    }
elif default_library == 'shared'
    cc_shared = shared_module('cc_shared', 'src/main.c',
        include_directories : 'include',
        dependencies : deps,
        link_with : cc_lang
    )

    langs += {
        'cc': {
            'shared': cc_shared
        }
    }
endif
