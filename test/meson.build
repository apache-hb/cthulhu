
util = [ 
    # sanity checks
    'util/pool', 'util/sstream', 
    # file streams
    'util/fstream/empty', 'util/fstream/hello'
]

lex = [
    # sanity checks
    'lex/end', 'lex/ident', 'lex/key',

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

    'lex/chars/char', 'lex/symbols'
]

tests = util + lex

foreach name : tests
    tname = '.'.join(name.split('/'))
    exe = executable(tname, name + '.cpp', cpp_args : args, dependencies : ctu)
    test(tname, exe, workdir : meson.current_source_dir())
endforeach
