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

   char const* fieldNames[] = { "name", "description", "copyright", "URL", "version", "date", "type" };
   try {

      plhs[ 0 ] = mxCreateStructMatrix( 1, 1, 7, fieldNames );
      mxSetFieldByNumber( plhs[ 0 ], 0, 0, dml::GetArray( dip::libraryInformation.name ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 1, dml::GetArray( dip::libraryInformation.description ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 2, dml::GetArray( dip::libraryInformation.copyright ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 3, dml::GetArray( dip::libraryInformation.URL ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 4, dml::GetArray( dip::libraryInformation.version ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 5, dml::GetArray( dip::libraryInformation.date ));
      mxSetFieldByNumber( plhs[ 0 ], 0, 6, dml::GetArray( dip::libraryInformation.type ));

   } DML_CATCH
}
