/*
 * DIPimage 3.0
 * This MEX-file implements the `asf` function
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
#include "diplib/morphology.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 6 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      int index = 1;

      dip::Range sizes = { 3, 7, 2 };
      if( nrhs > index ) {
         sizes = dml::GetRange( prhs[ index ] );
         ++index;
      }

      dip::String shape = dip::S::ELLIPTIC;
      if( nrhs > index ) {
         shape = dml::GetString( prhs[ index ] );
         ++index;
      }

      dip::String mode = dip::S::STRUCTURAL;
      if( nrhs > index ) {
         mode = dml::GetString( prhs[ index ] );
         ++index;
      }

      dip::String polarity = dip::S::OPENCLOSE;
      if( nrhs > index ) {
         polarity = dml::GetString( prhs[ index ] );
         ++index;
      }

      dip::StringArray bc;
      if( nrhs > index ) {
         bc = dml::GetStringArray( prhs[ index ] );
      }

      dip::AlternatingSequentialFilter( in, out, sizes, shape, mode, polarity, bc );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
