src = [ 'scan.c', 'driver.c' ]

libcc = library('c', src,
    dependencies : generic
)

options += declare_dependency(
    link_with : libcc,
    include_directories : '.'
)
