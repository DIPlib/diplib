/*
 * DIPimage 3.0
 * This MEX-file implements the `percf` function
 *
 * (c)2017, Cris Luengo.
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

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/nonlinear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 5 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::dfloat percentile = 50;
      dip::uint N = 1;
      if( nrhs > N ) {
         percentile = dml::GetFloat( prhs[ N ] );
         ++N;
      }

      dip::Kernel kernel;
      if( nrhs > N ) {
         if( mxIsNumeric( prhs[ N ] ) && ( mxGetNumberOfElements( prhs[ N ] ) <= in.Dimensionality() )) {
            // This looks like a sizes vector
            auto filterParam = dml::GetFloatArray( prhs[ N ] );
            ++N;
            if( nrhs > N ) {
               auto filterShape = dml::GetString( prhs[ N ] );
               kernel = { filterParam, filterShape };
               ++N;
            } else {
               kernel = { filterParam };
            }
         } else {
            // Assume it's an image?
            DML_MAX_ARGS( 4 );
            kernel = { dml::GetImage( prhs[ N ] ) };
            ++N;
         }
      }

      dip::StringArray bc;
      if( nrhs > N ) {
         bc = dml::GetStringArray( prhs[ N ] );
      }

      dip::PercentileFilter( in, out, percentile, kernel, bc );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
