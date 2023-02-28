fuzzers = {}

foreach name, lib : langs
    interface = executable(name, 'interface/main.c',
        dependencies : [ generic, lib, emit ]
    )

    fuzzers += { name: interface }
endforeach
