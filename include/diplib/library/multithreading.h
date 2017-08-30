/*
 * DIPlib 3.0
 * This file contains declarations for functions that support multithreading.
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


//
// NOTE!
// This file is included through diplib.h -- no need to include directly
//


#ifndef DIP_MULTITHREADING_H
#define DIP_MULTITHREADING_H

#include "diplib/library/types.h"


/// \file
/// \brief Declares functions to control multithreading within DIPlib. This file is always included through `diplib.h`.
/// \see infrastructure


namespace dip {

/// \addtogroup infrastructure
/// \{


/// \brief Sets the maximum number of threads to be using in computations.
///
/// The default maximum number of threads is given by the `OMP_NUM_THREADS` environment variable if set, or the
/// number of CPU cores otherwise.
///
/// Note that parallelized algorithms only spawn multiple threads for the computation if the amount of work
/// to be done is large enough to compensate for the overhead of spawning threads. Usually it is more beneficial
/// to manage multithreading at a higher level, for example by processing multiple images at the same time.
///
/// If `nThreads` is 1, disables multithreading within DIPlib.
///
/// If DIPlib was compiled without OpenMP support, this function does nothing.
DIP_EXPORT void SetNumberOfThreads( dip::uint nThreads );


/// \brief Gets the maximum number of threads that can be used in computations.
///
/// Returns the value given in the last call to `dip::SetNumberOfThreads`, or the default maximum value if that
/// function was never called.
///
/// If DIPlib was compiled without OpenMP support, this function always returns 1.
DIP_EXPORT dip::uint GetNumberOfThreads();


// Undocumented constant: how many clock cycles a thread must use to make it worth while to create it.
constexpr dip::uint nClockCyclesPerThread = 10000;

/// \}

} // namespace dip

#endif // DIP_MULTITHREADING_H
