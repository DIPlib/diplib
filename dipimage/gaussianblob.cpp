/*
 * DIPimage 3.0
 * This MEX-file implements the `gaussianblob` function
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

std::pair< dip::uint, dip::uint > CheckValueArray( mxArray const* mx, dip::uint N, dip::uint nDims ) {
   DIP_THROW_IF( !mxIsDouble( mx ) || mxIsComplex( mx ), "Floating-point array expected" );
   DIP_THROW_IF( mxGetNumberOfDimensions( mx ) != 2, "Value array of wrong size" );
   dip::uint cols = mxGetN( mx );
   DIP_THROW_IF(( cols != 1 ) && ( cols != nDims ), "Value array of wrong size" );
   dip::uint rows = mxGetM( mx );
   DIP_THROW_IF(( rows != 1 ) && ( rows != N ), "Value array of wrong size" );
   return std::make_pair( rows, cols );
}

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {

      DML_MIN_ARGS( 2 );
      DML_MAX_ARGS( 6 );

      dml::MatlabInterface mi;
      dip::Image const in = dml::GetImage( prhs[ 0 ] );
      dip::Image out = mi.NewImage();
      out.Copy( in );
      dip::uint nDims = out.Dimensionality();

      dip::uint N  = CheckCoordinateArray( prhs[ 1 ], nDims );
      double const* mxCoords = mxGetPr( prhs[ 1 ] );

      dip::dfloat sigma = 2;
      bool sigmaPerBlob = false;
      bool sigmaPerDim = false;
      double const* mxSigmas = nullptr;
      dip::uint sigmaStride = 1;
      if( nrhs > 2 ) {
         dip::uint rows, cols;
         std::tie( rows, cols ) = CheckValueArray( prhs[ 2 ], N, nDims );
         sigmaPerBlob = rows > 1;
         sigmaPerDim = cols > 1;
         mxSigmas = mxGetPr( prhs[ 2 ] );
         if( !sigmaPerBlob && !sigmaPerDim ) {
            sigma = *mxSigmas;
         }
         if( sigmaPerBlob ) {
            sigmaStride = N;
         }
      }

      dip::dfloat strength = 255;
      bool strengthPerBlob = false;
      bool strengthPerChannel = false;
      double const* mxStrengths = nullptr;
      dip::uint nTElem = out.TensorElements();
      dip::uint strengthStride = 1;
      if( nrhs > 3 ) {
         dip::uint rows, cols;
         std::tie( rows, cols ) = CheckValueArray( prhs[ 3 ], N, nTElem );
         strengthPerBlob = rows > 1;
         strengthPerChannel = cols > 1;
         mxStrengths = mxGetPr( prhs[ 3 ] );
         if( !strengthPerBlob && !strengthPerChannel ) {
            strength = *mxStrengths;
         }
         if( strengthPerBlob ) {
            strengthStride = N;
         }
      }

      bool spatial = true;
      if( nrhs > 4 ) {
         dip::String domain = dml::GetString( prhs[ 4 ] );
         spatial = dip::BooleanFromString( domain, dip::S::SPATIAL, dip::S::FREQUENCY );
      }

      dip::dfloat truncation = 3;
      if( nrhs > 5 ) {
         truncation = dml::GetFloat( prhs[ 5 ] );
      }

      dip::FloatArray coords( nDims );
      dip::FloatArray sigmas( nDims, sigma );
      bool sigmaNeedsConversion = true;
      dip::Image::Pixel value( dip::DT_DFLOAT, strengthPerChannel ? nTElem : 1 );
      dip::dfloat squareRootTwoPi = std::sqrt( 2.0 * dip::pi );
      dip::FloatArray sizes( nDims );
      dip::FloatArray origin( nDims );
      for( dip::uint jj = 0; jj < nDims; ++jj ) {
         sizes[ jj ] = static_cast< dip::dfloat >( out.Size( jj ));
         origin[ jj ] = static_cast< dip::dfloat >( out.Size( jj ) / 2 );
      }
      for( dip::uint ii = 0; ii < N; ++ii ) {
         // Copy coordinates
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            coords[ jj ] = mxCoords[ jj * N ];
         }
         ++mxCoords;
         // Copy sigmas
         if( sigmaPerDim ) {
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               sigmas[ jj ] = mxSigmas[ jj * sigmaStride ];
            }
            sigmaNeedsConversion = true;
         } else if( sigmaPerBlob ){
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               sigmas[ jj ] = *mxSigmas;
            }
            sigmaNeedsConversion = true;
         } // else: We've already filled in sigmas. Do the conversion only in the first pass through the loop.
         if( sigmaPerBlob ) {
            ++mxSigmas;
         }
         // Copy strength
         if( strengthPerChannel ) {
            for( dip::uint jj = 0; jj < nTElem; ++jj ) {
               value[ jj ] = mxStrengths[ jj * strengthStride ];
            }
         } else if( strengthPerBlob ){
            value[ 0 ] = *mxStrengths;
         } else {
            value[ 0 ] = strength;
         }
         if( strengthPerBlob ) {
            ++mxStrengths;
         }
         // If in frequency domain, convert to image domain values.
         if( !spatial ) {
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               coords[ jj ] = coords[ jj ] * sizes[ jj ] + origin[ jj ];
            }
            if( sigmaNeedsConversion ) {
               for( dip::uint jj = 0; jj < nDims; ++jj ) {
                  sigmas[ jj ] = sizes[ jj ] / ( 2.0 * dip::pi * sigmas[ jj ] );
               }
            }
            // Fix blob strength so we undo the normalization within dip::DrawBandlimitedPoint
            dip::dfloat normalization = 1.0;
            for( dip::uint jj = 0; jj < nDims; ++jj ) {
               normalization /= squareRootTwoPi * sigmas[ jj ];
            }
            value /= normalization;
         }
         sigmaNeedsConversion = false;
         // Draw the blob in the image
         dip::DrawBandlimitedPoint( out, coords, value, sigmas, truncation );
      }

      plhs[ 0 ] = mi.GetArray( out );

   } catch( const dip::Error& e ) {
      mexErrMsgTxt( e.what() );
   }
}
