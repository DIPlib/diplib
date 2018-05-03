/*
 * DIPimage 3.0
 * This MEX-file implements the `readics` function
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
#include "diplib/file_io.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::String const& filename = dml::GetString( prhs[ 0 ] );

      dip::UnsignedArray origin = {};
      dip::UnsignedArray sizes = {};
      dip::UnsignedArray spacing = {};
      if( nrhs > 1 ) {
         origin = dml::GetUnsignedArray( prhs[ 1 ] );
      }
      if( nrhs > 2 ) {
         sizes = dml::GetUnsignedArray( prhs[ 2 ] );
      }
      if( nrhs > 3 ) {
         spacing = dml::GetUnsignedArray( prhs[ 3 ] );
      }
      dip::FileInformation fileInformation = dip::ImageReadICS( out, filename, origin, sizes, spacing );
      // NOTE: "fast" option is useless here, as we cannot change the strides of out.

      plhs[ 0 ] = dml::GetArray( out );
      if( nlhs > 1 ) {
         plhs[ 1 ] = dml::GetArray( fileInformation );
      }

   } DML_CATCH
}
