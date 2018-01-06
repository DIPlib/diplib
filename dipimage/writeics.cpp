/*
 * DIPimage 3.0
 * This MEX-file implements the `writeics` function
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
      DML_MAX_ARGS( 5 );

      dip::Image image = dml::GetImage( prhs[ 0 ] );
      dip::String const& filename = dml::GetString( prhs[ 1 ] );

      dip::StringArray history = {};
      if( nrhs > 2 ) {
         history = dml::GetStringArray( prhs[ 2 ] );
      }
      dip::uint significantBits = 0;
      if( nrhs > 3 ) {
         significantBits = dml::GetUnsigned( prhs[ 3 ] );
      }
      dip::StringSet options = { dip::S::FAST };
      if( nrhs > 4 ) {
         options = dml::GetStringSet( prhs[ 4 ] );
      }
      dip::ImageWriteICS( image, filename, history, significantBits, options );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
