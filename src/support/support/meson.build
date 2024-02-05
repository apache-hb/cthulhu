support_api = configure_file(
    input : api_header,
    output : 'ctu_support_api.h',
    configuration : { 'MOD': 'SUPPORT' }
)

# collect language dependency objects
impl_deps = [ broker, arena, std, os, events ]
src = [ 'src/loader.c', 'src/support.c' ]

if module_loader.contains('static')
    lang_count = langs.keys().length()
    plugin_count = plugins.keys().length()
    target_count = targets.keys().length()
    if lang_count > 0
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

        foreach id, lang : langs
            impl_deps += lang['dep']

            mod = lang['mod']
            externs += 'CT_DRIVER_API extern const language_t ' + mod + ';'
            lang_mods += '&' + mod
        endforeach

        foreach id, plugin : plugins
            impl_deps += plugin['dep']

            mod = plugin['mod']
            externs += 'CT_DRIVER_API extern const plugin_t ' + mod + ';'
            plugin_mods += '&' + mod
        endforeach

        foreach id, target : targets
            impl_deps += target['dep']

            mod = target['mod']
            externs += 'CT_DRIVER_API extern const target_t ' + mod + ';'
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
        warning('No language modules found, static loader will not be built')
    endif
endif

if module_loader.contains('dynamic')
    impl_deps += [ os ]
    src += [ 'src/load_shared.c' ]
endif

loader_cdata = configuration_data()
loader_cdata.set10('CTU_LOADER_STATIC', module_loader.contains('static'))
loader_cdata.set10('CTU_LOADER_DYNAMIC', module_loader.contains('dynamic'))

loader_config = configure_file(
    output : 'loader_config.h',
    configuration : loader_cdata
)

support_include = include_directories('.', 'include')
support_impl_include = include_directories('src')

libsupport = library('support', src,
    build_by_default : not meson.is_subproject(),
    c_args : user_args + [ '-DCT_SUPPORT_BUILD=1' ],
    include_directories : [ support_include, support_impl_include, driver_include ],
    dependencies : impl_deps
)

support = declare_dependency(
    link_with : libsupport,
    include_directories : support_include,
    dependencies : [ core, os ]
)
