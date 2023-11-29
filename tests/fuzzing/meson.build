fuzzers = {}

foreach name, lib : langs
    interface = executable(name, 'interface/main.c',
        dependencies : [ lib, emit ]
    )

    fuzzers += { name: interface }
endforeach
