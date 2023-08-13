dir = meson.current_source_dir()

tests = {
    # 'attribs': {
    #     'pass': [
    #         'entry-point',
    #         'multi-attrib-array',
    #         'multi-attrib',
    #         'single-attrib-array'
    #     ],
    #     'fail': [
    #         'duplicate-attribs',
    #         'entry-var',
    #         'imported-entry',
    #         'invalid-attribute',
    #         'multiple-entry'
    #     ]
    # },
    # 'digits': {
    #     'pass': [
    #         'no-suffix',
    #         'suffix-zoo',
    #         'complex-init'
    #     ],
    #     'fail': [
    #         'invalid-suffix'
    #     ]
    # },
    # 'funcs': {
    #     'fail': [
    #         'ret-invalid',
    #         'dup-arg'
    #     ],
    #     'pass': [
    #         'decl-arg',
    #         'empty-body',
    #         'empty',
    #         'int-return',
    #         'many-args',
    #         'return-int',
    #         'type-return',
    #         'use-arg',
    #         'infinite-loop',
    #         'loop-else',
    #         'pass-ptr'
    #     ]
    # },
    # 'logic.loops': {
    #     'fail': [
    #         'loop-nonbool-expr',
    #         #'invalid-break-label',
    #         #'incorrect-break-label'
    #     ],
    #     'pass': [
    #         'loop',
    #         'for',
    #         'break'
    #         #'break-label',
    #         #'nested-break'
    #     ]
    # },
    # 'programs': {
    #     'pass': [
    #         'alias-var'
    #     ],
    #     'fail': []
    # },
    # 'types.alias': {
    #     'fail': [
    #         'discard-alias-name',
    #         'infinite-size-alias',
    #         'looped-pointer-alias',
    #         'multiple-pointer-loops',
    #         'duplicate-names',
    #         'pointer-loop'
    #     ],
    #     'pass': [
    #         'chained-aliases',
    #         'index-alias',
    #         'multiple-aliases',
    #         'single-alias'
    #     ]
    # },
    'types.struct': {
        'fail': [
            'alias-to-self',
            'infinite-size',
            'no-elements',
            'discard-struct',
            'duplicate-element'
        ],
        'pass': [
            'multiple-elements',
            'mutural-recursion',
            'recursive',
            'single-element',
            'discard-element'
        ]
    },
    # 'types.union': {
    #     'fail': [
    #         'discard-union',
    #         'no-elements',
    #         'duplicate-element',
    #         'infinite-size'
    #     ],
    #     'pass': [
    #         'multiple-elements',
    #         'single-element'
    #     ]
    # },
    # 'types.closure': {
    #     'fail': [
    #         'void-param'
    #     ],
    #     'pass': [
    #         'no-params',
    #         'one-param',
    #         'many-params',
    #         'return-closure'
    #     ]
    # },
    # 'types.variant': {
    #     'fail': [
    #         'duplicate-name',
    #         'discard-name'
    #     ],
    #     'pass': [
    #         'many-elements',
    #         'one-element',
    #         'no-elements'
    #     ]
    # },
    # 'types.array': {
    #     'fail': [
    #         'non-int-size'
    #     ],
    #     'pass': [
    #         'simple',
    #         '2d',
    #         'expr-size'
    #     ]
    # },
    # 'varargs': {
    #     'fail': [
    #         'forward-varargs',
    #         'only-varargs',
    #         'return-varargs',
    #         'vararg-global'
    #     ],
    #     'pass': []
    # },
    'variables': {
        'fail': [
            'bad-type',
            'invalid-init',
            'invalid',
            'newtype-init',
            'noinit-local',
            'noinit-notype',
            'recursive'
        ],
        'pass': [
            #'alias-init',
            'local-noinit',
            'uninit',
            'globals'
        ]
    }
}

crashes = {
    '0': false,
    '1': false,
    '2': false,
    #'3': true,
    '5': false,
    '6': false,
    #'7': true,
    '8': false,
    #'9': true,
    '10': false,
    '11': false,
    #'12': true,
    '13': false,
    '14': false,
    '15': false,
    '16': false,
    'discard-mod': false
}

cases = {
    # 'imports': {
    #     'fails': {
    #         'missing-import': [ 'main' ],
    #         'duplicate-name': [ 'main', 'lib', 'subdir/lib' ],
    #         'duplicate-import': [ 'main', 'lib' ],
    #         'private-symbol': [ 'main', 'lib' ]
    #     },
    #     'passes': {
    #         'no-imports': [ 'main' ],
    #         'single-import': [ 'main', 'lib' ],
    #         'circular-import': [ 'main', 'left', 'right' ]
    #     }
    # }
}

foreach case, tests : tests
    passes = tests['pass']
    fails = tests['fail']

    where = case.replace('.', '/')

    foreach name : passes
        path = dir/where/'pass'/name + '.ct'
        id = 'pass-' + case + '-' + name
        test(id, harness,
            args : [ id, path ],
            suite : 'ctu'
        )
    endforeach

    foreach name : fails
        path = dir/where/'fail'/name + '.ct'
        id = 'fail-' + case + '-' + name
        test(id, harness,
            args : [ id, path ],
            suite : 'ctu',
            should_fail : true
        )
    endforeach
endforeach

foreach name, data : cases
    fails = data['fails']
    passes = data['passes']

    foreach id, src : fails
        root = dir/name/'fail'/id
        sources = []

        foreach file : src
            sources += [ root/file + '.ct' ]
        endforeach

        id = 'fail-' + name + '-' + id
        test(id, harness,
            args : [ id ] + sources,
            suite : 'ctu',
            should_fail : true
        )
    endforeach

    foreach id, src : passes
        root = dir/name/'pass'/id
        sources = []

        foreach file : src
            sources += [ root/file + '.ct' ]
        endforeach

        id = 'pass-' + name + '-' + id
        test(id, harness,
            args : [ id ] + sources,
            suite : 'ctu'
        )
    endforeach
endforeach

foreach case, success : crashes
    path = dir/'crashes'/case + '.ct'
    id = 'crash-' + case
    test(id, harness,
        args : [ id, path ],
        suite : 'ctu',
        should_fail : not success
    )
endforeach
