# Language drivers
What are they and how do I make one.

## What are they
A language driver consists of a language parser, typechecker, and ast transformation. A language driver is responsible for compiling its own source text and turning that source into the common hlir format that the cthulhu framework uses.

## How do i make one
A language driver starts by initializing the common runtime, then itself, then passing information about itself into the common main function. This can be done in a short main method such as 
```c
#include "cthulhu/driver/driver.h"

static driver_t DRIVER = {
    .name = "MyLanguage",
    .version = "major.minor.patch",
    .parse = my_parse_function,
    .sema = my_typecheck_function
};

int main(int argc, const char **argv) {
    // init the common runtime, always do this first
    common_init();

    // perform initialization that your language requires
    // init globals, log data, etc

    // run the comon main function and return the exit code
    return common_main(argc, argv, DRIVER);
}
```

Now you need to implement `my_parse_function` and `my_typecheck_function`. TODO