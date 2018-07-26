/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the FIR Gabor filter.
 *
 * (c)2018, Cris Luengo.
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

#include "diplib.h"
#include "diplib/linear.h"
#include "diplib/generation.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/iterators.h"

namespace dip {

namespace {

inline dip::uint HalfGaborSize(
      dfloat sigma,
      dfloat truncation
) {
   return clamp_cast< dip::uint >( std::ceil( truncation * sigma ));
}

// Creates half of a complex-valued Gabor kernel, with the x=0 at the right end (last element) of the output array.
// `out[0] + i*out[1]` is the first element, etc. As expected by `OneDimensionalFilter`.
std::vector< dfloat > MakeHalfGabor(
      dfloat sigma,
      dfloat frequency,
      dfloat truncation
) {
   dip::uint halfFilterSize = 1 + HalfGaborSize( sigma, truncation );
   std::vector< dfloat > filter( 2 * halfFilterSize );
   auto ptr = reinterpret_cast< dcomplex* >( filter.data() );
   dip::uint r0 = halfFilterSize - 1;
   dfloat factor = -1.0 / ( 2.0 * sigma * sigma );
   dfloat norm = 1.0 / ( std::sqrt( 2.0 * pi ) * sigma );
   frequency *= 2 * pi;
   ptr[ r0 ] = norm;
   for( dip::uint rr = 1; rr < halfFilterSize; rr++ ) {
      dfloat rad = static_cast< dfloat >( rr );
      dfloat g = std::exp( factor * ( rad * rad )) * norm;
      dfloat phi = rad * frequency;
      ptr[ r0 - rr ] = g * dcomplex{ std::cos( phi ), -std::sin( phi ) };
   }
   return filter;
}

// Create 1D full Gabor filter
std::vector< dfloat > MakeGabor(
      dfloat sigma,
      dfloat frequency,
      dfloat truncation
) {
   // Handle sigma == 0.0
   if( sigma == 0.0 ) {
      return { 1.0 };
   }
   // Create half Gabor
   std::vector< dfloat > gabor;
   DIP_STACK_TRACE_THIS( gabor = MakeHalfGabor( sigma, frequency, truncation ));
   dip::uint halfFilterSize = gabor.size() / 2 - 1;
   // Complete the Gabor
   gabor.resize( 2 * ( halfFilterSize * 2 + 1 ));
   auto ptr = reinterpret_cast< dcomplex* >( gabor.data() );
   for( dip::uint ii = 1; ii <= halfFilterSize; ++ii ) {
      ptr[ halfFilterSize + ii ] = std::conj( ptr[ halfFilterSize - ii ] );
   }
   return gabor;
}

} // namespace

void CreateGabor(
      Image& out,
      FloatArray const& sigmas,
      FloatArray const& frequencies,
      dfloat truncation
) {
   // Verify dimensionality
   dip::uint nDims = sigmas.size();
   DIP_THROW_IF( frequencies.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );

   // Adjust truncation to default if needed
   if( truncation <= 0.0 ) {
      truncation = 3.0;
   }

   // Create 1D gaussian for each dimension
   std::vector< std::vector< dfloat >> gabors( nDims );
   std::vector< dcomplex const* > ptrs( nDims );
   UnsignedArray outSizes( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_STACK_TRACE_THIS( gabors[ ii ] = MakeGabor( sigmas[ ii ], frequencies[ ii ], truncation ));
      outSizes[ ii ] = gabors[ ii ].size() / 2;
      ptrs[ ii ] =  reinterpret_cast< dcomplex* >( gabors[ ii ].data() );
   }

   // Create output image
   std::cout << "[CreateGabor] sigmas = " << sigmas << '\n';
   std::cout << "[CreateGabor] outSizes = " << outSizes << '\n';
   out.ReForge( outSizes, 1, DT_DCOMPLEX );
   std::cout << "[CreateGabor] out = " << out << '\n';
   ImageIterator< dcomplex > itOut( out );
   do {
      const UnsignedArray& coords = itOut.Coordinates();
      // Multiply 1D Kernels
      dcomplex value = 1.0;
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         value *= ptrs[ ii ][ coords[ ii ]];
      }
      *itOut = value;
   } while( ++itOut );
}

void GaborFIR(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      FloatArray const& frequencies,
      StringArray const& boundaryCondition,
      BooleanArray process,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_START_STACK_TRACE
      ArrayUseParameter( sigmas, nDims, 1.0 );
      ArrayUseParameter( process, nDims, true );
   DIP_END_STACK_TRACE
   OneDimensionalFilterArray filter( nDims );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if(( sigmas[ ii ] > 0.0 ) && ( in.Size( ii ) > 1 )) {
         bool found = false;
         for( dip::uint jj = 0; jj < ii; ++jj ) {
            if( process[ jj ] && ( sigmas[ jj ] == sigmas[ ii ] ) && ( frequencies[ jj ] == frequencies[ ii ] )) {
               filter[ ii ] = filter[ jj ];
               found = true;
               break;
            }
         }
         if( !found ) {
            filter[ ii ].symmetry = S::CONJ;
            filter[ ii ].isComplex = true;
            filter[ ii ].filter = MakeHalfGabor( sigmas[ ii ], frequencies[ ii ], truncation );
            // NOTE: origin defaults to the middle of the filter, so we don't need to set it explicitly here.
         }
      } else {
         process[ ii ] = false;
      }
   }
   SeparableConvolution( in, out, filter, boundaryCondition, process );
}


namespace {

void ApplyScaleFilters(
      Image const& ftIn,
      Image& radius,
      ImageArray& outar,
      FloatArray const& wavelengths,
      dfloat bandwidth
) {
   dfloat expScaling = std::log( bandwidth );
   expScaling = 1.0 / ( 2.0 * expScaling * expScaling );

   UnsignedArray center = radius.Sizes();
   center /= 2;
   radius.At( center ) = 1;

   // TODO: Kovesi also makes a low-pass filter to remove the frequency components "in the corners". Is this important?

   dip::uint nFrequencyScales = wavelengths.size();
   outar.resize( nFrequencyScales );
   for( dip::uint scale = 0; scale < nFrequencyScales; ++scale ) {
      Image tmp;
      if( !ftIn.IsForged() ) {
         outar[ scale ].ReForge( radius.Sizes(), 1, DT_SFLOAT );
         tmp = outar[ scale ];
         tmp.Protect();
      }
      dfloat wavelength = wavelengths[ scale ];
      auto lineFilter = Framework::NewMonadicScanLineFilter< sfloat >( [ wavelength, expScaling ]( auto its ){
         return static_cast< sfloat >( std::exp( -std::pow( std::log( *its[ 0 ] * wavelength ), 2 ) * expScaling ));
      }, 50 );
      Framework::ScanMonadic( radius, tmp, DT_SFLOAT, DT_SFLOAT, 1, *lineFilter );
      if( ftIn.IsForged() ) {
         Multiply( tmp, ftIn, outar[ scale ] ); // output is complex-valued
      }
      outar[ scale ].At( center ) = 0;
   }
}

} // namespace

void LogGaborFilterBank(
      Image const& in,
      Image& out,
      FloatArray const& wavelengths,
      dfloat bandwidth,
      dip::uint nOrientations,
      String const& inRepresentation,
      String const& outRepresentation
) {
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.Sizes().all(), "Raw image sizes not valid" ); // must test valid sizes in case it's not forged
   dip::uint nFrequencyScales = wavelengths.size();
   DIP_THROW_IF( nFrequencyScales < 1, E::ARRAY_PARAMETER_EMPTY );
   DIP_THROW_IF( nOrientations < 1, E::PARAMETER_OUT_OF_RANGE );
   DIP_THROW_IF( bandwidth <= 0, E::PARAMETER_OUT_OF_RANGE );
   bool onlyScale = nOrientations == 1;
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( !onlyScale && ( nDims != 2 ), E::DIMENSIONALITY_NOT_SUPPORTED );
   bool spatialDomainOutput;
   DIP_STACK_TRACE_THIS( spatialDomainOutput = BooleanFromString( outRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool inputIsReal = true; // if no input image is given, it's like we're given a delta pulse image -- it's real.
   Image ftIn;
   if( in.IsForged() ) {
      // Get Fourier-domain representation of input image
      bool spatialDomainInput;
      DIP_STACK_TRACE_THIS( spatialDomainInput = BooleanFromString( inRepresentation, S::SPATIAL, S::FREQUENCY ));
      if( spatialDomainInput ) {
         inputIsReal = !in.DataType().IsComplex();
         ftIn = FourierTransform( in );
      } else {
         inputIsReal = false;
         ftIn = in;
         if( ftIn.Aliases( out )) {
            out.Strip(); // cannot work in-place
         }
      }
   } else {
      ftIn.SetSizes( in.Sizes() ); // Copy over the sizes array.
   }

   // Options:
   //  - !in.IsForged()
   //     - inputIsReal
   //        - onlyScale
   //           - FD output => real
   //           - SD output => real
   //        - !onlyScale
   //           - FD output => real
   //           - SD output => complex
   //     - !inputIsReal
   //        - does not happen!
   //  - in.IsForged()
   //     - inputIsReal
   //        - onlyScale
   //           - FD output => complex
   //           - SD output => real
   //        - !onlyScale
   //           - FD output => complex
   //           - SD output => complex
   //     - !inputIsReal
   //        - onlyScale
   //           - FD output => complex
   //           - SD output => complex
   //        - !onlyScale
   //           - FD output => complex
   //           - SD output => complex
   bool ouputIsReal = ( !in.IsForged() && ( onlyScale || !spatialDomainOutput ))
                      || ( in.IsForged() && inputIsReal && onlyScale && spatialDomainOutput );

   // Options for inverse transform
   StringSet options = { S::INVERSE };
   if( spatialDomainOutput && ouputIsReal ) {
      options.insert( S::REAL );
   }

   // Forge output tensor image
   DataType outDataType = ouputIsReal ? DT_SFLOAT : DT_SCOMPLEX;
   out.ReForge( ftIn.Sizes(), nOrientations * nFrequencyScales, outDataType, Option::AcceptDataTypeChange::DONT_ALLOW );
   out.ReshapeTensor( nOrientations, nFrequencyScales );

   // Create coordinates image
   Image coord = CreateCoordinates( ftIn.Sizes(), { "frequency" } );
   Image radius = Norm( coord );
   UnsignedArray center = ftIn.Sizes();
   center /= 2;
   radius.At( center ) = 1; // Value at origin should not be 0, so we can take the log later on
   DIP_ASSERT( radius.DataType() == DT_SFLOAT );

   if( onlyScale ) {

      // Create scale filtered images
      // ApplyScaleFilters produces a SCOMPLEX output if ftIn.IsForged(), otherwise it is SFLOAT
      // If the type matches the output type's, write directly to output
      ImageArray outar( nFrequencyScales );
      if( ftIn.IsForged() ^ ouputIsReal ) {
         for( dip::uint scale = 0; scale < nFrequencyScales; ++scale ) {
            outar[ scale ] = out[ scale ];
            outar[ scale ].Protect();
         }
      }
      ApplyScaleFilters( ftIn, radius, outar, wavelengths, bandwidth );

      if( spatialDomainOutput ) {
         // Apply inverse Fourier transform
         for( dip::uint scale = 0; scale < nFrequencyScales; ++scale ) {
            Image tmp = out[ scale ];
            tmp.Protect();
            FourierTransform( outar[ scale ], tmp, options );
         }
      }
      return; // We're done!
   }

   // If we're here, we're dealing with a 2D image, and want 2 or more orientations.

   // Apply scale filters to the input
   ImageArray scaleFilter;
   ApplyScaleFilters( ftIn, radius, scaleFilter, wavelengths, bandwidth );
   coord /= radius;
   DIP_ASSERT( coord.DataType() == DT_SFLOAT );

   // Construct and apply angle filters to the previous result
   dfloat sigmaTheta = pi / static_cast< dfloat >( nOrientations ) / 1.3; // magic constant, see Kovesi.
   dfloat expScaling = 1.0 / ( 2.0 * sigmaTheta * sigmaTheta );
   for( dip::uint orientation = 0; orientation < nOrientations; ++orientation ) {
      // Construct angle selection filter
      dfloat angle = static_cast< dfloat >( orientation ) * pi / static_cast< dfloat >( nOrientations );
      Image::Pixel rotMatrix{ sfloat( std::cos( angle )), sfloat( std::sin( angle )), -sfloat( std::sin( angle )), sfloat( std::cos( angle )) };
      rotMatrix.ReshapeTensor( 2, 2 );
      Image radialFilter = rotMatrix * coord;
      Angle( radialFilter, radialFilter );
      auto lineFilter = Framework::NewMonadicScanLineFilter< sfloat >( [ expScaling ]( auto its ){
         return static_cast< sfloat >( std::exp(( *its[ 0 ] ) * ( *its[ 0 ] ) * -expScaling ));
      }, 30 );
      Framework::ScanMonadic( radialFilter, radialFilter, DT_SFLOAT, DT_SFLOAT, 1, *lineFilter );
      // Filter each scale with this angle selection filter
      for( dip::uint scale = 0; scale < nFrequencyScales; ++scale ) {
         Image destination = out[ UnsignedArray{ orientation, scale } ];
         destination.Protect();          // ensure it will not be reforged
         Image tempStorage;
         Image& ftDestination = ( spatialDomainOutput && ouputIsReal ) ? tempStorage : destination;
         Multiply( radialFilter, scaleFilter[ scale ], ftDestination );
         if( spatialDomainOutput ) {
            FourierTransform( ftDestination, destination, options );
         }
      }
   }
}

} // namespace dip
