langtests += {
    'ctu': {
        'ext': 'ct',

        'tests': {
            'attribs': {
                'pass': {
                    'single entry point': 'entry-point',
                    'multiple attributes in a single attribseq': 'multi-attrib-array',
                    'multiple attributes': 'multi-attrib',
                    'single attribute in array': 'single-attrib-array'
                },
                'fail': {
                    'duplicate attributes': 'duplicate-attribs',
                    'entry point with variable': 'entry-var',
                    'imported entry point': 'imported-entry',
                    'invalid attribute': 'invalid-attribute',
                    'multiple entry points': 'multiple-entry'
                }
            },
            'digits': {
                'pass': {
                    'no suffix': 'no-suffix',
                    #'suffix zoo': 'suffix-zoo',
                    'complex init': 'complex-init'
                },
                'fail': {
                    'invalid suffix': 'invalid-suffix'
                }
            },
            'funcs': {
                'fail': {
                    'return invalid type': 'ret-invalid',
                    'duplicate argument': 'dup-arg'
                },
                'pass': {
                    'single argument': 'decl-arg',
                    'empty body': 'empty-body',
                    'empty function': 'empty',
                    'integer return': 'int-return',
                    'many arguments': 'many-args',
                    'return integer': 'return-int',
                    'return type': 'type-return',
                    'use argument': 'use-arg',
                    'infinite loop': 'infinite-loop',
                    'loop else': 'loop-else',
                    'pass pointer': 'pass-ptr'
                }
            },
            'logic/loops': {
                'fail': {
                    'loop with non-boolean expression': 'loop-nonbool-expr',
                    'invalid break label': 'invalid-break-label',
                    'incorrect break label': 'incorrect-break-label'
                },
                'pass': {
                    'loop': 'loop',
                    'for': 'for',
                    'break': 'break',
                    'break with label': 'break-label',
                    'nested break': 'nested-break'
                }
            },
            'logic/return': {
                'pass': {
                    'boolean return': 'bool-return'
                },
                'fail': {
                    'missing return': 'missing-return'
                }
            },
            'globals': {
                'pass': {
                    'global string': 'global-string'
                }
            },
            'programs': {
                'pass': {
                    'alias variable': 'alias-var'
                }
            },
            'types/alias': {
                'fail': {
                    'discard alias name': 'discard-alias-name',
                    'infinite size alias': 'infinite-size-alias',
                    'looped pointer alias': 'looped-pointer-alias',
                    'multiple pointer loops': 'multiple-pointer-loops',
                    'duplicate names': 'duplicate-names',
                    'pointer loop': 'pointer-loop'
                },
                'pass': {
                    'chained aliases': 'chained-aliases',
                    'index alias': 'index-alias',
                    'multiple aliases': 'multiple-aliases',
                    'single alias': 'single-alias'
                }
            },
            'types/struct': {
                'fail': {
                    'alias to self': 'alias-to-self',
                    'infinite size': 'infinite-size',
                    'no elements': 'no-elements',
                    'discard struct': 'discard-struct',
                    'duplicate element': 'duplicate-element'
                },
                'pass': {
                    'multiple elements': 'multiple-elements',
                    'mutural recursion': 'mutural-recursion',
                    'recursive': 'recursive',
                    'single element': 'single-element',
                    'discard element': 'discard-element',
                    'use struct': 'use-struct'
                }
            },
            'types/union': {
                'fail': {
                    'discard union': 'discard-union',
                    'no elements': 'no-elements',
                    'duplicate element': 'duplicate-element',
                    'infinite size': 'infinite-size'
                },
                'pass': {
                    'multiple elements': 'multiple-elements',
                    'single element': 'single-element'
                }
            },
            'types/closure': {
                'fail': {
                    'void parameter': 'void-param'
                },
                'pass': {
                    'no parameters': 'no-params',
                    'one parameter': 'one-param',
                    'many parameters': 'many-params',
                    'return closure': 'return-closure'
                }
            },
            'types/variant': {
                'fail': {
                    'duplicate name': 'duplicate-name',
                    'discard name': 'discard-name',
                    'invalid underlying': 'invalid-underlying'
                },
                'pass': {
                    'many elements': 'many-elements',
                    'one element': 'one-element',
                    'no elements': 'no-elements',
                    'underlying': 'underlying',
                    'use variant': 'use-variant'
                }
            },
            'types/array': {
                'fail': {
                    'non-integer size': 'non-int-size'
                },
                'pass': {
                    'simple': 'simple',
                    '2D': '2d',
                    'expression size': 'expr-size'
                }
            },
            'varargs': {
                'fail': {
                    #'forward varargs': 'forward-varargs',
                    'only varargs': 'only-varargs',
                    'return varargs': 'return-varargs',
                    'vararg global': 'vararg-global'
                },
                'pass': {
                    'extern varargs': 'extern-varargs'
                }
            },
            'variables': {
                'fail': {
                    'bad type': 'bad-type',
                    'invalid init': 'invalid-init',
                    'invalid': 'invalid',
                    'newtype init': 'newtype-init',
                    'no init local': 'noinit-local',
                    'no init no type': 'noinit-notype',
                    'recursive': 'recursive'
                },
                'pass': {
                    'alias init': 'alias-init',
                    'local no init': 'local-noinit',
                    'uninit': 'uninit',
                    'globals': 'globals'
                }
            },
            'edgecases': {
                'fail': { },
                'pass': {
                    'struct ordering': 'struct-ordering',
                    'string locals': 'string-locals'
                }
            }
        },
        'crashes': {
            '0': false,
            '1': false,
            '2': false,
            '3': true,
            '5': false,
            '6': false,
            '7': true,
            '8': false,
            '9': false,
            '10': false,
            '11': false,
            '12': true,
            '13': false,
            '14': false,
            '15': false,
            '16': false,
            'discard-mod': false
        },
        'modules': {
            'missing import': {
                'dir': 'missing-import',
                'should_fail': true,
                'files': [ 'main' ]
            },
            'duplicate name': {
                'dir': 'duplicate-name',
                'should_fail': true,
                'files': [ 'main', 'lib', 'subdir/lib' ]
            },
            'duplicate import': {
                'dir': 'duplicate-import',
                'should_fail': true,
                'files': [ 'main', 'lib' ]
            },
            'private symbol': {
                'dir': 'private-symbol',
                'should_fail': true,
                'files': [ 'main', 'lib' ]
            },
            'no imports': {
                'dir': 'no-imports',
                'should_fail': false,
                'files': [ 'main' ]
            },
            'single import': {
                'dir': 'single-import',
                'should_fail': false,
                'files': [ 'main', 'lib' ]
            },
            'circular import': {
                'dir': 'circular-import',
                'should_fail': false,
                'files': [ 'main', 'left', 'right' ]
            }
        }
    }
}
