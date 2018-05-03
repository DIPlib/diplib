/*
 * DIPimage 3.0
 * This MEX-file implements the `ced` function
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/nonlinear.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 7 );

      dml::streambuf b;

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::dfloat derivativeSigma = 1;
      if( nrhs > 1 ) {
         derivativeSigma = dml::GetFloat( prhs[ 1 ] );
      }
      dip::dfloat regularizationSigma = 3;
      if( nrhs > 2 ) {
         regularizationSigma = dml::GetFloat( prhs[ 2 ] );
      }

      dip::uint iterations = 5;
      if( nrhs > 3 ) {
         iterations = dml::GetUnsigned( prhs[ 3 ] );
      }

      dip::StringSet flags = {};
      if( nrhs > 4 ) {
         dip::String coef = dml::GetString( prhs[ 4 ] );
         flags.insert( std::move( coef ));
      }
      if( nrhs > 5 ) {
         dip::String flavour = dml::GetString( prhs[ 5 ] );
         flags.insert( std::move( flavour ));
      }
      if( nrhs > 6 ) {
         if( !dml::GetBoolean( prhs[ 6 ] )) {
            flags.emplace( "resample" );
         }
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      CoherenceEnhancingDiffusion( in, out, derivativeSigma, regularizationSigma, iterations, flags );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
