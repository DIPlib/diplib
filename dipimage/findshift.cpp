/*
 * DIPimage 3.0
 * This MEX-file implements the `findshift` function
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

#undef DIP__ENABLE_DOCTEST
#include "dip_matlab_interface.h"
#include "diplib/analysis.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 5 );

      dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
      dip::Image const in2 = dml::GetImage( prhs[ 1 ] );

      dip::String method = "integer only";
      dip::dfloat parameter = 0;
      dip::UnsignedArray maxShift = {};

      if( nrhs > 2 ) {
         method = dml::GetString( prhs[ 2 ] );
         if(( method == "integer" ) || ( method == "integer only" )) {
            method = "integer only";
         } else if( method == "ffts" ) {
            method = "CPF";
         } else if( method == "grs" ) {
            method = "MTS";
         } else {
            dml::ToUpper( method );
         }
      }
      if( nrhs > 3 ) {
         parameter = dml::GetFloat( prhs[ 3 ] );
      }
      if( nrhs > 4 ) {
         maxShift = dml::GetUnsignedArray( prhs[ 4 ] );
      }

      auto out = dip::FindShift( in1, in2, method, parameter, maxShift );

      plhs[ 0 ] = dml::GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
