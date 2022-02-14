project('aws', 'c')

curl = dependency('libcurl')
cjson = subproject('cjson').get_variable('cjson')

deps = [ curl, cjson ]
src = [ 'aws/runtime.c', ]

libaws = library('aws-runtime', src,
    dependencies : deps
)

aws = declare_dependency(
    link_with : libaws,
    include_directories : '.',
    dependencies : deps
)
