/*
 * DIPimage 3.0
 * This MEX-file implements the 'percentile' function
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

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      // Get image
      dip::Image in;
      in = dml::GetImage( prhs[ 0 ] );

      // Get parameter
      dip::dfloat percentile = dml::GetFloat( prhs[ 1 ] );
      dip::uint nDims = in.Dimensionality();

      if(( nrhs == 3 ) && mxIsChar( prhs[ 2 ] )) {
         dip::String flag = dml::GetString( prhs[ 2 ] );
         if( flag != "tensor" ) {
            DIP_THROW_INVALID_FLAG( flag );
         }
         in.TensorToSpatial( nDims );
         dip::BooleanArray process( nDims + 1, false );
         process[ nDims ] = true;
         dip::Percentile( in, {}, out, percentile, process );
         out.Squeeze( nDims );
         plhs[ 0 ] = mi.GetArray( out );
         return;
      }

      // Get mask image
      dip::Image mask;
      if( nrhs > 2 ) {
         mask = dml::GetImage( prhs[ 2 ] );
      }

      if( nlhs == 2 ) { // Output position as well

         DIP_THROW_IF( nDims < 1, dip::E::DIMENSIONALITY_NOT_SUPPORTED );

         // Get optional dimensions
         dip::uint dim = nDims;
         if( nrhs > 3 ) {
            dim = dml::GetUnsigned( prhs[ 3 ] );
            DIP_THROW_IF( ( dim <= 0 ) || ( dim > nDims ), "DIM argument out of range" );
         }
         --dim;

         // Do the thing
         dip::BooleanArray process( nDims, false );
         process[ dim ] = true;
         dip::Percentile( in, mask, out, percentile, process );
         plhs[ 0 ] = mi.GetArray( out );

         dip::Image out2 = mi.NewImage();
         dip::PositionPercentile( in, mask, out2, percentile, dim );
         plhs[ 1 ] = mi.GetArray( out2 );

      } else {

         // Get optional process array
         dip::BooleanArray process;
         if( nrhs > 3 ) {
            process = dml::GetProcessArray( prhs[ 3 ], nDims );
         }

         // Do the thing
         dip::Percentile( in, mask, out, percentile, process );

         // Done
         if( nrhs > 2 ) {
            plhs[ 0 ] = mi.GetArray( out );
         } else {
            plhs[ 0 ] = dml::GetArray( out.At( 0 ));
         }

      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
