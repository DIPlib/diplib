/*
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

void mexFunction( int /*nlhs*/, mxArray* plhs[], int /*nrhs*/, const mxArray* /*prhs*/[] ) {

   char const* fieldNames[] = { "name", "description", "copyright", "URL", "version", "date", "type",
                                "isReleaseBuild", "usingOpenMP", "stackTracesEnabled", "assertsEnabled", "usingUnicode",
                                "hasICS", "hasTIFF", "hasJPEG", "hasPNG", "usingFFTW", "usingFreeType" };
   try {

      plhs[ 0 ] = mxCreateStructMatrix( 1, 1, 18, fieldNames );
      mxSetFieldByNumber( plhs[ 0 ], 0, 0, dml::GetArray( dip::libraryInformation.name ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 1, dml::GetArray( dip::libraryInformation.description ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 2, dml::GetArray( dip::libraryInformation.copyright ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 3, dml::GetArray( dip::libraryInformation.URL ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 4, dml::GetArray( dip::libraryInformation.version ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 5, dml::GetArray( dip::libraryInformation.date ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 6, dml::GetArray( dip::libraryInformation.type ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 7, dml::GetArray( dip::libraryInformation.isReleaseBuild ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 8, dml::GetArray( dip::libraryInformation.usingOpenMP ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 9, dml::GetArray( dip::libraryInformation.stackTracesEnabled ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 10, dml::GetArray( dip::libraryInformation.assertsEnabled ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 11, dml::GetArray( dip::libraryInformation.usingUnicode ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 12, dml::GetArray( dip::libraryInformation.hasICS ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 13, dml::GetArray( dip::libraryInformation.hasTIFF ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 14, dml::GetArray( dip::libraryInformation.hasJPEG ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 15, dml::GetArray( dip::libraryInformation.hasPNG ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 16, dml::GetArray( dip::libraryInformation.usingFFTW ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 17, dml::GetArray( dip::libraryInformation.usingFreeType ));

   } DML_CATCH
}
