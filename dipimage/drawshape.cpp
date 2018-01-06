/*
 * DIPimage 3.0
 * This MEX-file implements the `drawshape` function
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

#include <include/diplib.h>
#include "dip_matlab_interface.h"
#include "diplib/generation.h"

dip::uint CheckCoordinateArray( mxArray const* mx, dip::uint nDims ) {
   DIP_THROW_IF( !mxIsDouble( mx ) || mxIsComplex( mx ), "Floating-point array expected" );
   DIP_THROW_IF(( mxGetNumberOfDimensions( mx ) != 2 ) || ( mxGetN( mx ) != nDims ), "Coordinate array of wrong size" );
   return mxGetM( mx );
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 3 );
      DML_MAX_ARGS( 7 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      out.Copy( in );

      dip::FloatArray sizes = dml::GetFloatArray( prhs[ 1 ] );
      dip::FloatArray origin = dml::GetFloatArray( prhs[ 2 ] );
      int index = 3;
      dip::String shape = "ellipsoid";
      if( nrhs > index ) {
         shape = dml::GetString( prhs[ index ] );
      }
      ++index;
      dip::Image::Pixel color = { 255 };
      if( nrhs > index ) {
         color.swap( dml::GetPixel( prhs[ index ] )); // we cannot assign to a pixel!
      }
      ++index;
      dip::dfloat sigma = 0;
      if( nrhs > index ) {
         sigma = dml::GetFloat( prhs[ index ] );
      }
      ++index;
      dip::dfloat truncation = 3;
      if( nrhs > index ) {
         truncation = dml::GetFloat( prhs[ index ] );
      }

      if(( shape == "ellipse" ) || shape == "ellipsoid" ) {
         dip::DrawEllipsoid( out, sizes, origin, color );
      } else if(( shape == "disk" ) || shape == "ball" ) {
         DIP_THROW_IF( sizes.size() != 1, dip::E::ARRAY_ILLEGAL_SIZE );
         if( sigma == 0.0 ) {
            dip::DrawEllipsoid( out, sizes, origin, color );
         } else {
            dip::DrawBandlimitedBall( out, sizes[ 0 ], origin, color, dip::S::FILLED, sigma, truncation );
         }
      } else if(( shape == "circle" ) || shape == "sphere" ) {
         DIP_THROW_IF( sizes.size() != 1, dip::E::ARRAY_ILLEGAL_SIZE );
         dip::DrawBandlimitedBall( out, sizes[ 0 ], origin, color, dip::S::EMPTY, sigma, truncation );
      } else if(( shape == "rectangle" ) || shape == "box" ) {
         if( sigma == 0.0 ) {
            dip::DrawBox( out, sizes, origin, color );
         } else {
            dip::DrawBandlimitedBox( out, sizes, origin, color, dip::S::FILLED, sigma, truncation );
         }
      } else if( shape == "box shell" ) {
         dip::DrawBandlimitedBox( out, sizes, origin, color, dip::S::EMPTY, sigma, truncation );
      } else if( shape == "diamond" ) {
         dip::DrawDiamond( out, sizes, origin, color );
      } else {
         DIP_THROW( dip::E::INVALID_FLAG );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
