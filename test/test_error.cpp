#include <iostream>
#include "diplib/library/error.h"

// Testing exceptions and related macros.
// When compiling with `cmake -DENABLE_ASSERT=ON ..`, the program should exit with a `dip::AssertionError` exception.
// When compiling with `cmake -DENABLE_ASSERT=OFF ..`, the program should exit with a `dip::ParameterError` exception.

void test2() {
   DIP_THROW("A test case");
};

void test1() {
   DIP_START_STACK_TRACE
   test2();
   DIP_END_STACK_TRACE
}

int main() {
   DIP_START_STACK_TRACE
   DIP_ASSERT( 6 > 2 );
   DIP_ASSERT( 6 < 2 );
   test1();
   DIP_END_STACK_TRACE
   return 0;
}
