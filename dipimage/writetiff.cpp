/*
 * DIPimage 3.0
 * This MEX-file implements the `writetiff` function
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

void mexFunction( int /*nlhs*/, mxArray* /*plhs*/[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dip::Image image = dml::GetImage( prhs[ 0 ] );
      dip::String const& filename = dml::GetString( prhs[ 1 ] );

      dip::String compression;
      if( nrhs > 2 ) {
         compression = dml::GetString( prhs[ 2 ] );
      }
      dip::uint jpegLevel = 80;
      if( nrhs > 3 ) {
         jpegLevel = dml::GetUnsigned( prhs[ 3 ] );
      }

      dip::ImageWriteTIFF( image, filename, compression, jpegLevel );

   } DML_CATCH
}
