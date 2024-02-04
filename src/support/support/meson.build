# collect language dependency objects
deps = []

externs = []
mods = []

foreach id, lang : langs
    deps += lang['dep']

    mod = lang['mod']
    externs += 'CT_DRIVER_API extern const language_t ' + mod + ';'
    mods += '&' + mod
endforeach

impl = configure_file(
    input : 'data/langs.c.in',
    output : 'langs.c',
    configuration : {
        'EXTERNS': '\n'.join(externs),
        'MODS': ', '.join(mods),
        'LEN': externs.length()
    }
)

support_api = configure_file(
    input : api_header,
    output : 'ctu_support_api.h',
    configuration : { 'MOD': 'SUPPORT' }
)

support_include = include_directories('.', 'include')

libsupport = library('support', impl,
    build_by_default : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_SUPPORT_BUILD=1' ],
    include_directories : [ support_include, driver_include ],
    dependencies : deps + [ runtime, memory ]
)

support = declare_dependency(
    link_with : libsupport,
    include_directories : support_include,
    dependencies : deps
)
