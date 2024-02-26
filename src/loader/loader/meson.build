support_api = configure_file(
    input : api_header,
    output : 'ctu_support_api.h',
    configuration : { 'MOD': 'SUPPORT' }
)

# collect language dependency objects
impl_deps = [ broker, arena, std, os, events ]
src = [ 'src/loader.c', 'src/support.c' ]

lang_count = langs.keys().length()
plugin_count = plugins.keys().length()
target_count = targets.keys().length()

has_static_modules = lang_count > 0 or plugin_count > 0 or target_count > 0

if default_library == 'static' and has_static_modules
    loader_header = configure_file(
        input : 'data/enum_modules.h.in',
        output : 'enum_modules.h',
        configuration : {
            'LANG_COUNT': lang_count,
            'PLUGIN_COUNT': plugin_count,
            'TARGET_COUNT': target_count
        }
    )

    externs = []
    lang_mods = []
    plugin_mods = []
    target_mods = []

    # TODO: this is pretty ugly, but it works for now
    foreach id, lang : langs
        impl_deps += lang['dep']

        mod = lang['mod']
        externs += 'extern const language_t ' + mod + ';'
        lang_mods += '&' + mod
    endforeach

    foreach id, plugin : plugins
        impl_deps += plugin['dep']

        mod = plugin['mod']
        externs += 'extern const plugin_t ' + mod + ';'
        plugin_mods += '&' + mod
    endforeach

    foreach id, target : targets
        impl_deps += target['dep']

        mod = target['mod']
        externs += 'extern const target_t ' + mod + ';'
        target_mods += '&' + mod
    endforeach

    static_loader = configure_file(
        input : 'data/enum_modules.c.in',
        output : 'enum_modules.c',
        configuration : {
            'EXTERNS': '\n'.join(externs),
            'LANGS': lang_count > 0 ? ', '.join(lang_mods) : '0',
            'PLUGINS': plugin_count > 0 ? ', '.join(plugin_mods) : '0',
            'TARGETS': target_count > 0 ? ', '.join(target_mods) : '0'
        }
    )

    loader_summary = {
        'Language modules': lang_count,
        'Plugin modules': plugin_count,
        'Target modules': target_count
    }

    summary(loader_summary, section : 'Static module loader')

    src += [ static_loader, 'src/load_static.c' ]
else
    src += [ 'src/disable_static.c' ]
endif

if default_library == 'shared'
    impl_deps += [ os ]
    src += [ 'src/load_shared.c' ]
else
    src += [ 'src/disable_shared.c' ]
endif

loader_cdata = configuration_data()
loader_cdata.set10('CTU_LOADER_STATIC', default_library == 'static')
loader_cdata.set10('CTU_LOADER_DYNAMIC', default_library == 'shared')

loader_config = configure_file(
    output : 'loader_config.h',
    configuration : loader_cdata
)

support_include = include_directories('.', 'include')
support_impl_include = include_directories('src')

# if no modules are provided then some loops never run
# gcc warns on this
support_args = cc.get_supported_arguments('-Wno-type-limits')

libsupport = library('support', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    c_args : user_args + support_args + [ '-DCT_SUPPORT_BUILD=1' ],
    include_directories : [ support_include, support_impl_include, driver_include ],
    dependencies : impl_deps
)

support = declare_dependency(
    link_with : libsupport,
    include_directories : support_include,
    dependencies : [ core, os ]
)
