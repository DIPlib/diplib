/*
 * DIPimage 3.0
 * This MEX-file implements the `drawpolygon` function
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
#include "diplib/chain_code.h"

dip::Polygon GetPolygon( mxArray const* mx ) {
   if( mxIsDouble( mx ) && !mxIsComplex( mx )) {
      dip::uint n = mxGetM( mx );
      DIP_THROW_IF( mxGetN( mx ) != 2, "Coordinate array of wrong size" );
      dip::Polygon out;
      out.vertices.resize( n );
      double* data = mxGetPr( mx );
      for( auto& o : out.vertices ) {
         o = { data[ 0 ], data[ n ] };
         ++data;
      }
      return out;
   } else if( mxIsCell( mx ) && dml::IsVector( mx )) {
      dip::uint n = mxGetNumberOfElements( mx );
      dip::Polygon out;
      out.vertices.resize( n );
      for( dip::uint ii = 0; ii < n; ++ii ) {
         mxArray const* elem = mxGetCell( mx, ii );
         DIP_THROW_IF( mxGetNumberOfElements( elem ) != 2, "Coordinate array of wrong size" );
         try {
            auto tmp = dml::GetFloatArray( elem );
            out.vertices[ ii ] = { tmp[ 0 ], tmp[ 1 ] };
         } catch( dip::Error& ) {
            DIP_THROW( "Coordinates in array must be numeric arrays" );
         }
      }
      return out;
   }
   DIP_THROW( "Coordinate array expected" );
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 4 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      out.Copy( in );

      dip::Image::Pixel color = { 255 };
      if( nrhs > 2 ) {
         color.swap( dml::GetPixel( prhs[ 2 ] )); // we cannot assign to a pixel!
      }

      dip::String mode = dip::S::OPEN;
      if( nrhs > 3 ) {
         mode = dml::GetString( prhs[ 3 ] );
      }

      if( out.Dimensionality() == 2 ) {
         dip::Polygon coords = GetPolygon( prhs[ 1 ] );
         dip::DrawPolygon2D( out, coords, color, mode );
      } else {
         dip::CoordinateArray coords = dml::GetCoordinateArray( prhs[ 1 ] );
         if( mode == dip::S::CLOSED ) {
            if( coords.front() != coords.back() ) {
               coords.push_back( coords.front() );
            }
         } else {
            DIP_THROW_IF( mode != dip::S::OPEN, dip::E::INVALID_FLAG );
         }
         dip::DrawLines( out, coords, color );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
