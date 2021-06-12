links = [ '-lgccjit' ]

gcc = library('gcc-backend', 'gcc.c',
    link_args : links
)

backends += declare_dependency(
    link_args : links,
    link_with : gcc
)
