# collect language dependency objects
deps = []

externs = []
mods = []

foreach id, lang : langs
    deps += lang['dep']

    mod = lang['mod']
    externs += 'extern const language_t ' + mod + ';'
    mods += mod
endforeach

impl = configure_file(
    input : 'support/data/langs.c.in',
    output : 'langs.c',
    configuration : {
        'EXTERNS': '\n'.join(externs),
        'MODS': ', '.join(mods),
    }
)

libsupport = library('support', impl, 
    include_directories : 'support/include',
    dependencies : deps + [ mediator ]
)

support = declare_dependency(
    link_with : libsupport,
    include_directories : 'support/include',
    dependencies : deps
)

foreach interface : get_option('interfaces')
    subdir(interface)
endforeach
