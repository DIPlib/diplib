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

#undef DIP__ENABLE_DOCTEST
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

      plhs[ 0 ] = mi.GetArray( out );
      if( nlhs > 1 ) {
         constexpr int nFields = 10;
         char const* fieldNames[ nFields ] = {
               "name",
               "fileType",
               "dataType",
               "significantBits",
               "sizes",
               "tensorElements",
               "colorSpace",
               "pixelSize",
               "numberOfImages",
               "history"
         };
         mwSize dims[ 2 ] = { 1, 1 };
         plhs[ 1 ] = mxCreateStructArray( 2, dims, nFields, fieldNames );
         mxSetField( plhs[ 1 ], 0, fieldNames[ 0 ], dml::GetArray( fileInformation.name ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 1 ], dml::GetArray( fileInformation.fileType ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 2 ], dml::GetArray( dip::String{ fileInformation.dataType.Name() } ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 3 ], dml::GetArray( fileInformation.significantBits ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 4 ], dml::GetArray( fileInformation.sizes ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 5 ], dml::GetArray( fileInformation.tensorElements ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 6 ], dml::GetArray( fileInformation.colorSpace ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 7 ], dml::GetArray( fileInformation.pixelSize ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 8 ], dml::GetArray( fileInformation.numberOfImages ));
         mxSetField( plhs[ 1 ], 0, fieldNames[ 9 ], dml::GetArray( fileInformation.history ));
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
