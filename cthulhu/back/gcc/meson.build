gccjit = dependency('gccjit', required : get_option('gcc'))

if gccjit.found()
    gcc = library('gcc-backend', 'gcc.c',
        dependencies : gccjit
    )

    backends += declare_dependency(
        dependencies : gccjit,
        link_with : gcc
    )
else
    backends += declare_dependency(
        link_with : library('gcc-fallback', 'fallback.c')
    )
endif
