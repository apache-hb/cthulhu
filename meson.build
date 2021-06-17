project('ctu', 'c',
    default_options : [
        'c_std=c11', # flex under msvc requires c11
        'cpp_std=c++17', # llvm headers require c++17
        'warning_level=2', # bison generates code that warns on gcc at level 3
        'default_library=static',
        'werror=true'
    ]
)

msvc = meson.get_compiler('c').get_id() == 'msvc'
windows = build_machine.system() == 'windows'

src = [
    'cthulhu/util/util.c',
    'cthulhu/util/report.c'
]

libutil = library('util', src,
    include_directories : '.'
)

util = declare_dependency(
    link_with : libutil,
    include_directories : '.'
)

if windows
    flex = find_program('win_flex', version : '>=2.6')
    bison = find_program('win_bison', version : '>=3.7')
else
    flex = find_program('flex', version : '>=2.6')
    bison = find_program('bison', version : '>=3.7')
endif

command = [ flex ]
if windows
    command += '--wincompat'
endif
command += '@INPUT@'

# create our flex lexer that bison needs
lex = custom_target('lex',
    command : command,
    input : 'cthulhu/front/cthulhu.l',
    output : [ 'flex.c', 'flex.h' ],
    depends : libutil
)

# create our bison parser that our ast needs
parse = custom_target('parse',
    command : [ bison, '-d', '@INPUT@', '-Wdeprecated', '-v' ],
    input : 'cthulhu/front/cthulhu.y',
    output : [ 'bison.c', 'bison.h' ],
    depends : [ lex, libutil ]
)

src = [ 
    'cthulhu/front/ast.c', 
    'cthulhu/front/compile.c',
    'cthulhu/front/debug/debug.c'
]

args = [ '-D_POSIX_C_SOURCE=700' ]

# turns text/files into an ast
libfront = library('ast', lex, parse, src,
    c_args : args,
    dependencies : util,
    include_directories : '.'
)

front = declare_dependency(
    link_with : libfront, 
    dependencies : util,
    sources : [ lex, parse ],
    compile_args : args,
    include_directories : '.'
)

libsema = library('sema', [ 'cthulhu/sema/sema.c' ],
    dependencies : [ util, front ],
    include_directories : '.'
)

sema = declare_dependency(
    link_with : libsema,
    include_directories : '.'
)

src = [
    'cthulhu/middle/ir.c',
    'cthulhu/middle/debug/debug.c'
]

libmiddle = library('middle', src,
    dependencies : [ front, util ],
    include_directories : '.'
)

middle = declare_dependency(
    dependencies : util,
    link_with : libmiddle,
    include_directories : '.'
)

# collect all our backends
subdir('cthulhu/back')

args = [ ]

if msvc
    args += '-D_CRT_SECURE_NO_WARNINGS=1'
endif

# user facing frontend
driver = executable('ctc', 'cthulhu/main.c',
    include_directories : '.',
    c_args : args,
    dependencies : backends + [ sema, middle, util ]
)

if not meson.is_subproject()
    executable('test-runner', 'tools/test.c',
        c_args : '-D_CRT_SECURE_NO_WARNINGS'
    )
    subdir('tests')
endif
