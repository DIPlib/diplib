/*
 * DIPimage 3.0
 * This MEX-file implements the `fmmatch` function
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
#include "diplib/analysis.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dip::Image const in1 = dml::GetImage( prhs[ 0 ] );
      dip::Image const in2 = dml::GetImage( prhs[ 1 ] );

      dip::String interpolationMethod = nrhs > 2 ? dml::GetString( prhs[ 2 ] ) : dip::S::LINEAR;

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      auto matrix = dip::FourierMellinMatch2D( in1, in2, out, interpolationMethod );

      plhs[ 0 ] = dml::GetArray( out );
      if( nlhs > 1 ) {
         DIP_ASSERT( matrix.size() == 6 );
         plhs[ 1 ] = mxCreateDoubleMatrix( 2, 3, mxREAL );
         double* ptr = mxGetPr( plhs[ 1 ] );
         for( dip::uint ii = 0; ii < 6; ++ii ) {
            ptr[ ii ] = matrix[ ii ];
         }
      }

   } DML_CATCH
}
