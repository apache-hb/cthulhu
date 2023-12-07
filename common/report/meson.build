src = [
    'src/report/report.c',
    'src/report/report-ext.c'
]

libreport = library('report', src,
    c_args : user_args,
    dependencies : [ scan, std, memory ],
    include_directories : 'include'
)

report = declare_dependency(
    link_with : libreport,
    dependencies : [ core, base, os ],
    include_directories : 'include'
)
