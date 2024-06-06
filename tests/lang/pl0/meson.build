
langtests += {
    'pl0': {
        'ext': 'pl0',

        'tests': {
            'consts': {
                'pass': {
                    'const initialization dependency order': 'chain-init',
                    'case insensitive symbol resolution': 'case-insensitive-lookup',
                    '> 1 consts': 'many-elements',
                    '1 const': 'single-element'
                },
                'fail': {
                    'duplicate case insensitive names': 'case-insensitive',
                    'recursive initialization in a subexpression': 'complex-loop',
                    'duplicate global names': 'duplicate-names',
                    'muturally recursive const init': 'loop-init',
                    'recursive const init': 'simple-loop-init'
                }
            },
            'values': {
                'pass': {
                    'multiple globals': 'multiple-names',
                    'one global': 'single-name'
                },
                'fail': {
                    'case insensitive duplicate symbol': 'case-insensitive-duplicate',
                    'duplicate symbol': 'duplicate-name',
                    'global shadows const': 'shadow-const',
                }
            },
            'procedures': {
                'pass': {
                    'procedure call': 'call',
                    'mutually recursive procedures': 'mutual-recurse',
                    'recursive procedure': 'recurse',
                },
                'fail': {
                    'case insensitive duplicate procedure': 'case-insensitive',
                    'duplicate procedure name': 'proc-shadow',
                    'procedure name shadows const': 'shadow-const',
                    'local variable shadows global': 'shadow-local',
                    'procedure name shadows global': 'shadow-var',
                }
            },
            'programs': {
                'pass': {
                    'calculate primes': 'prime',
                    'square': 'square',
                    'empty': 'empty',
                },
                'fail': {
                    'crash': 'crash',
                }
            }
        },
        'crashes': {
            '0': false, '1': false, '2': false,
            '3': false, '4': false, '5': false, '6': false,
            '7': false, '8': false, '9': false, '10': false,
            '11': false, '12': false, '13': false, '14': false,
            '15': false, '16': false,
            '17': false, '18': false,
            '19': false, '20': true, '21': false, '22': false,
            '23': false, '24': false, '25': false, '26': false,
            '27': true, '28': true, '29': true, '30': false,
            '31': false
        },
        'modules': {
            'simple case': {
                'dir': '0',
                'should_fail': false,
                'files': [ 'lib', 'main' ],
            },
            'muturally recursive imports': {
                'dir': '1',
                'should_fail': false,
                'files': [ 'lhs', 'rhs', 'main' ]
            },
            'multiple entry points': {
                'dir': 'multiple-entries',
                'should_fail': true,
                'files': [ 'main1', 'main2' ]
            }
        }
    }
}
