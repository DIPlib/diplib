/*
 * DIPimage 3.0
 * This MEX-file implements the 'prod' function
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
#include "diplib/statistics.h"
#include "diplib/math.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;

      dip::Image in;
      dip::Image mask;
      dip::Image out = mi.NewImage();

      // Get input image
      in = dml::GetImage( prhs[ 0 ] );

      // Handle tensor flag
      if(( nrhs == 2 ) && mxIsChar( prhs[ 1 ] )) {
         dip::String flag = dml::GetString( prhs[ 1 ] );
         if( flag != "tensor" ) {
            DIP_THROW_INVALID_FLAG( flag );
         }
         dip::ProductTensorElements( in, out );
         plhs[ 0 ] = dml::GetArray( out );
         return;
      }

      // Get mask image
      if( nrhs > 1 ) {
         mask = dml::GetImage( prhs[ 1 ] );
      }

      // Get optional process array
      dip::BooleanArray process;
      if( nrhs > 2 ) {
         process = dml::GetProcessArray( prhs[ 2 ], in.Dimensionality() );
      }

      // Do the thing
      dip::Product( in, mask, out, process );

      // Done
      if( nrhs > 2 ) {
         plhs[ 0 ] = dml::GetArray( out );
      } else {
         plhs[ 0 ] = dml::GetArray( out.At( 0 ));
      }

   } DML_CATCH
}
