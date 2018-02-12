/*
 * DIPimage 3.0
 * This MEX-file implements the 'max' function
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
#include "diplib/math.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, mxArray const* prhs[] ) {
   try {

      DML_MIN_ARGS( 1 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;

      dip::Image in1;
      dip::Image in2; // either 2nd image or mask image
      dip::Image out = mi.NewImage();

      // Get first image
      in1 = dml::GetImage( prhs[ 0 ] );

      // Handle tensor flag
      if(( nrhs == 2 ) && mxIsChar( prhs[ 1 ] )) {
         dip::String flag = dml::GetString( prhs[ 1 ] );
         if( flag != "tensor" ) {
            DIP_THROW_INVALID_FLAG( flag );
         }
         dip::MaximumTensorElement( in1, out );
         plhs[ 0 ] = mi.GetArray( out );
         return;
      }

      // Get second image
      if( nrhs > 1 ) {
         in2 = dml::GetImage( prhs[ 1 ] );
      }

      // Get optional process array
      dip::BooleanArray process;
      bool hasProcess = false;
      if( nrhs > 2 ) {
         process = dml::GetProcessArray( prhs[ 2 ], in1.Dimensionality() );
         hasProcess = true;
      }

      // Operation mode
      if( !in2.IsForged() || in2.DataType().IsBinary()) {
         // Maximum pixel projection
         dip::Maximum( in1, in2, out, process );
         if( hasProcess ) {
            plhs[ 0 ] = mi.GetArray( out );
         } else {
            plhs[ 0 ] = dml::GetArray( out.At( 0 ));
         }
         if( nlhs > 1 ) {
            // Compute position also
            dip::uint k = process.count();
            if( !hasProcess || ( k == in1.Dimensionality() )) {
               plhs[ 1 ] = dml::GetArray( dip::MaximumPixel( in1, in2 ));
            } else if( k == 1 ) {
               k = process.find( true );
               dip::Image out2 = mi.NewImage();
               dip::PositionMaximum( in1, in2, out2, k );
               plhs[ 1 ] = mi.GetArray( out2 );
            } else {
               DIP_THROW( "Cannot produce position value for more than one dimension" );
            }
         }
      } else {
         // Maximum over two images
         dip::Supremum( in1, in2, out );
         plhs[ 0 ] = mi.GetArray( out );
      }

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
