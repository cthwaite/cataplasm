cataplasm [wip]
====

Cataplasm is a tiny test framework for C++11 that emulates the core functionality of 
[Catch](https://github.com/philsquared/Catch).

- Single header file ~10% the size of Catch's single-header.
- Generates small executables.
- Full expansion of expressions.
- Catch-style section execution, each 'leaf section' is run exactly once per TEST_CASE.
- [cloc](https://github.com/AlDanial/cloc) output:
```
-------------------------------------------------------------------------------
Language                     files          blank        comment           code
-------------------------------------------------------------------------------
C/C++ Header                     1             82             49            686
-------------------------------------------------------------------------------
```

Test Macros
----

- **TEST_CASE** (*name*, *tags*) - define a new test case; name and tags are both optional.
- **SECTION** (*name*, *tags*) - define a new section within a test case; name and tags are both optional.
- **ENSURE** (*expression*) - succeeds only if *expression* is true. Halts test on failure.
- **VERIFY** (*expression*) - succeeds only if *expression* is true.
- **FORBID** (*expression*) - succeeds only if *expression* is **not** true. Halts test on failure.
- **REJECT** (*expression*) - succeeds only if *expression* is **not** true.
- **THROWS** (*expression*) - succeeds only if *expression* throws an exception.
- **NO_THROWS** (*expression*) - succeeds only if *expression* does not throw an exception.
- **THROWS_AS** (*expression*, *exception*) - succeeds only if *expression* throws the specified *exception*.
- **PASS** (*expression*) - always succeeds.
- **FAIL** (*expression*) - always fails.
- **NOTICE** (*expression*) - prints to stdout.
- **WARN** (*expression*) - prints to stdout in *bright red*.

Command-line flags
----
```
USAGE:
./sample [-h] [-e|-v] [-t|-x] TAG1;TAG2;...

Arguments:
        -h        Prints this help message.
        -e        Expand all expressions (also enables verbose mode).
        -t TAGS   Run test blocks tagged with any of the specified tags.
        -x TAGS   Run only test blocks tagged with *all* of the specified tags.
        -v        Use verbose mode, printing the results of all tests.
```

Example
----
A complete program, using one of the Catch examples:

```c++
#define CATAPLASM_MAIN
#include "cataplasm.hpp"

TEST_CASE( "vectors can be sized and resized", "vector" ) {

    std::vector<int> v( 5 );

    ENSURE( v.size() == 5 );
    ENSURE( v.capacity() >= 5 );

    SECTION( "resizing bigger changes size and capacity" ) {
        v.resize( 10 );

        ENSURE( v.size() == 10 );
        ENSURE( v.capacity() >= 10 );
    }
    SECTION( "resizing smaller changes size but not capacity" ) {
        v.resize( 0 );

        ENSURE( v.size() == 0 );
        ENSURE( v.capacity() >= 5 );
    }
    SECTION( "reserving bigger changes capacity but not size" ) {
        v.reserve( 10 );

        ENSURE( v.size() == 5 );
        ENSURE( v.capacity() >= 10 );

        SECTION( "reserving smaller again does not change capacity" ) {
            v.reserve( 7 );

            ENSURE( v.capacity() >= 10 );
        }
    }
    SECTION( "reserving smaller does not change size or capacity" ) {
        v.reserve( 0 );

        ENSURE( v.size() == 5 );
        ENSURE( v.capacity() >= 5 );
    }
}
```
