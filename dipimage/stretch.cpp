/*
 * DIPimage 3.0
 * This MEX-file implements the `stretch` function
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

#include "dip_matlab_interface.h"
#include "diplib/mapping.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 8 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::dfloat low = 0.0;
      if( nrhs > 1 ) {
         low = dml::GetFloat( prhs[ 1 ] );
      }
      dip::dfloat high = 100.0;
      if( nrhs > 2 ) {
         high = dml::GetFloat( prhs[ 2 ] );
      }
      dip::dfloat minimum = 0.0;
      if( nrhs > 3 ) {
         minimum = dml::GetFloat( prhs[ 3 ] );
      }
      dip::dfloat maximum = 255.0;
      if( nrhs > 4 ) {
         maximum = dml::GetFloat( prhs[ 4 ] );
      }
      dip::String method = dip::S::LINEAR;
      if( nrhs > 5 ) {
         method = dml::GetString( prhs[ 5 ] );
      }
      dip::dfloat param1 = 1.0;
      if( nrhs > 6 ) {
         param1 = dml::GetFloat( prhs[ 6 ] );
      }
      dip::dfloat param2 = 0.0;
      if( nrhs > 7 ) {
         param2 = dml::GetFloat( prhs[ 7 ] );
      }

      dip::ContrastStretch( in, out, low, high, minimum, maximum, method, param1, param2 );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
