/*
 * DIPlib 3.0
 * This file contains definitions for functions that support multithreading.
 *
 * (c)2014-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#include "diplib/library/multithreading.h"

#ifdef _OPENMP

#include <omp.h>

namespace dip {

namespace {

dip::uint maxNumberOfThreads = static_cast< dip::uint >( omp_get_max_threads() ); // This responds to the OMP_NUM_THREADS environment variable.

}

void SetNumberOfThreads( dip::uint nThreads ) {
   DIP_THROW_IF( nThreads < 1, E::INVALID_PARAMETER );
   maxNumberOfThreads = nThreads;
}

dip::uint GetNumberOfThreads() {
   return maxNumberOfThreads;
}

} // namespace dip

#else // not OpenMP

namespace dip {

void SetNumberOfThreads( dip::uint nThreads ) {}

dip::uint GetNumberOfThreads() {
   return 1;
}

} // namespace dip

#endif
