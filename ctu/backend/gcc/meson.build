src = [ 'gcc.c', 'type.c', 'value.c' ]

libgcc = library('gcc', src,
    include_directories : '.',
    link_args : [ '-lgccjit' ],
    dependencies : generic
)

options += declare_dependency(
    link_with : libgcc,
    link_args : [ '-lgccjit' ],
    include_directories : '.'
)
