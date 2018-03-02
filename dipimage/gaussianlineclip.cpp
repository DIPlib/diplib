/*
 * DIPimage 3.0
 * This MEX-file implements the `gaussianlineclip` function
 *
 * (c)2017, Cris Luengo.
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
#include "diplib/generation.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::dfloat sigma = 1;
      if( nrhs > 1 ) {
         sigma = dml::GetFloat( prhs[ 1 ] );
      }
      bool normalisetoone = false;
      if( nrhs > 2 ) {
         normalisetoone = dml::GetBoolean( prhs[ 2 ] );
      }
      dip::dfloat truncation = 3;
      if( nrhs > 3 ) {
         truncation = dml::GetFloat( prhs[ 3 ] );
      }

      dip::dfloat value = 1.0;
      if( normalisetoone ) {
         value = std::pow( 2.0 * dip::pi * sigma * sigma, static_cast< dip::dfloat >( in.Dimensionality() ) / 2.0 );
      }

      dip::GaussianLineClip( in, out, { value }, sigma, truncation );

      plhs[ 0 ] = dml::GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
