# Cthulhu {#mainpage}
Wow this project got out of hand.

## A brief overview

Born out of frustrating with existing tooling.
Cthulhu is a set of tools for building compilers. It is designed to be as
low friction as possible to get started with and easily extensible.

Started in 2019, Cthulhu now contains 2 complete language frontends
    * [PL0](https://en.wikipedia.org/wiki/PL/0)
    * Cthulhu. The namesake language, a C like language with a few extra features.

And 1 language frontend in progress
    * [Oberon-2](https://en.wikipedia.org/wiki/Oberon-2)
        * Currently has partial support, but is missing the standard library and object oriented features.

## Compelling features

Currently cthulhu is the only compiler (to my knowledge) that supports
language interop between languages without boilerplate.
This is achieved by using common interfaces between language drivers.
    * All drivers follow the same compilation phases, allowing for symbol visibility.
    * All languages use the same intermediate form, called `tree`. allowing for interop between languages.
        * This even includes languages that follow different type systems.
        * This even allows for interop between language runtimes, for example a program written in Cthulhu may call out to a function available in the JVM runtime.

As alot of code can be shared, adding a feature for one language may often require no work for other languages to implement.

## More information

[Build instructions](@ref building).

[Contribution guide](@ref contributing).
