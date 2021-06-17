# test target quad parsing
foreach back : [ 'auto', 'c', 'local', 'gcc', 'llvm', 'qbe' ]
    foreach arch : [ 'auto', 'i8086', 'x86', 'x64' ]
        foreach fmt : [ 'elf', 'coff' ]
            foreach mode : [ 'exec', 'static', 'shared' ]
                name = back + '-' + arch + '-' + fmt + '-' + mode
                test(name, driver, args : [ '--target=' + name ], suite : 'quad-parsing')
            endforeach
        endforeach
    endforeach
endforeach

# test help command
test('help', driver, args : [ '--help' ], suite : 'help')

# list all quads
test('quads', driver, args : [ '--quads' ], suite : 'help')

# tests that are expected to pass
pass = [ 'hello', 'binary', 'return' ]

foreach case : pass
    path = meson.current_source_dir()/'pass'/case + '.ct'
    test('pass-' + case, driver, 
        args : [ path, '--target=gcc-x64-elf-exec' ], 
        suite : 'compile'
    )
endforeach

# tests that are expected to fail
fail = [ ]

foreach case : fail
    path = meson.current_source_dir()/'fail'/case + '.ct'
    test('fail-' + case, driver, 
        args : [ path, '--target=gcc-x64-elf-exec' ],
        suite : 'compile', 
        should_fail : true
    )
endforeach
