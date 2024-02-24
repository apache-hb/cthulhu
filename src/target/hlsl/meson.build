src = [ 'src/target.c' ]

libtarget_hlsl = static_library('target_hlsl', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : [ target ]
)

targets += {
    'hlsl': {
        'dep': declare_dependency(link_with : libtarget_hlsl),
        'mod': 'kTargetHlsl'
    }
}
