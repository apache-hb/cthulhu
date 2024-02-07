exe = executable('test_json', 'test_json.c',
    dependencies : [ unit, json, io, notify ]
)

test('test_json', exe, suite : 'unit')

if has_cpp
    exe = executable('test_json_cxx', 'test_json.cpp',
        dependencies : [ unit, json, io, notify ]
    )

    test('test_json_cxx', exe, suite : 'unit')
endif

