project('ctu', 'c',
    default_options : [
        'c_std=gnu99',
        'cpp_std=gnu++17',
        'warning_level=2',
        'default_library=static',
        'werror=true'
    ]
)

llvm = get_option('llvm-backend')
gcc = get_option('gcc-backend')
qbe = get_option('qbe-backend')
fuzzing = get_option('fuzzing')

config = configuration_data()

config.set10('CTU_LLVM', llvm)
config.set10('CTU_GCC', gcc)
config.set10('CTU_QBE', qbe)
config.set10('CTU_FUZZING', fuzzing)

configure_file(output : 'config.h',
    configuration : config
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

src = [ 
    'cthulhu/front/ast.c', 
    'cthulhu/front/compile.c',
    'cthulhu/front/debug/debug.c'
]

# turns text/files into an ast
libfront = library('ast', lex, parse, src,
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

src = [
    'cthulhu/middle/ir.c',
    'cthulhu/middle/debug/debug.c'
]

libmiddle = library('middle', src,
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
