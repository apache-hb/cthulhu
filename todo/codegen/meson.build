src = [
    'src/codegen.c'
]

libcodegen = library('codegen', src,
    include_directories : 'include',
    dependencies : [ base, std, memory, io ]
)

codegen = declare_dependency(
    link_with : libcodegen,
    include_directories : 'include'
)

src = [
    'src/main.c'
]

executable('tool_codegen', src,
    dependencies : [ base, std, memory, io, codegen ]
)
