project('aws', 'c')

curl = dependency('libcurl')

src = [ 'aws/runtime.c', ]

libaws = library('aws-runtime', src,
    dependencies : curl
)

aws = declare_dependency(
    link_with : libaws,
    include_directories : '.',
    dependencies : curl
)
