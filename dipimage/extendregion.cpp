/*
 * DIPimage 3.0
 * This MEX-file implements the `extendregion` function
 *
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
#include "diplib/boundary.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      out.Copy( in );

      dip::RangeArray ranges;
      dip::UnsignedArray origin;
      dip::UnsignedArray sizes;
      bool useRanges = true;

      int index = 1;
      if( mxIsCell( prhs[ index ] )) {
         ranges = dml::GetRangeArray( prhs[ index ] );
         ++index;
      } else {
         DML_MIN_ARGS( 3 );
         useRanges = false;
         origin = dml::GetUnsignedArray( prhs[ index ] );
         ++index;
         sizes = dml::GetUnsignedArray( prhs[ index ] );
         ++index;
      }

      dip::StringArray bc;
      if( nrhs > index ) {
         bc = dml::GetStringArray( prhs[ index ]);
         ++index;
      }
      DML_MAX_ARGS( index );

      if( useRanges ) {
         dip::ExtendRegion( out, ranges, bc );
      } else {
         dip::ExtendRegion( out, origin, sizes, bc );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
