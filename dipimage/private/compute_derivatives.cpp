/*
 * DIPimage 3.0
 * This MEX-file implements the `compute_derivatives` private function
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
#include "diplib/linear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 6 );

      dip::String output = dml::GetString( prhs[ 0 ] );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 1 ] );
      dip::Image out = mi.NewImage();

      dip::FloatArray sigmas = { 1 };
      dip::String method = dip::S::BEST;
      dip::StringArray bc = {};
      dip::BooleanArray process = {};
      dip::dfloat truncation = 3;

      int index = 2;
      if( nrhs > index ) {
         sigmas = dml::GetFloatArray( prhs[ index ] );
         ++index;
      }
      if( nrhs > index ) {
         method = dml::GetString( prhs[ index ] );
         ++index;
      }
      if( nrhs > index ) {
         bc = dml::GetStringArray( prhs[ index ] );
         ++index;
      }
      if( nrhs > index ) {
         process = dml::GetProcessArray( prhs[ index ], in.Dimensionality() );
         ++index;
      }
      if( nrhs > index ) {
         truncation = dml::GetFloat( prhs[ index ] );
      }

      if( output == "gradientvector" ) {
         dip::Gradient( in, out, sigmas, method, bc, process, truncation );
      } else if( output == "gradmag" ) {
         dip::GradientMagnitude( in, out, sigmas, method, bc, process, truncation );
      } else if( output == "hessian" ) {
         dip::Hessian( in, out, sigmas, method, bc, process, truncation );
      } else if( output == "laplace" ) {
         dip::Laplace( in, out, sigmas, method, bc, process, truncation );
      } else if( output == "dgg" ) {
         dip::Dgg( in, out, sigmas, method, bc, process, truncation );
      } else if( output == "laplace_min_dgg" ) {
         dip::LaplaceMinusDgg( in, out, sigmas, method, bc, process, truncation );
      } else if( output == "laplace_plus_dgg" ) {
         dip::LaplacePlusDgg( in, out, sigmas, method, bc, process, truncation );
      } else {
         DIP_THROW_INVALID_FLAG( output );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
