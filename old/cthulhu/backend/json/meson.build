src = [ 'json.c' ]

cjson = subproject('cjson').get_variable('json')

libjson = library('json', src,
    include_directories : '.',
    dependencies : [ generic, cjson ]
)

options += declare_dependency(
    link_with : libjson,
    include_directories : '.'
)
