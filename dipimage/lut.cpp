/*
 * DIPimage 3.0
 * This MEX-file implements the `lut` function
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
#include "diplib/lookup_table.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 5 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      dip::Image in = dml::GetImage( prhs[ 0 ] );
      dip::Image table = dml::GetImage( prhs[ 1 ] );
      if( table.Dimensionality() == 2 ) {
         DIP_THROW_IF( !table.IsScalar(), dip::E::DIMENSIONALITY_NOT_SUPPORTED );
         table.SpatialToTensor( 0 );
      } else if( table.Dimensionality() != 1 ) {
         DIP_THROW( dip::E::DIMENSIONALITY_NOT_SUPPORTED );
      }
      int index = 2;

      dip::FloatArray indices;
      if(( nrhs > index ) && ( mxIsNumeric( prhs[ index ] ))) {
         indices = dml::GetFloatArray( prhs[ index ] );
         ++index;
      }

      dip::LookupTable lut( table, indices );

      dip::String method = dip::S::LINEAR;
      if( nrhs > index ) {
         method = dml::GetString( prhs[ index ] );
         ++index;
      }
      if( nrhs > index ) {
         if( mxIsNumeric( prhs[ index ] )) {
            dip::FloatArray bounds = dml::GetFloatArray( prhs[ index ] );
            if( bounds.size() == 1 ) {
               lut.SetOutOfBoundsValue( bounds[ 0 ] );
            } else if( bounds.size() == 2 ) {
               lut.SetOutOfBoundsValue( bounds[ 0 ], bounds[ 1 ] );
            } else {
               DIP_THROW( dip::E::INVALID_FLAG );
            }
         } else {
            dip::String bounds = dml::GetString( prhs[ index ] );
            if( bounds == "clamp" ) {
               lut.ClampOutOfBoundsValues();
            } else if( bounds == dip::S::KEEP ) {
               lut.KeepInputValueOnOutOfBounds();
            } else {
               DIP_THROW( dip::E::INVALID_FLAG );
            }
         }
      }

      lut.Apply( in, out, method );

      if( table.IsColor() ) {
         out.SetColorSpace( table.ColorSpace() );
      } else if( table.TensorElements() == 3 ) {
         out.SetColorSpace( "RGB" );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
