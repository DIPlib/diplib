/*
 * DIPimage 3.0
 * This MEX-file implements the `ramp` function
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
#include "diplib/generation.h"

void mexFunction( int nlhs, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 3 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      out.DataType() = dip::DT_SFLOAT;
      if( mxIsNumeric( prhs[ 0 ] ) && dml::IsVector( prhs[ 0 ] )) {
         out.SetSizes( dml::GetUnsignedArray( prhs[ 0 ] ));
      } else {
         dip::Image tmp = dml::GetImage( prhs[ 0 ] );
         out.SetSizes( tmp.Sizes() );
         out.SetPixelSize( tmp.PixelSize() );
      }
      out.Forge();

      dip::uint dimension = dml::GetUnsigned( prhs[ 1 ] );

      dip::StringSet mode = {};
      if( nrhs > 2 ) {
         // Parse mode strings
         if( mxIsChar( prhs[ 2 ] )) {
            dip::String tmp = dml::GetString( prhs[ 2 ] );
            if( tmp[ 0 ] == 'm' ) {
               mode.insert( "math" );
               tmp = tmp.erase( 0, 1 );
            }
            mode.insert( tmp );
         } else {
            mode = dml::GetStringSet( prhs[ 2 ] );
         }
      }

      dip::FillRamp( out, dimension, mode );

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
