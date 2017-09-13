/*
 * DIPlib 3.0
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

#ifdef DIP__DOCTEST_IN_SHARED_LIB

#include "diplib/library/unit_tests_shared_lib.h"

namespace dip {

int run_unit_tests( int argc, const char* const* argv ) {
   doctest::Context context(argc, argv);
   return context.run();
}

} // namespace dip

#endif // DIP__DOCTEST_IN_SHARED_LIB
