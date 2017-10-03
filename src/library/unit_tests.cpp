/*
 * DIPlib 3.0
 * This file is not part of the DIPlib library, it is the main() for the
 * unit tests program.
 *
 * (c)2017, Cris Luengo.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

// Define this value when compiling the unit_tests program
#ifdef DIP__IMPLEMENT_UNIT_TESTS

#ifdef DIP__DOCTEST_IN_SHARED_LIB

#include "diplib.h"
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

namespace dip {
// Declaration for function in `src/library/unit_tests_shared_lib.cpp`
DIP_EXPORT int run_unit_tests( int argc, const char* const* argv );
}

int main( int argc, const char* const* argv ) {
   doctest::Context context( argc, argv );
   int res = dip::run_unit_tests( argc, argv );
   if( context.shouldExit() ) {  // important - query flags (and --exit) rely on the user doing this
      return res;                // propagate the result of the tests
   }
   return res; // the result from doctest is propagated here as well
}

#else // !DIP__DOCTEST_IN_SHARED_LIB

#include "diplib.h"
#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#endif // DIP__DOCTEST_IN_SHARED_LIB

#endif // DIP__IMPLEMENT_UNIT_TESTS
