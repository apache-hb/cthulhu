libunit = library('unit', 'unit/ct-test.cpp',
    include_directories : include_directories('unit'),
    dependencies : [ base, std ]
)

unit = declare_dependency(link_with : libunit)

cases = [
    'util.str',
    'util.map',
    'util.set'
]

foreach case : cases
    name = case.replace('.', '/')
    path = name + '.cpp'

    exe = executable(case, path,
        include_directories : '.',
        dependencies : [ unit, base ]
    )

    test(case, exe, suite : 'unit')
endforeach
