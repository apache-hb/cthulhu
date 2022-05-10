# TODO items

## Urgent items

* wasm backend
  * this is going to need ssa

* s3 api

## Blocking items

* rework logging 
  * need custom sinks for aws lambda to be usable
  * will these be intergrated with interfaces or a seperate entity

* document more stuff
  * other people have to contribute now

* extract most of `generic` into subprojects
  * better seperation of code into more logical blocks
  * makes it easier to generate variations of the compiler

* create plugin system
  * this would allow our secret sauce to be kept closed source
  * while the majority of the framework would be open source
    * other people can fix our bugs for us
    * brownie points for being open source
    * means we can use libraries like libgccjit
  * this would also allow for codegen shims to be added in
    * makes gathering metrics possible and exensible

* general code cleanup
  * this is a forever task

## Optional items

* better debug tools

* more test coverage

* dedicated fuzzing frontends

* add ssa emitter
  * direct asm/wasm output would be preferrable to relying on C/wat2wasm

* optimisation passes
  * more speed never hurts
  * would get us on the compiler framework map so to speak
  * would save us money in the long run with lambda runtimes

* static analysis support
  * makes it easier to find code defects
  * helps optimisations
  * if we get a C frontend it will help us find defects in the compiler itself

* properly prefix the entire compiler
  * will make writing plugins easier in the future
