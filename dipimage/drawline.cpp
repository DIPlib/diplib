/*
 * DIPimage 3.0
 * This MEX-file implements the `drawline` function
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

dip::uint CheckCoordinateArray( mxArray const* mx, dip::uint nDims ) {
   DIP_THROW_IF( !mxIsDouble( mx ) || mxIsComplex( mx ), "Floating-point array expected" );
   DIP_THROW_IF(( mxGetNumberOfDimensions( mx ) != 2 ) || ( mxGetN( mx ) != nDims ), "Coordinate array of wrong size" );
   return mxGetM( mx );
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 3 );
      DML_MAX_ARGS( 5 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      out.Copy( in );
      dip::uint nDims = out.Dimensionality();

      dip::uint N  = CheckCoordinateArray( prhs[ 1 ], nDims );
      dip::uint N2 = CheckCoordinateArray( prhs[ 2 ], nDims );
      DIP_THROW_IF( N != N2, "Coordinate arrays not of same length" );
      double const* mxStart = mxGetPr( prhs[ 1 ] );
      double const* mxEnd   = mxGetPr( prhs[ 2 ] );

      dip::Image::Pixel color = { 255 };
      if( nrhs > 3 ) {
         color.swap( dml::GetPixel( prhs[ 3 ] )); // we cannot assign to a pixel!
      }

      dip::dfloat sigma = 0;
      if( nrhs > 4 ) {
         sigma = dml::GetFloat( prhs[ 4 ] );
      }
      dip::dfloat truncation = 3;
      if( nrhs > 5 ) {
         truncation = dml::GetFloat( prhs[ 5 ] );
      }

      if( sigma == 0.0 ) {
         dip::UnsignedArray start( nDims );
         dip::UnsignedArray end( nDims );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               start[ jj ] = static_cast< dip::uint >( std::round( mxStart[ jj * N ] ));
               end[ jj ] = static_cast< dip::uint >( std::round( mxEnd[ jj * N ] ));
            }
            dip::DrawLine( out, start, end, color );
            ++mxStart;
            ++mxEnd;
         }
      } else {
         dip::FloatArray start( nDims );
         dip::FloatArray end( nDims );
         for( dip::uint ii = 0; ii < N; ++ii ) {
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               start[ jj ] = mxStart[ jj * N ];
               end[ jj ] = mxEnd[ jj * N ];
            }
            dip::DrawBandlimitedLine( out, start, end, color, sigma, truncation );
            ++mxStart;
            ++mxEnd;
         }
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
