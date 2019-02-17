/*
 * DIPlib 3.0
 * This file is not part of the DIPlib library, it is the main() for the
 * unit tests program.
 *
 * (c)2017-2018, Cris Luengo.
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

#ifdef DIP__ENABLE_DOCTEST

// Define this value when compiling the unit_tests program
#ifdef DIP__IMPLEMENT_UNIT_TESTS

#ifdef DIP__DOCTEST_IN_SHARED_LIB

#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#include "doctest.h"

#include "diplib.h"
#include "diplib/linear.h"

DOCTEST_TEST_CASE("[DIPlib] checking that exceptions can be caught outside the shared library") {
   dip::Image img( { 1 }, 1 );
   DOCTEST_CHECK_THROWS_AS( dip::GaussFIR( img, { 1 }, { 0 }, { "illegal BC" } ), dip::ParameterError );
}

int main( int argc, const char* const* argv ) {
   // force the use of a symbol from the DIP shared library so tests from it get registered
   dip::DataType::SuggestInteger( dip::DT_UINT8 );

   doctest::Context context( argc, argv );
   int res = context.run();
   if( context.shouldExit() ) {  // important - query flags (and --exit) rely on the user doing this
      return res;                // propagate the result of the tests
   }
   return res; // the result from doctest is propagated here as well
}

#else // !DIP__DOCTEST_IN_SHARED_LIB

#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"

#endif // DIP__DOCTEST_IN_SHARED_LIB

#else //! DIP__IMPLEMENT_UNIT_TESTS

#ifdef DIP__DOCTEST_IN_SHARED_LIB

#define DOCTEST_CONFIG_IMPLEMENTATION_IN_DLL
#define DOCTEST_CONFIG_IMPLEMENT
#include "doctest.h"

#endif // DIP__DOCTEST_IN_SHARED_LIB

#endif // DIP__IMPLEMENT_UNIT_TESTS

#endif // DIP__ENABLE_DOCTEST
