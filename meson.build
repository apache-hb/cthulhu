project('ctu', 'c',
    default_options : [
        'c_std=gnu99',
        'warning_level=2',
        'default_library=static',
        'werror=true'
    ]
)

libreport = library('report', 'cthulhu/report/report.c',
    include_directories : '.'
)

report = declare_dependency(
    link_with : libreport,
    include_directories : '.'
)

# create our flex lexer that bison needs
lex = custom_target('lex',
    command : [ find_program('flex'), '@INPUT@' ],
    input : 'cthulhu/front/cthulhu.l',
    output : [ 'flex.c', 'flex.h' ],
    depends : libreport
)

# create our bison parser that our ast needs
parse = custom_target('parse',
    command : [ find_program('bison'), '-d', '@INPUT@', '-Wdeprecated', '-v' ],
    input : 'cthulhu/front/cthulhu.y',
    output : [ 'bison.c', 'bison.h' ],
    depends : [ lex, libreport ]
)

# turns text/files into an ast
libfront = library('ast', lex, parse, [ 'cthulhu/front/front.c', 'cthulhu/front/compile.c' ],
    dependencies : report,
    include_directories : '.'
)

front = declare_dependency(
    link_with : libfront, 
    dependencies : report,
    sources : [ lex, parse ],
    include_directories : '.'
)

libsema = library('sema', [ 'cthulhu/sema/sema.c' ],
    dependencies : report,
    include_directories : '.'
)

# turns an ast into an ir
# - handles typechecking and semantics
# - handles IR optimizations
libmiddle = library('middle', 'cthulhu/middle/ir.c',
    dependencies : [ front, report ],
    include_directories : '.'
)

middle = declare_dependency(
    dependencies : report,
    link_with : libmiddle,
)

# collect all our backends
subdir('cthulhu/back')

# user facing frontend
front = executable('ctc', 'cthulhu/main.c',
    include_directories : '.',
    dependencies : backends + [ middle, report ]
)
