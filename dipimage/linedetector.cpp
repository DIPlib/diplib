/*
 * DIPimage 3.0
 * This MEX-file implements the `linedetector` function
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
      DML_MAX_ARGS( 5 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::String method = ( nrhs > 1 ) ? dml::GetString( prhs[ 1 ] ) : "Frangi";
      dml::ToLower( method );
      dip::String polarity = ( nrhs > 4 ) ? dml::GetString( prhs[ 4 ] ) : dip::S::WHITE;

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      if( method == "frangi" ) {
         dip::FloatArray sigmas = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 2.0 };
         dip::FloatArray parameters = ( nrhs > 3 ) ? dml::GetFloatArray( prhs[ 3 ] ) : dip::FloatArray{};
         dip::FrangiVesselness( in, out, sigmas, parameters, polarity );
      } else if( method == "danielsson" ) {
         dip::FloatArray sigmas = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 2.0 };
         dip::DanielssonLineDetector( in, out, sigmas, polarity );
      } else if( method == "matched" ) {
         dip::dfloat sigma = ( nrhs > 2 ) ? dml::GetFloat( prhs[ 2 ] ) : 2.0;
         dip::dfloat length = ( nrhs > 3 ) ? dml::GetFloat( prhs[ 3 ] ) : 10.0;
         dip::MatchedFiltersLineDetector2D( in, out, sigma, length, polarity );
      } else if( method == "rorpo" ) {
         dip::uint length = ( nrhs > 2 ) ? dml::GetUnsigned( prhs[ 2 ] ) : 15;
         dip::RORPOLineDetector( in, out, length, polarity );
      } else {
         DIP_THROW_INVALID_FLAG( method );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
