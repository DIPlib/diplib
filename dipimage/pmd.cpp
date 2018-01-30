/*
 * DIPimage 3.0
 * This MEX-file implements the `pmd` function
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
      DML_MAX_ARGS( 5 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );

      dip::uint iterations = 5;
      if( nrhs > 1 ) {
         iterations = dml::GetUnsigned( prhs[ 1 ] );
      }

      dip::dfloat K = 10;
      if( nrhs > 2 ) {
         K = dml::GetFloat( prhs[ 2 ] );
      }

      dip::dfloat lambda = 0.25;
      if( nrhs > 3 ) {
         lambda = dml::GetFloat( prhs[ 3 ] );
      }

      dip::String g = "Gauss";
      if( nrhs > 4 ) {
         g = dml::GetString( prhs[ 4 ] );
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::PeronaMalik( in, out, iterations, K, lambda, g );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
