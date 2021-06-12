add_languages('cpp')

llvm = dependency('llvm')

deps = [ llvm, middle ]

libllvm = library('llvm-backend', 'llvm.cpp',
    dependencies : deps
)

backends += declare_dependency(
    dependencies : deps,
    link_with : libllvm
)
