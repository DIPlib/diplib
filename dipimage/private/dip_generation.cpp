/*
 * DIPimage 3.0
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

#include "diplib/generation.h"
#include "diplib/boundary.h"

#include "diplib/chain_code.h"

namespace {

dip::Random random;

void coordinates( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MAX_ARGS( 4 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.DataType() = dip::DT_SFLOAT;
   if( nrhs > 0 ) {
      if( mxIsNumeric( prhs[ 0 ] ) && dml::IsVector( prhs[ 0 ] )) {
         out.SetSizes( dml::GetUnsignedArray( prhs[ 0 ] ));
      } else {
         dip::Image tmp = dml::GetImage( prhs[ 0 ] );
         out.SetSizes( tmp.Sizes() );
         out.SetPixelSize( tmp.PixelSize() );
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
         DIP_THROW( dip::E::INVALID_PARAMETER );
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
   plhs[ 0 ] = dml::GetArray( out );
}

dip::uint CheckCoordinateArray( mxArray const* mx, dip::uint nDims ) {
   DIP_THROW_IF( !mxIsDouble( mx ) || mxIsComplex( mx ), "Floating-point array expected" );
   DIP_THROW_IF(( mxGetNumberOfDimensions( mx ) != 2 ) || ( mxGetN( mx ) != nDims ), "Coordinate array of wrong size" );
   return mxGetM( mx );
}

void drawline( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 3 );
   DML_MAX_ARGS( 7 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint nDims = in.Dimensionality();
   dip::uint N  = CheckCoordinateArray( prhs[ 1 ], nDims );
   dip::uint N2 = CheckCoordinateArray( prhs[ 2 ], nDims );
   DIP_THROW_IF( N != N2, "Coordinate arrays not of same length" );
   double const* mxStart = mxGetPr( prhs[ 1 ] );
   double const* mxEnd   = mxGetPr( prhs[ 2 ] );
   dip::Image::Pixel color{ 255 };
   if( nrhs > 3 ) {
      color.swap( dml::GetPixel( prhs[ 3 ] )); // we cannot assign to a pixel!
   }
   dip::dfloat sigma = nrhs > 4 ? dml::GetFloat( prhs[ 4 ] ) : 0.0;
   dip::dfloat truncation = nrhs > 5 ? dml::GetFloat( prhs[ 5 ] ) : 3.0;
   dip::String blend = nrhs > 6 ? dml::GetString( prhs[ 6 ] ) : "assign";
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.Copy( in );
   if( sigma == 0.0 ) {
      dip::UnsignedArray start( nDims );
      dip::UnsignedArray end( nDims );
      for( dip::uint ii = 0; ii < N; ++ii ) {
         for( dip::uint jj = 0; jj < nDims; ++jj ) {
            start[ jj ] = static_cast< dip::uint >( std::round( mxStart[ jj * N ] ));
            end[ jj ] = static_cast< dip::uint >( std::round( mxEnd[ jj * N ] ));
         }
         dip::DrawLine( out, start, end, color, blend );
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
   plhs[ 0 ] = dml::GetArray( out );
}

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
   }
   if( mxIsCell( mx ) && dml::IsVector( mx )) {
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

void drawpolygon( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::Image::Pixel color{ 255 };
   if( nrhs > 2 ) {
      color.swap( dml::GetPixel( prhs[ 2 ] )); // we cannot assign to a pixel!
   }
   dip::String mode = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : dip::S::OPEN;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.Copy( in );
   if( in.Dimensionality() == 2 ) {
      dip::Polygon coords = GetPolygon( prhs[ 1 ] );
      dip::DrawPolygon2D( out, coords, color, mode );
   } else {
      dip::CoordinateArray coords = dml::GetCoordinateArray( prhs[ 1 ] );
      if( mode == dip::S::CLOSED ) {
         if( coords.front() != coords.back() ) {
            coords.push_back( coords.front() );
         }
      } else if( mode != dip::S::OPEN ) {
         DIP_THROW_INVALID_FLAG( mode );
      }
      dip::DrawLines( out, coords, color );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void drawshape( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 3 );
   DML_MAX_ARGS( 7 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::FloatArray sizes = dml::GetFloatArray( prhs[ 1 ] );
   dip::FloatArray origin = dml::GetFloatArray( prhs[ 2 ] );
   dip::String shape = nrhs > 3 ? dml::GetString( prhs[ 3 ] ) : "ellipsoid";
   dip::Image::Pixel color{ 255 };
   if( nrhs > 4 ) {
      color.swap( dml::GetPixel( prhs[ 4 ] )); // we cannot assign to a pixel!
   }
   dip::dfloat sigma = nrhs > 5 ? dml::GetFloat( prhs[ 5 ] ) : 0;
   dip::dfloat truncation = nrhs > 6 ? dml::GetFloat( prhs[ 6 ] ) : 3;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.Copy( in );
   if(( shape == "ellipse" ) || shape == "ellipsoid" ) {
      dip::DrawEllipsoid( out, sizes, origin, color );
   } else if(( shape == "disk" ) || shape == "ball" ) {
      DIP_THROW_IF( sizes.size() != 1, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
      if( sigma == 0.0 ) {
         dip::DrawEllipsoid( out, sizes, origin, color );
      } else {
         dip::DrawBandlimitedBall( out, sizes[ 0 ], origin, color, dip::S::FILLED, sigma, truncation );
      }
   } else if(( shape == "circle" ) || shape == "sphere" ) {
      DIP_THROW_IF( sizes.size() != 1, dip::E::ARRAY_PARAMETER_WRONG_LENGTH );
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
      DIP_THROW_INVALID_FLAG( shape );
   }
   plhs[ 0 ] = dml::GetArray( out );
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

void gaussianblob( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   DML_MAX_ARGS( 6 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::uint nDims = in.Dimensionality();
   dip::uint N  = CheckCoordinateArray( prhs[ 1 ], nDims );
   double const* mxCoords = mxGetPr( prhs[ 1 ] );
   // sigma input parameter
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
   // strength input parameter
   dip::dfloat strength = 255;
   bool strengthPerBlob = false;
   bool strengthPerChannel = false;
   double const* mxStrengths = nullptr;
   dip::uint nTElem = in.TensorElements();
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
   // Other input parameters
   bool spatial = nrhs > 4 ? dip::BooleanFromString( dml::GetString( prhs[ 4 ] ), dip::S::SPATIAL, dip::S::FREQUENCY ) : true;
   dip::dfloat truncation = nrhs > 5 ? dml::GetFloat( prhs[ 5 ] ) : 3.0;
   // Draw each blob
   dip::FloatArray coords( nDims );
   dip::FloatArray sigmas( nDims, sigma );
   bool sigmaNeedsConversion = true;
   dip::Image::Pixel value( dip::DT_DFLOAT, strengthPerChannel ? nTElem : 1 );
   dip::dfloat squareRootTwoPi = std::sqrt( 2.0 * dip::pi );
   dip::FloatArray sizes{ in.Sizes() };
   dip::FloatArray origin = in.GetCenter();
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.Copy( in );
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
   plhs[ 0 ] = dml::GetArray( out );
}

void gaussianedgeclip( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat sigma = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 1.0;
   dip::dfloat truncation = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 3.0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::GaussianEdgeClip( in, out, { 1.0 }, sigma, truncation );
   plhs[ 0 ] = dml::GetArray( out );
}

void gaussianlineclip( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::dfloat sigma = nrhs > 1 ? dml::GetFloat( prhs[ 1 ] ) : 1.0;
   bool normalisetoone = nrhs > 2 ? dml::GetBoolean( prhs[ 2 ] ) : false;
   dip::dfloat truncation = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 3.0;
   dip::dfloat value = normalisetoone ? std::sqrt( 2.0 * dip::pi ) * sigma : 1.0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::GaussianLineClip( in, out, { value }, sigma, truncation );
   plhs[ 0 ] = dml::GetArray( out );
}

void noise( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 4 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String type = nrhs > 1 ? dml::GetString( prhs[ 1 ] ) : "gaussian";
   dip::dfloat param1 = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 1.0;
   dip::dfloat param2 = nrhs > 3 ? dml::GetFloat( prhs[ 3 ] ) : 0.0;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   if( type == "gaussian" ) {
      dip::GaussianNoise( in, out, random, param1 * param1 );
   } else if( type == "uniform" ) {
      dip::UniformNoise( in, out, random, param1, param2 );
   } else if( type == "poisson" ) {
      dip::PoissonNoise( in, out, random, param1 );
   } else if( type == "binary" ) {
      dip::BinaryNoise( in, out, random, param1, param2 );
   } else if( type == "saltpepper" ) {
      dip::SaltPepperNoise( in, out, random, param1, param2, 255.0 );
   } else if( type == "brownian" ) {
      dip::ColoredNoise( in, out, random, param1 * param1, -2.0 );
   } else if( type == "pink" ) {
      if( param2 <= 0.0 ) {
         param2 = 1.0;
      }
      dip::ColoredNoise( in, out, random, param1 * param1, -param2 );
   } else if( type == "blue" ) {
      if( param2 <= 0.0 ) {
         param2 = 1.0;
      }
      dip::ColoredNoise( in, out, random, param1 * param1, param2 );
   } else if( type == "violet" ) {
      dip::ColoredNoise( in, out, random, param1 * param1, 2.0 );
   } else {
      DIP_THROW_INVALID_FLAG( type );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void setborder( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::UnsignedArray border = nrhs > 2 ? dml::GetUnsignedArray( prhs[ 2 ] ) : dip::UnsignedArray{ 1 };
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.Copy( in );
   if( nrhs > 1 ) {
      dip::Image::Pixel value = dml::GetPixel( prhs[ 1 ] );
      dip::SetBorder( out, value, border );
   } else {
      dip::SetBorder( out, { 0 }, border );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

void testobject( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   int index = 0;
   dip::TestObjectParams params;
   if(( nrhs > 0 ) && !mxIsChar( prhs[ 0 ] )) {
      // testobject(image, [name-value pairs])
      dip::Image tmp = dml::GetImage( prhs[ 0 ] );
      dip::Convert( tmp, out, dip::DataType::SuggestFloat( tmp.DataType() ));
      params.objectShape = "custom";
      index = 1;
   } else {
      // testobject(object, imgSizes, objSizes, [name-value pairs])
      params.objectShape = ( nrhs > 0 ) ? dml::GetString( prhs[ 0 ] ) : dip::String( dip::S::ELLIPSOID );
      dip::UnsignedArray imgSizes = ( nrhs > 1 ) ? dml::GetUnsignedArray( prhs[ 1 ] ) : dip::UnsignedArray{ 256, 256 };
      params.objectSizes = ( nrhs > 2 ) ? dml::GetFloatArray( prhs[ 2 ] ) : dip::FloatArray{ 128, 128 };
      out.ReForge( imgSizes, 1, dip::DT_SFLOAT );
      index = 3;
   }
   // Name-value pairs
   int n = nrhs - index;
   DIP_THROW_IF(( n > 0 ) && ( n & 1 ), "Wrong number of input arguments, an even number of arguments needed for the name-value pairs" );
   while( index < nrhs ) {
      dip::String name = dml::GetString( prhs[ index ] );
      ++index;
      if( name == "objectAmplitude" ) {
         params.objectAmplitude = dml::GetFloat( prhs[ index ] );
      } else if( name == "randomShift" ) {
         params.randomShift = dml::GetBoolean( prhs[ index ] );
      } else if( name == "generationMethod" ) {
         params.generationMethod = dml::GetString( prhs[ index ] );
      } else if( name == "modulationDepth" ) {
         params.modulationDepth = dml::GetFloat( prhs[ index ] );
      } else if( name == "modulationFrequency" ) {
         params.modulationFrequency = dml::GetFloatArray( prhs[ index ] );
      } else if( name == "pointSpreadFunction" ) {
         params.pointSpreadFunction = dml::GetString( prhs[ index ] );
      } else if( name == "oversampling" ) {
         params.oversampling = dml::GetFloat( prhs[ index ] );
      } else if( name == "backgroundValue" ) {
         params.backgroundValue = dml::GetFloat( prhs[ index ] );
      } else if( name == "signalNoiseRatio" ) {
         params.signalNoiseRatio = dml::GetFloat( prhs[ index ] );
      } else if( name == "gaussianNoise" ) {
         params.gaussianNoise = dml::GetFloat( prhs[ index ] );
      } else if( name == "poissonNoise" ) {
         params.poissonNoise = dml::GetFloat( prhs[ index ] );
      } else {
         DIP_THROW( "Invalid parameter name: " + name );
      }
      ++index;
   }
   dip::TestObject( out, params, random );
   plhs[ 0 ] = dml::GetArray( out );
}

void window( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 1 );
   DML_MAX_ARGS( 3 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::String type = nrhs > 1 ? dml::GetString( prhs[ 1 ] ) : "Hamming";
   dip::dfloat parameter = nrhs > 2 ? dml::GetFloat( prhs[ 2 ] ) : 0.5;
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   dip::ApplyWindow( in, out, type, parameter );
   plhs[ 0 ] = dml::GetArray( out );
}

void extendregion( mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   DML_MIN_ARGS( 2 );
   dip::Image const in = dml::GetImage( prhs[ 0 ] );
   dip::RangeArray ranges;
   dip::UnsignedArray origin;
   dip::UnsignedArray sizes;
   bool useRanges;
   int index = 1;
   if( mxIsCell( prhs[ index ] )) {
      useRanges = true;
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
   DML_MAX_ARGS( index + 1 );
   dip::StringArray bc = nrhs > index ? dml::GetStringArray( prhs[ index ]) : dip::StringArray{};
   dml::MatlabInterface mi;
   dip::Image out = mi.NewImage();
   out.Copy( in );
   if( useRanges ) {
      dip::ExtendRegion( out, ranges, bc );
   } else {
      dip::ExtendRegion( out, origin, sizes, bc );
   }
   plhs[ 0 ] = dml::GetArray( out );
}

} // namespace

// Gateway function

void mexFunction( int /*nlhs*/, mxArray* plhs[], int nrhs, const mxArray* prhs[] ) {
   try {
      DML_MIN_ARGS( 1 );
      dip::String function = dml::GetString( prhs[ 0 ] );
      prhs += 1;
      nrhs -= 1;

      if( function == "coordinates" ) {
         coordinates( plhs, nrhs, prhs );
      } else if( function == "drawline" ) {
         drawline( plhs, nrhs, prhs );
      } else if( function == "drawpolygon" ) {
         drawpolygon( plhs, nrhs, prhs );
      } else if( function == "drawshape" ) {
         drawshape( plhs, nrhs, prhs );
      } else if( function == "gaussianblob" ) {
         gaussianblob( plhs, nrhs, prhs );
      } else if( function == "gaussianedgeclip" ) {
         gaussianedgeclip( plhs, nrhs, prhs );
      } else if( function == "gaussianlineclip" ) {
         gaussianlineclip( plhs, nrhs, prhs );
      } else if( function == "noise" ) {
         noise( plhs, nrhs, prhs );
      } else if( function == "setborder" ) {
         setborder( plhs, nrhs, prhs );
      } else if( function == "testobject" ) {
         testobject( plhs, nrhs, prhs );
      } else if( function == "window" ) {
         window( plhs, nrhs, prhs );

      } else if( function == "extendregion" ) {
         extendregion( plhs, nrhs, prhs );

      } else {
         DIP_THROW_INVALID_FLAG( function );
      }

   } DML_CATCH
}
