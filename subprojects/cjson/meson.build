project('cJSON', 'c',
    version : '1.5.17'
)

args = []

libjson = library('json', [ 'cJSON.c' ],
    c_args : args
)

json = declare_dependency(
    link_with : libjson,
    include_directories : '.'
)
