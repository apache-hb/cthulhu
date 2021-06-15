cc = meson.get_compiler('c')
gccjit = cc.find_library('gccjit', required : get_option('gcc'))

if gccjit.found()
    gcc = library('gcc-backend', 'gcc.c',
        dependencies : [ gccjit, middle ]
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
