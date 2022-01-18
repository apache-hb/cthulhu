project('cjson', 'c')

libjson = library('json', 'cJSON.c')

json = declare_dependency(
    link_with : libjson,
    include_directories : '.'
)
