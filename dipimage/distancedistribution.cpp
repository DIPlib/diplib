/*
 * DIPimage 3.0
 * This MEX-file implements the `distancedistribution` function
 *
 * (c)2018, Cris Luengo.
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

#include "dip_matlab_interface.h"
#include "diplib/analysis.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 5 );

      dip::Image const object = dml::GetImage( prhs[ 0 ] );
      dip::Image const region = dml::GetImage( prhs[ 1 ] );
      dip::uint length = nrhs > 2 ? dml::GetUnsigned( prhs[ 2 ] ) : 100;

      dip::Distribution out = dip::DistanceDistribution( object, region, length );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
