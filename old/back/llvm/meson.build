add_languages('cpp')
llvm = dependency('llvm', 
    required : get_option('llvm'), 
    static : true
)

if llvm.found()
    deps = [ llvm, middle ]

    libllvm = library('llvm-backend', 'llvm.cpp',
        dependencies : deps
    )

    backends += declare_dependency(
        dependencies : deps,
        link_with : libllvm
    )
else
    backends += declare_dependency(
        link_with : library('llvm-fallback', 'fallback.c',
            dependencies : [ util ]
        )
    )
endif
