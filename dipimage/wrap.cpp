/*
 * DIPimage 3.0
 * This MEX-file implements the `wrap` function
 *
 * (c)2017-2018, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 * Based on original DIPimage code: (c)1999-2014, Delft University of Technology.
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

#include "dip_matlab_interface.h"
#include "diplib/geometry.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 2 );
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();
      dip::IntegerArray shift = dml::GetIntegerArray( prhs[ 1 ] );
      dip::Wrap( in, out, shift );
      plhs[ 0 ] = dml::GetArray( out );
   } DML_CATCH
}
