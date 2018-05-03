/*
 * DIPimage 3.0
 * This MEX-file implements the `readtiff` function
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
#include "diplib/file_io.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 6 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::String const& filename = dml::GetString( prhs[ 0 ] );

      dip::Range imageNumbers{ 0 };
      if( nrhs > 1 ) {
         imageNumbers = dml::GetRange( prhs[ 1 ] );
      }
      dip::UnsignedArray origin = {};
      if( nrhs > 2 ) {
         origin = dml::GetUnsignedArray( prhs[ 2 ] );
      }
      dip::UnsignedArray sizes = {};
      if( nrhs > 3 ) {
         sizes = dml::GetUnsignedArray( prhs[ 3 ] );
      }
      dip::UnsignedArray spacing = {};
      if( nrhs > 4 ) {
         spacing = dml::GetUnsignedArray( prhs[ 4 ] );
      }
      dip::Range channels = {};
      if( nrhs > 5 ) {
         channels = dml::GetRange( prhs[ 5 ] );
      }
      dip::FileInformation fileInformation = dip::ImageReadTIFF( out, filename, imageNumbers, origin, sizes, spacing, channels);

      plhs[ 0 ] = dml::GetArray( out );
      if( nlhs > 1 ) {
         plhs[ 1 ] = dml::GetArray( fileInformation );
      }

   } DML_CATCH
}
