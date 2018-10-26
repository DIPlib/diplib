/*
 * DIPimage 3.0
 * This MEX-file implements the 'mean' function
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
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      // Is there a mode string as last argument?
      dip::String mode = "";
      if(( nrhs > 1 ) && mxIsChar( prhs[ nrhs - 1 ] )) {
         mode = dml::GetString( prhs[ nrhs - 1 ] );
         --nrhs;
      }

      // Get image
      dip::Image in;
      in = dml::GetImage( prhs[ 0 ] );

      // Get mask image
      dip::Image mask;
      if( nrhs > 1 ) {
         mask = dml::GetImage( prhs[ 1 ] );
      }

      // Get optional process array
      dip::BooleanArray process;
      if( nrhs > 2 ) {
         process = dml::GetProcessArray( prhs[ 2 ], in.Dimensionality() );
      }

      // Do the thing
      if(( nrhs == 1 ) && ( mode == "tensor" )) { // nrhs == 1 because we did --nrhs earlier!
         dip::MeanTensorElement( in, out );
         plhs[ 0 ] = dml::GetArray( out );
      } else {
         dip::Mean( in, mask, out, mode, process );
         if( nrhs > 2 ) {
            plhs[ 0 ] = dml::GetArray( out );
         } else {
            plhs[ 0 ] = dml::GetArray( out.At( 0 ));
         }
      }

   } DML_CATCH
}
