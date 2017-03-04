cataplasm [wip]
====

Cataplasm is a tiny test framework for C++11 that emulates the core functionality of 
[Catch](https://github.com/philsquared/Catch).

- Single header file ~10% the size of Catch's single-header.
- Generates small executables.
- Full expansion of expressions.
- Catch-style section execution, each 'leaf section' is run exactly once per TEST_CASE.

Test Macros
----

### Blocks
- **TEST_CASE** (*name*, *tags*) - define a new test case; name and tags are both optional.
- **SECTION** (*name*, *tags*) - define a new section within a test case; name and tags are both optional.

### Assertions
- **ENSURE** (*expression*) - succeeds only if *expression* is true. Halts test on failure.
- **VERIFY** (*expression*) - succeeds only if *expression* is true.
- **FORBID** (*expression*) - succeeds only if *expression* is **not** true. Halts test on failure.
- **REJECT** (*expression*) - succeeds only if *expression* is **not** true.

### Conditional assertions
- **IF_VERIFY** (*expression*) - execute the following block only if *expression* is true.
- **IF_REJECT** (*expression*) - execute the following block only if *expression* is **not** true.

### Exceptions
- **THROWS** (*expression*) - succeeds only if *expression* throws an exception.
- **NO_THROWS** (*expression*) - succeeds only if *expression* does not throw an exception.
- **THROWS_AS** (*expression*, *exception*) - succeeds only if *expression* throws the specified *exception*.

### Special macros
- **PASS** (*expression*) - always succeeds.
- **FAIL** (*expression*) - always fails.
- **NOTICE** (*expression*) - prints to stdout.
- **WARN** (*expression*) - prints to stdout in *bright red*.

### Catch conversions
Defining 'CATAPLASM_CATCH' before including cataplasm will enable redefinition of the following Catch macros:

| Catch | Cataplasm |
|---------|--------|
| REQUIRE | ENSURE |
| REQUIRE_FALSE | FORBID |
| REQUIRE_THROWS | THROWS |
| REQUIRE_THROWS_AS | THROWS_AS |
| REQUIRE_NOTHROW | NO_THROW |
| CHECK | VERIFY |
| CHECK_FALSE | REJECT |
| CHECKED_IF | VERIFY_IF |
| INFO | INFO |
| WARN | WARN |
| SCENARIO | TEST_CASE("Scenario: " name, tags) |
| GIVEN | SECTION("   Given: " desc, "") |
| WHEN | SECTION("    When: " desc, "") |
| AND_WHEN | SECTION("And when: " desc, "") |
| THEN | SECTION("    Then: " desc, "") |
| AND_THEN | SECTION("     And: " desc, "") |

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
