langtests += {
    'obr': {
        'ext': 'obr',

        'tests': {
            'values': {
                'pass': {
                    'single value': 'single-single',
                    'multiple values in the same type declaration': 'single-multi',
                    'mutiple values with multiple type declarations': 'multi-single',
                    'multiple values with multiple type declarations': 'multi-multi',
                    'constant value': 'const-value'
                }
            },
            'warns': {
                'pass': {
                    'mismatching module begin and end blocks': 'mod'
                }
            },
            'functions': {
                'pass': {
                    'procedure without return': 'no-return',
                    'function with return value': 'return-val',
                    'function with void return': 'return-void'
                },
                'fail': {
                    'function without return': 'return-type',
                    'function with empty return': 'void-return',
                    'procedure with value return': 'void-return2'
                }
            },
            'types': {
                'pass': {
                    'alias type': 'alias',
                    'built-in types': 'builtins',
                    'nested type': 'nested',
                    'record type': 'records',

                    'array type': 'arrays',
                    'nested array type': 'nested-arrays'
                },
                'fail': {
                    'invalid type': 'invalid-type',
                    'recursive pointer type': 'recursive-pointer',
                    'recursive type': 'recursive',

                    'array type with boolean length': 'array-length-bool',
                    'array type with non-constant length': 'array-length-not-const',
                    'array type with zero length': 'array-size-zero',
                    'array type with negative length': 'array-length-wrong-sign',
                    'array type with too big length': 'array-length-too-big'
                }
            }
        },
        'modules': {
            'simple': {
                'dir': 'simple',
                'should_fail': false,
                'files': [ 'test' ]
            }
            # TODO: add single when IO module works
        }
    }
}
