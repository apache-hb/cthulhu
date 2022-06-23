project('cJSON', 'c',
    version : '1.5.17'
)

libjson = library('json', [ 'cJSON.c' ])

json = declare_dependency(
    link_with : libjson,
    include_directories : '.'
)
