/*
 * (c)2014-2021, Cris Luengo.
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


#ifndef DIP_MULTITHREADING_H
#define DIP_MULTITHREADING_H

#ifdef _OPENMP
#include <omp.h>  // IWYU pragma: export
#else
// We don't have OpenMP, these are OpenMP function stubs to avoid conditional compilation elsewhere.
inline int omp_get_thread_num() { return 0; }
inline int omp_get_max_threads() { return 1; }
inline int omp_get_num_threads() { return 1; }
#endif

#include "diplib.h"


/// \file
/// \brief Declares functions to control multithreading within *DIPlib*, and imports the *OpenMP* header.
/// See \ref infrastructure.


namespace dip {


/// \addtogroup infrastructure


/// \brief Sets the maximum number of threads to be using in computations.
///
/// The default maximum number of threads is given by the `OMP_NUM_THREADS` environment variable if set, or the
/// number of CPU cores otherwise.
///
/// Note that parallelized algorithms only spawn multiple threads for the computation if the amount of work
/// to be done is large enough to compensate for the overhead of spawning threads.
///
/// If `nThreads` is 1, disables multithreading within *DIPlib*. Usually it is more beneficial to manage multithreading
/// at a higher level, for example by processing multiple images at the same time. If you do so, set `nThreads` to 1.
/// Furthermore, it seems that calling multithreaded *DIPlib* functions from within an OpenMP parallel section
/// doesn't work, so within an OpenMP parallel section you should always set `nThreads` to 1.
///
/// If `nThreads` is 0, resets the maximum number of threads to the default value.
///
/// Note that this number is thread-local, meaning it only applies to the current thread from which this
/// function is called. For every newly spawned thread, the maximum number of threads is the default as described
/// above, not the value manually set prior to spawning the thread.
///
/// If *DIPlib* was compiled without *OpenMP* support, this function does nothing.
DIP_EXPORT void SetNumberOfThreads( dip::uint nThreads );


/// \brief Gets the maximum number of threads that can be used in computations.
///
/// Returns the value given in the last call to \ref dip::SetNumberOfThreads within the current thread,
/// or the default maximum value if that function was never called within the current thread.
///
/// If *DIPlib* was compiled without *OpenMP* support, this function always returns 1.
DIP_EXPORT dip::uint GetNumberOfThreads();


// Undocumented constant: how many operations (clock cycles) it takes to make it worth going into multiple threads.
// (experimentally determined on Cris' computer, might be different elsewhere).
// I also noticed that going to 2 threads or 4 threads does not make a huge difference in overhead, so this is a
// threshold for single vs multithreaded computation, not a threshold per thread created.
constexpr dip::uint threadingThreshold = 70000;


/// \endgroup

} // namespace dip

#endif // DIP_MULTITHREADING_H
