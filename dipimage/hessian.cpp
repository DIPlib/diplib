/*
 * DIPimage 3.0
 * This MEX-file implements the `hessian` function
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

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 5 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();

      dip::FloatArray sigmas = { 1 };
      dip::String method = dip::S::BEST;
      dip::StringArray bc = {};
      dip::BooleanArray process = {};
      dip::dfloat truncation = 3;

      if( nrhs > 1 ) {
         sigmas = dml::GetFloatArray( prhs[ 1 ] );
      }
      if( nrhs > 2 ) {
         method = dml::GetString( prhs[ 2 ] );
      }
      if( nrhs > 3 ) {
         bc = dml::GetStringArray( prhs[ 3 ] );
      }
      if( nrhs > 4 ) {
         process = dml::GetProcessArray( prhs[ 4 ], in.Dimensionality() );
      }
      if( nrhs > 5 ) {
         truncation = dml::GetFloat( prhs[ 5 ] );
      }

      dip::Hessian( in, out, sigmas, method, bc, process, truncation );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
