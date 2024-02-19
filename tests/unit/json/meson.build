exe = executable('json_c', 'json.c',
    dependencies : [ unit, json, io, notify ]
)

test('test_json_c', exe, suite : 'unit')

if has_cpp
    exe = executable('json_cxx', 'json.cpp',
        dependencies : [ unit, json, io, notify ]
    )

    test('test_json_cxx', exe, suite : 'unit')
endif
