cataplasm [wip]
====

Cataplasm is a small test framework for C++11 that emulates the core functionality of 
[Catch](https://github.com/philsquared/Catch).

- Single header file.
- ~720 loc
- Full expansion of expressions.
- Catch-style section execution, each 'leaf section' is run exactly once per TEST_CASE.

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
