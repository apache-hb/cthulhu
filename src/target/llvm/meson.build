llvm = dependency('llvm', components : ['core'], method : 'config-tool', static : true)
llvm_libdir = llvm.get_variable(configtool: 'libdir')

deps = [
    target, fs, io, events, std, tree, ssa, base,

    llvm
]

src = [
    'src/ssa.c',
    'src/target.c'
]

libtarget_llvm = static_library('target_llvm', src,
    build_by_default : not meson.is_subproject(),
    install : not meson.is_subproject(),
    include_directories : 'include',
    dependencies : deps,
    override_options : [ 'b_vscrt=mt' ],
)

targets += {
    'llvm': {
        'dep': declare_dependency(link_with : libtarget_llvm),
        'mod': 'kTargetLLVM',

        'static': libtarget_llvm,
        'module': 'kTargetLLVM'
    }
}
