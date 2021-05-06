parse = [
    [ 'parse.var1', true ],
    [ 'parse.var2', true ],
    [ 'parse.var3', true ],
    [ 'parse.var4', true ],
    [ 'parse.var5', false ],
    [ 'parse.var6', false ]
]

sema = [
    [ 'sema.var1', true ],
    [ 'sema.var2', true ],
    [ 'sema.var3', false ],
    [ 'sema.unary1', false ],
    [ 'sema.unary2', false ],
    [ 'sema.unary3', true ],
    [ 'sema.unary4', true ],
    [ 'sema.unary5', false ],
    [ 'sema.unary6', false ],
    [ 'sema.binary1', true ],
    [ 'sema.binary2', false ],
    [ 'sema.binary3', false ],
    [ 'sema.binary4', true ],
    [ 'sema.binary5', false ],
    [ 'sema.binary6', false ],
    [ 'sema.ref1', false ],
    [ 'sema.deref1', true ],
    [ 'sema.deref2', false ],
    [ 'sema.deref3', false ],
    [ 'sema.export1', true ],
    [ 'sema.export2', true ],
]

func = [
    [ 'func.empty1', true ],
    [ 'func.empty2', true ],
    [ 'func.empty3', true ],
    [ 'func.recurse1', true ],
    [ 'func.arg1', true ],
    [ 'func.arg2', false ],
    [ 'func.arg3', true ],
    [ 'func.arg4', true ],
    [ 'func.arg5', false ],
]

foreach case : parse + sema + func
    name = case[0]
    pass = case[1]
    parts = name.split('.')

    path = name.replace('.', '/') + '.ct'

    test(parts[1], ctc, 
        suite : parts[0],
        workdir : meson.current_source_dir(),
        args : [ path ], 
        should_fail : not pass
    )
endforeach
