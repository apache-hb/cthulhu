project('cJSON', 'c')

src = [ 'cJSON.c' ]

libjson = library('json', src)

json = declare_dependency(
    link_with : libjson,
    include_directories : '.'
)
