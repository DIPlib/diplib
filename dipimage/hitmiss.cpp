/*
 * DIPimage 3.0
 * This MEX-file implements the `hitmiss` function
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
#include "diplib/morphology.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );

      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image const se0 = dml::GetImage( prhs[ 1 ] );

      dip::Image se1, se2;
      int index = 2;
      if(( nrhs > index ) && !mxIsChar( prhs[ index ] )) {
         se1 = se0;
         se2 = dml::GetImage( prhs[ index ] );
         ++index;
      } else {
         se1 = se0 == 1;
         se2 = se0 == 0;
      }

      DML_MAX_ARGS( index + 2 );

      dip::String mode = dip::S::UNCONSTRAINED;
      if( nrhs > index ) {
         mode = dml::GetString( prhs[ index ] );
         ++index;
      }

      dip::StringArray bc = {};
      if( nrhs > index ) {
         bc = dml::GetStringArray( prhs[ index ] );
      }

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::HitAndMiss( in, out, se1, se2, mode, bc );

      plhs[ 0 ] = dml::GetArray( out );

   } DML_CATCH
}
