
util = [ 
    # sanity checks
    'util/pool', 'util/sstream', 
    # file streams
    'util/fstream/empty', 'util/fstream/hello'
]

lex = [
    # sanity checks
    'lex/end', 'lex/ident', 
    
    # keywords
    'lex/keys/key', 'lex/keys/all',

    # raw strings
    'lex/strings/rstring1', 'lex/strings/rstring2',
    
    # single line strings
    'lex/strings/string1', 'lex/strings/encode',

    # string interning
    'lex/intern/idents', 'lex/intern/mixed', 'lex/intern/strings',

    # TODO: this test is bugged due to https://github.com/DuffsDevice/tiny-utf8/issues/58
    # leave it disabled until thats resolved
    # 'lex/rstring3'

    # diagnostic messages
    'lex/diagnostic1', 'lex/diagnostic2',

    'lex/chars/char', 'lex/symbols/symbols', 'lex/symbols/dots',
    'lex/symbols/templates', 'lex/symbols/all',

    'lex/numbers/base2', 'lex/numbers/base10', 'lex/numbers/base16', 
    'lex/numbers/mixed', 'lex/numbers/suffix'
]

parse = [
    'parse/types/name', 'parse/types/qual', 'parse/types/pointer',
    'parse/types/var1', 'parse/types/var2',
    'parse/types/closure1', 'parse/types/closure2',
    'parse/types/array1', 'parse/types/array2',

    'parse/expr/bool'
]

tests = util + lex + parse

foreach name : tests
    tname = '.'.join(name.split('/'))
    exe = executable(tname, name + '.cpp', cpp_args : args, dependencies : ctu)
    test(tname, exe, workdir : meson.current_source_dir())
endforeach
