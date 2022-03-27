project('cJSON', 'c',
    version : '1.5.17'
)

args = []

if host_machine.system() == 'windows'
    args += [ '/w0' ]
endif

libjson = library('json', [ 'cJSON.c' ],
    c_args : args
)

json = declare_dependency(
    link_with : libjson,
    include_directories : '.'
)
