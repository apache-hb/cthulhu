src = [ 'src/target.c' ]

libtarget_hlsl = library('target_hlsl', src,
    build_by_default : false,
    include_directories : 'include',
    dependencies : [ target ]
)

targets += {
    'hlsl': {
        'dep': declare_dependency(link_with : libtarget_hlsl),
    }
}
