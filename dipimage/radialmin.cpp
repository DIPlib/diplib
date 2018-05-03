/*
 * DIPimage 3.0
 * This MEX-file implements the `radialmin` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/statistics.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 5 );

      int arg = 0;
      dip::Image const in = dml::GetImage( prhs[ arg ] );
      ++arg;

      dip::Image mask;
      if( nrhs > arg ) {
         if( !mxIsNumeric( prhs[ arg ] ) || !dml::IsScalar( prhs[ arg ] )) {
            // It seems we might have a mask image as 2nd input argument
            mask = dml::GetImage( prhs[ arg ] );
            ++arg;
         }
      }

      dip::dfloat binSize = 1;
      if( nrhs > arg ) {
         binSize = dml::GetFloat( prhs[ arg ] );
         ++arg;
      }

      dip::String maxRadius = dip::S::OUTERRADIUS;
      if( nrhs > arg ) {
         if( mxIsNumeric( prhs[ arg ] ) && dml::IsScalar( prhs[ arg ] )) {
            maxRadius = dml::GetBoolean( prhs[ arg ] ) ? dip::S::INNERRADIUS : dip::S::OUTERRADIUS;
         } else {
            maxRadius = dml::GetString( prhs[ arg ] );
         }
         ++arg;
      }

      dip::FloatArray center = {};
      if( nrhs > arg ) {
         if( mxIsChar( prhs[ arg ] )) {
            dip::String mode = dml::GetString( prhs[ arg ] );
            center = in.GetCenter( mode );
         } else {
            center = dml::GetFloatArray( prhs[ arg ] );
         }
         ++arg;
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::RadialMinimum( in, mask, out, binSize, maxRadius, center );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
