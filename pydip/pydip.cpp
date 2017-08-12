/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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

#include "pydip.h"
#include "diplib/neighborlist.h"

static_assert( sizeof( bool ) == sizeof( dip::bin ), "bool is not one byte, how can I work with logical Python buffers?" );

PYBIND11_MODULE( PyDIP_bin, m) {
   m.doc() = "The portion of the PyDIP module that contains the C++ DIPlib bindings.";

   init_image( m );
   init_math( m );
   init_statistics( m );
   init_filtering( m );
   init_morphology( m );
   init_analysis( m );
   init_measurement( m );
   init_assorted( m );
}
