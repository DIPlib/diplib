/*
 * DIPimage 3.0
 * This MEX-file implements the `coordinates` function
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
#include "diplib/generation.h"

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 0 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image out = mi.NewImage();

      out.DataType() = dip::DT_SFLOAT;
      if( nrhs > 0 ) {
         if( mxIsNumeric( prhs[ 0 ] ) && dml::IsVector( prhs[ 0 ] )) {
            out.SetSizes( dml::GetUnsignedArray( prhs[ 0 ] ));
         } else {
            dip::Image tmp = dml::GetImage( prhs[ 0 ] );
            out.SetSizes( tmp.Sizes());
            out.SetPixelSize( tmp.PixelSize());
         }
      } else {
         out.SetSizes( { 256, 256 } );
      }

      dip::StringSet mode = {};
      if( nrhs > 2 ) {
         dip::String origin = dml::GetString( prhs[ 2 ] );
         if( origin[ 0 ] == 'm' ) {
            mode.insert( dip::S::MATH );
            origin = origin.erase( 0, 1 );
         }
         mode.insert( origin );
      } else {
         mode.insert( dip::S::RIGHT );
      }
      if( nrhs > 3 ) {
         dip::StringArray options = dml::GetStringArray( prhs[ 3 ] );
         for( auto& opt : options ) {
            mode.insert( opt );
         }
      }

      if(( nrhs > 1 ) && ( mxIsNumeric( prhs[ 1 ] ))) {
         out.Forge();
         dip::uint dim = dml::GetUnsigned( prhs[ 1 ] );
         if( dim == 0 ) {
            DIP_THROW( dip::E::PARAMETER_OUT_OF_RANGE );
         }
         dip::FillRamp( out, dim - 1, mode );
      } else {
         dip::String value = dip::S::CARTESIAN;
         if( nrhs > 1 ) {
            value = dml::GetString( prhs[ 1 ] );
         }
         if(( value == dip::S::CARTESIAN ) || ( value == dip::S::SPHERICAL )) {
            out.SetTensorSizes( out.Dimensionality() );
            out.Forge();
            dip::FillCoordinates( out, mode, value );
         } else if( value == "radius" ) {
            out.Forge();
            dip::FillRadiusCoordinate( out, mode );
         } else if( value == "phi" ) {
            out.Forge();
            dip::FillPhiCoordinate( out, mode );
         } else if( value == "theta" ) {
            out.Forge();
            dip::FillThetaCoordinate( out, mode );
         } else {
            DIP_THROW_INVALID_FLAG( value );
         }
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
