# TODO items

## Urgent items

* run compiler on aws lambda
  * wasm backend
  * port to lambda if needed

## Blocking items

* add wasm support
  * run wat2wasm on the output is not great

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

* get this all running inside aws lambda
  * i dont know how s3 buckets work
  * i am too scared of spending my life savings in 5 seconds

* general code cleanup
  * this is a forever task

## Optional items

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

* make util functions more verbose
  * its getting a bit hard to reason about what some functions are doing based on name alone now

* properly prefix the entire compiler
  * will make writing plugins easier in the future
