/*
 * DIPimage 3.0
 * This MEX-file implements the `cornerdetector` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/detection.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 4 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::String method = ( nrhs > 1 ) ? dml::GetString( prhs[ 1 ] ) : "ShiTomasi";
      dml::ToLower( method );

      dip::FloatArray sigmas = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 2.0 };

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      if( method == "harris" ) {
         dip::dfloat kappa = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 0.04;
         dip::HarrisCornerDetector( in, out, kappa, sigmas );
      } else if( method == "shitomasi" ) {
         dip::ShiTomasiCornerDetector( in, out, sigmas );
      } else if( method == "noble" ) {
         dip::NobleCornerDetector( in, out, sigmas );
      } else if( method == "wangbrady" ) {
         dip::dfloat threshold = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 0.1;
         dip::WangBradyCornerDetector( in, out, threshold, sigmas );
      } else {
         DIP_THROW_INVALID_FLAG( method );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
