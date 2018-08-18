/*
 * DIPimage 3.0
 * This MEX-file implements the `derivative` function
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
#include "diplib/linear.h"
#include "diplib/generation.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 6 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::UnsignedArray order = { 0 };
      dip::FloatArray sigmas = { 1 };
      dip::String method = dip::S::BEST;
      dip::StringArray bc = {};
      dip::dfloat truncation = 3;

      if( nrhs > 1 ) {
         order = dml::GetUnsignedArray( prhs[ 1 ] );
      }
      if( nrhs > 2 ) {
         sigmas = dml::GetFloatArray( prhs[ 2 ] );
      }
      if( nrhs > 3 ) {
         method = dml::GetString( prhs[ 3 ] );
      }
      if( nrhs > 4 ) {
         bc = dml::GetStringArray( prhs[ 4 ] );
      }
      if( nrhs > 5 ) {
         truncation = dml::GetFloat( prhs[ 5 ] );
      }

      if( method == "kernel" ) {
         dip::CreateGauss( out, sigmas, order, truncation );
      } else {
         dip::Derivative( in, out, order, sigmas, method, bc, truncation );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
