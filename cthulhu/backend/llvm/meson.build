src = [ 'llvm.c' ]

llvm = dependency('llvm')

libllvm = library('llvm', src,
    include_directories : '.',
    dependencies : [ generic, llvm ]
)

options += declare_dependency(
    link_with : libllvm,
    include_directories : '.',
    dependencies : llvm
)
