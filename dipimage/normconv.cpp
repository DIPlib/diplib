/*
 * DIPimage 3.0
 * This MEX-file implements the `normconv` function
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
#include "diplib/linear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {

   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 7 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image const mask = dml::GetImage( prhs[ 1 ] );
      dip::Image out = mi.NewImage();

      bool computeDerivative = false;
      dip::uint dimension = 0;
      dip::FloatArray sigmas = { 1.0 };
      dip::String method = dip::S::BEST;
      dip::StringArray boundaryCondition = { dip::S::ADD_ZEROS };
      dip::dfloat truncation = 3;

      if( nrhs > 2 ) {
         if( !mxIsEmpty( prhs[ 2 ] )) {
            dimension = dml::GetUnsigned( prhs[ 2 ] );
            DIP_THROW_IF( ( dimension < 1 ) || ( dimension > in.Dimensionality() ), "Dimension index out of range" );
            --dimension;
            computeDerivative = true;
         }
      }
      if( nrhs > 3 ) {
         sigmas = dml::GetFloatArray( prhs[ 3 ] );
      }
      if( nrhs > 4 ) {
         method = dml::GetString( prhs[ 4 ] );
      }
      if( nrhs > 5 ) {
         boundaryCondition = dml::GetStringArray( prhs[ 5 ] );
      }
      if( nrhs > 6 ) {
         truncation = dml::GetFloat( prhs[ 6 ] );
      }

      if( computeDerivative ) {
         dip::NormalizedDifferentialConvolution( in, mask, out, dimension, sigmas, method, boundaryCondition, truncation );
      } else {
         dip::NormalizedConvolution( in, mask, out, sigmas, method, boundaryCondition, truncation );
      }

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
