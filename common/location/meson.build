src = [
    'src/report/report.c',
    'src/report/report-ext.c',

    'src/scan/node.c',
    'src/scan/scan.c',
    'src/scan/compile.c'
]

# this is both libreport and libscan as they depend on each other
liblocation = library('location', src,
    c_args : user_args,
    dependencies : [ std, io, os ],
    include_directories : 'include'
)

location = declare_dependency(
    link_with : liblocation,
    dependencies : [ std, io, os ],
    include_directories : 'include'
)
