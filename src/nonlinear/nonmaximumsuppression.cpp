/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the non-maximum suppression.
 *
 * (c)2017-2019, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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
#include "diplib/nonlinear.h"
#include "diplib/math.h"
#include "diplib/overload.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/neighborlist.h"

namespace dip {

namespace {

template< typename TPI >
void NonMaximumSuppression1D(
      Image const& gradmag,
      Image const& mask,
      Image& out
) {
   dip::uint size = gradmag.Size( 0 );
   dip::sint instride = gradmag.Stride( 0 );
   TPI const* pin = static_cast< TPI const* >( gradmag.Origin() );
   dip::sint outstride = out.Stride( 0 );
   TPI* pout = static_cast< TPI* >( out.Origin() );
   dip::sint maskstride = 0;
   bin* pmask = nullptr;
   if( mask.IsForged() ) {
      maskstride = mask.Stride( 0 );
      pmask = static_cast< bin* >( mask.Origin() );
   }
   // First pixel
   *pout = 0;
   pin += instride;
   pout += outstride;
   pmask += maskstride; // if !pmask, adds zero to the nullptr.
   // The bulk of the pixels
   for( dip::uint ii = 1; ii < size - 1; ++ii ) {
      if(( !pmask || *pmask ) && ( *pin > 0 )) {
         TPI m1 = *(pin-instride);
         TPI m2 = *(pin+instride);
         if((( *pin > m1 ) && ( *pin >= m2 )) || (( *pin >= m1 ) && ( *pin > m2 ))) {
            *pout = *pin;
         } else {
            *pout = 0;
         }
      } else {
         *pout = 0;
      }
      pin += instride;
      pout += outstride;
      pmask += maskstride;
   }
   // Last pixel
   *pout = 0;
}

bool IsOnEdge( UnsignedArray const& coords, UnsignedArray const& sizes, dip::uint procDim ) {
   for( dip::uint ii = 0; ii < coords.size(); ++ii ) {
      if( ii != procDim ) {
         if(( coords[ ii ] == 0 ) || ( coords[ ii ] == sizes[ ii ] - 1 )) {
            return true;
         }
      }
   }
   return false;
}

template< typename TPI >
class NonMaximumSuppression2D : public Framework::ScanLineFilter {
   public:
      NonMaximumSuppression2D( UnsignedArray const& sizes, IntegerArray const& gradmagStrides, bool interpolate ) :
            sizes_( sizes ), gradmagStrides_( gradmagStrides ), interpolate_( interpolate ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return interpolate_ ? 20 : 12;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         dip::sint gmstridex = gradmagStrides_[ 0 ];
         dip::sint gmstridey = gradmagStrides_[ 1 ];
         TPI const* pgm = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint gmStride = params.inBuffer[ 0 ].stride;
         TPI const* pgv = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         dip::sint gvStride = params.inBuffer[ 1 ].stride;
         dip::sint tensorStride = params.inBuffer[ 1 ].tensorStride;
         bin const* pmask = nullptr;
         dip::sint maskStride = 0;
         if( params.inBuffer.size() > 2 ) {
            // If there are three input buffers, we have a mask image.
            pmask = static_cast< bin const* >( params.inBuffer[ 2 ].buffer );
            maskStride = params.inBuffer[ 2 ].stride;
         }
         TPI* pout = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         auto bufferLength = params.bufferLength;
         auto prodDim = params.dimension;
         auto const& coords = params.position;

         // Are we on an edge?
         bool onEdge = IsOnEdge( coords, sizes_, prodDim );
         if( onEdge ) {
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               *pout = 0;
               pout += outStride;
            }
            return;
         }

         // First pixel
         *pout = 0;

         // The bulk of the pixels
         for( dip::uint ii = 2; ii < bufferLength; ++ii ) {
            pgm += gmStride;
            pgv += gvStride;
            pout += outStride;
            pmask += maskStride; // if !pmask, adds zero to the nullptr.

            if(( !pmask || *pmask ) && ( *pgm > 0 )) {

               // Gradient at current location
               TPI dx = pgv[ 0 ];
               TPI dy = pgv[ tensorStride ];
               TPI absdx = std::abs( dx );
               TPI absdy = std::abs( dy );

               TPI m1, m2;
               if( interpolate_ ) {

                  // Get gradient magnitude values to interpolate between
                  TPI delta, mag1, mag2, mag3, mag4;
                  if( absdy > absdx ) {
                     delta = absdx / absdy;
                     // Get values above and below the current point
                     mag2 = *( pgm - gmstridey );
                     mag4 = *( pgm + gmstridey );
                     // Get values diagonally w.r.t. the current point
                     if( std::signbit( dx ) != std::signbit( dy )) {
                        mag1 = *( pgm - gmstridey + gmstridex );
                        mag3 = *( pgm + gmstridey - gmstridex );
                     } else {
                        mag1 = *( pgm - gmstridey - gmstridex );
                        mag3 = *( pgm + gmstridey + gmstridex );
                     }
                  } else {
                     delta = absdy / absdx;
                     // Get values left and right of the current point
                     mag2 = *( pgm + gmstridex );
                     mag4 = *( pgm - gmstridex );
                     // Get values diagonally w.r.t. the current point
                     if( std::signbit( dx ) != std::signbit( dy )) {
                        mag1 = *( pgm - gmstridey + gmstridex );
                        mag3 = *( pgm + gmstridey - gmstridex );
                     } else {
                        mag1 = *( pgm + gmstridey + gmstridex );
                        mag3 = *( pgm - gmstridey - gmstridex );
                     }
                  }

                  // Interpolate gradient magnitude values
                  m1 = delta * mag1 + ( 1 - delta ) * mag2;
                  m2 = delta * mag3 + ( 1 - delta ) * mag4;

               } else {

                  // Get gradient magnitude values at nearest integer location
                  dip::sint ss;
                  if( absdx > absdy ) {
                     ss = round_cast( dy / dx ); // dy rounded to -1, 0 or 1
                     ss *= gmstridey;
                     ss += gmstridex;            // dx is 1
                  } else {
                     ss = round_cast( dx / dy ); // dx rounded to -1, 0 or 1
                     ss *= gmstridex;
                     ss += gmstridey;            // dy is 1
                  }
                  m1 = *( pgm + ss );
                  m2 = *( pgm - ss );

               }

               // Determine if the current point is a maximum point
               // Note that at most one side is allowed to be 'flat'
               if((( *pgm > m1 ) && ( *pgm >= m2 )) || (( *pgm >= m1 ) && ( *pgm > m2 ))) {
                  *pout = *pgm;
               } else {
                  *pout = 0;
               }
            } else {
               *pout = 0;
            }
         }

         // Last pixel
         pout += outStride;
         *pout = 0;
      }
   private:
      UnsignedArray const& sizes_;
      IntegerArray const& gradmagStrides_;
      bool interpolate_;
};

template< typename TPI >
class NonMaximumSuppressionND : public Framework::ScanLineFilter {
   public:
      NonMaximumSuppressionND( UnsignedArray const& sizes, IntegerArray const& gradmagStrides ) :
            sizes_( sizes ), gradmagStrides_( gradmagStrides ) {}
      virtual dip::uint GetNumberOfOperations( dip::uint, dip::uint, dip::uint ) override {
         return 6 * sizes_.size() + 2;
      }
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* pgm = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         dip::sint gmStride = params.inBuffer[ 0 ].stride;
         TPI const* pgv = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         dip::sint gvStride = params.inBuffer[ 1 ].stride;
         dip::sint tensorStride = params.inBuffer[ 1 ].tensorStride;
         bin const* pmask = nullptr;
         dip::sint maskStride = 0;
         if( params.inBuffer.size() > 2 ) {
            // If there are three input buffers, we have a mask image.
            pmask = static_cast< bin const* >( params.inBuffer[ 2 ].buffer );
            maskStride = params.inBuffer[ 2 ].stride;
         }
         TPI* pout = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         auto bufferLength = params.bufferLength;
         auto prodDim = params.dimension;
         auto const& coords = params.position;
         auto nDims = params.inBuffer[ 1 ].tensorLength;
         DIP_ASSERT( sizes_.size() == nDims );
         DIP_ASSERT( coords.size() == nDims );

         // Are we on an edge?
         bool onEdge = IsOnEdge( coords, sizes_, prodDim );
         if( onEdge ) {
            for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
               *pout = 0;
               pout += outStride;
            }
            return;
         }

         // First pixel
         *pout = 0;

         // The bulk of the pixels
         FloatArray g( nDims );
         for( dip::uint ii = 2; ii < bufferLength; ++ii ) {
            pgm += gmStride;
            pgv += gvStride;
            pout += outStride;
            pmask += maskStride; // if !pmask, adds zero to the nullptr.

            if(( !pmask || *pmask ) && ( *pgm > 0 )) {

               // Max gradient value at current location
               TPI max = std::abs( pgv[ 0 ] );
               for( dip::uint dd = 1; dd < nDims; ++dd ) {
                  max = std::max( max, std::abs( pgv[ static_cast< dip::sint >( dd ) * tensorStride ] ));
               }

               // Get gradient magnitude values at nearest integer location
               dip::sint ss = round_cast( pgv[ 0 ] / max ) * gradmagStrides_[ 0 ];
               for( dip::uint dd = 1; dd < nDims; ++dd ) {
                  ss += round_cast( pgv[ static_cast< dip::sint >( dd ) * tensorStride ] / max ) * gradmagStrides_[ dd ];
               }
               TPI m1 = *( pgm + ss );
               TPI m2 = *( pgm - ss );

               // Determine if the current point is a maximum point
               // Note that at most one side is allowed to be 'flat'
               if((( *pgm > m1 ) && ( *pgm >= m2 )) || (( *pgm >= m1 ) && ( *pgm > m2 ))) {
                  *pout = *pgm;
               } else {
                  *pout = 0;
               }
            } else {
               *pout = 0;
            }
         }

         // Last pixel
         pout += outStride;
         *pout = 0;
      }
   private:
      UnsignedArray const& sizes_;
      IntegerArray const& gradmagStrides_;
};

} // namespace

void NonMaximumSuppression(
      Image const& c_gradmag,
      Image const& gradient,
      Image const& c_mask,
      Image& out,
      String const& mode
) {
   Image gradmag;
   dip::uint nDims;
   DataType ovlType;
   if( c_gradmag.IsForged() && ( c_gradmag.Dimensionality() == 1 )) {

      // In this case we can ignore gradient
      gradmag = c_gradmag.QuickCopy();
      ovlType = gradmag.DataType();
      nDims = 1;

   } else {

      DIP_THROW_IF( !gradient.IsForged(), E::IMAGE_NOT_FORGED );
      ovlType = gradient.DataType();
      nDims = gradient.Dimensionality();
      DIP_THROW_IF(( nDims < 1 ), E::DIMENSIONALITY_NOT_SUPPORTED );
      DIP_THROW_IF( gradient.TensorElements() != nDims, E::NTENSORELEM_DONT_MATCH );
      DIP_THROW_IF( !ovlType.IsFloat(), E::DATA_TYPE_NOT_SUPPORTED );

      gradmag = c_gradmag.QuickCopy();
      if( gradmag.IsForged() ) {
         DIP_THROW_IF( !gradmag.IsScalar(), E::IMAGE_NOT_SCALAR );
         DIP_THROW_IF( gradmag.Sizes() != gradient.Sizes(), E::SIZES_DONT_MATCH );
         DIP_THROW_IF( gradmag.DataType() != ovlType, E::DATA_TYPES_DONT_MATCH );
      } else {
         DIP_STACK_TRACE_THIS( gradmag = Norm( gradient ));
      }

   }

   Image mask;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( gradient.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( gradient.Sizes() );
      DIP_END_STACK_TRACE
   }

   bool interpolate;
   DIP_STACK_TRACE_THIS( interpolate = BooleanFromString( mode, S::INTERPOLATE, S::ROUND ));

   if( nDims == 1 ) {
      PixelSize ps = gradmag.HasPixelSize() ? gradmag.PixelSize() : gradient.PixelSize();
      out.ReForge( gradmag );
      out.SetPixelSize( ps );
      DIP_OVL_CALL_FLOAT( NonMaximumSuppression1D, ( gradmag, mask, out ), ovlType );
      return;
   }

   std::unique_ptr< Framework::ScanLineFilter > scanLineFilter;
   // Note that because all input images are of the types specified, and we don't request any expansion,
   // the line input buffers for the line filter will point directly to the images, we can access neighboring
   // pixels. Be careful when doing things like this!
   if( nDims == 2 ) {
      // The 2D version does interpolation, and if !interpolate, it's ~15% faster than the generic nD version.
      DIP_OVL_NEW_FLOAT( scanLineFilter, NonMaximumSuppression2D, ( gradmag.Sizes(), gradmag.Strides(), interpolate ), ovlType );
   } else {
      DIP_OVL_NEW_FLOAT( scanLineFilter, NonMaximumSuppressionND, ( gradmag.Sizes(), gradmag.Strides() ), ovlType );
   }

   ImageConstRefArray inar{ gradmag, gradient };
   DataTypeArray indt{ ovlType, ovlType };
   if( mask.IsForged() ) {
      inar.emplace_back( mask );
      indt.push_back( DT_BIN );
   }
   ImageRefArray outar{ out };
   Framework::Scan( inar, outar, indt, { ovlType }, { ovlType }, { 1 }, *scanLineFilter, Framework::ScanOption::NeedCoordinates );
}

// ---

namespace {

template< typename TPI >
void MoveToLocalMinimumInternal(
      Image const& bin,       // binary
      Image const& weights,   // TPI
      Image& out,             // binary
      NeighborList const& neighbors,
      IntegerArray const& weights_offsets,
      IntegerArray const& out_offsets,
      dip::uint procDim
) {
   JointImageIterator< dip::bin, TPI, dip::bin > it( { bin, weights, out }, procDim );
   BooleanArray use( neighbors.Size(), true );
   dip::uint lastPixel = bin.Size( procDim ) - 1;
   do {
      auto binIt = it.template GetLineIterator< 0 >();
      auto weightIt = it.template GetLineIterator< 1 >();
      auto outIt = it.template GetLineIterator< 2 >();
      UnsignedArray coords = it.Coordinates();
      if( it.IsOnEdge() ) {
         // This is for lines that go along the image edge
         coords[ procDim ] = 1;
         auto neigh = neighbors.begin();
         for( dip::uint jj = 0; jj < neighbors.Size(); ++jj, ++neigh ) {
            use[ jj ] = neigh.IsInImage( coords, bin.Sizes() );
         }
         // First pixel on line
         if( *binIt ) {
            coords[ procDim ] = 0;
            neigh = neighbors.begin();
            TPI weight = *weightIt;
            dip::sint offset = 0;
            for( dip::uint jj = 0; jj < neighbors.Size(); ++jj, ++neigh ) {
               if( use[ jj ] && neigh.IsInImage( coords, bin.Sizes()) ) {
                  TPI value = *( weightIt.Pointer() + weights_offsets[ jj ] );
                  if( value < weight ) {
                     weight = value;
                     offset = out_offsets[ jj ];
                  }
               }
            }
            *( outIt.Pointer() + offset ) = true;
         }
         ++binIt;
         ++weightIt;
         ++outIt;
         for( dip::uint ii = 1; ii < lastPixel; ++ii ) {
            // Bulk pixels on line
            if( *binIt ) {
               TPI weight = *weightIt;
               dip::sint offset = 0;
               for( dip::uint jj = 0; jj < neighbors.Size(); ++jj ) {
                  if( use[ jj ] ) {
                     TPI value = *( weightIt.Pointer() + weights_offsets[ jj ] );
                     if( value < weight ) {
                        weight = value;
                        offset = out_offsets[ jj ];
                     }
                  }
               }
               *( outIt.Pointer() + offset ) = true;
            }
            ++binIt;
            ++weightIt;
            ++outIt;
         }
         // Last pixel on line
         if( *binIt ) {
            coords[ procDim ] = lastPixel;
            neigh = neighbors.begin();
            TPI weight = *weightIt;
            dip::sint offset = 0;
            for( dip::uint jj = 0; jj < neighbors.Size(); ++jj, ++neigh ) {
               if( use[ jj ] && neigh.IsInImage( coords, bin.Sizes()) ) {
                  TPI value = *( weightIt.Pointer() + weights_offsets[ jj ] );
                  if( value < weight ) {
                     weight = value;
                     offset = out_offsets[ jj ];
                  }
               }
            }
            *( outIt.Pointer() + offset ) = true;
         }
      } else {
         // This is for lines that do not go along the image edge
         // First pixel on line
         if( *binIt ) {
            coords[ procDim ] = 0;
            auto neigh = neighbors.begin();
            TPI weight = *weightIt;
            dip::sint offset = 0;
            for( dip::uint jj = 0; jj < neighbors.Size(); ++jj, ++neigh ) {
               if( neigh.IsInImage( coords, bin.Sizes()) ) {
                  TPI value = *( weightIt.Pointer() + weights_offsets[ jj ] );
                  if( value < weight ) {
                     weight = value;
                     offset = out_offsets[ jj ];
                  }
               }
            }
            *( outIt.Pointer() + offset ) = true;
         }
         ++binIt;
         ++weightIt;
         ++outIt;
         for( dip::uint ii = 1; ii < lastPixel; ++ii ) {
            // Bulk pixels on line
            if( *binIt ) {
               TPI weight = *weightIt;
               dip::sint offset = 0;
               for( dip::uint jj = 0; jj < neighbors.Size(); ++jj ) {
                  TPI value = *( weightIt.Pointer() + weights_offsets[ jj ] );
                  if( value < weight ) {
                     weight = value;
                     offset = out_offsets[ jj ];
                  }
               }
               *( outIt.Pointer() + offset ) = true;
            }
            ++binIt;
            ++weightIt;
            ++outIt;
         }
         // Last pixel on line
         if( *binIt ) {
            coords[ procDim ] = lastPixel;
            auto neigh = neighbors.begin();
            TPI weight = *weightIt;
            dip::sint offset = 0;
            for( dip::uint jj = 0; jj < neighbors.Size(); ++jj, ++neigh ) {
               if( neigh.IsInImage( coords, bin.Sizes()) ) {
                  TPI value = *( weightIt.Pointer() + weights_offsets[ jj ] );
                  if( value < weight ) {
                     weight = value;
                     offset = out_offsets[ jj ];
                  }
               }
            }
            *( outIt.Pointer() + offset ) = true;
         }
      }
   } while( ++it );
}

} // namespace

void MoveToLocalMinimum(
      Image const& c_bin,
      Image const& weights,
      Image& out
) {
   DIP_THROW_IF( !c_bin.IsForged() || !weights.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_bin.IsScalar() || !weights.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_bin.DataType().IsBinary() || !weights.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( c_bin.Sizes() != weights.Sizes(), E::SIZES_DONT_MATCH );
   Image bin = c_bin;
   if( out.SharesData( weights ) || out.Aliases( bin )) {
      out.Strip(); // Don't work in-place
   }
   out.ReForge( bin );
   out.Fill( 0 );
   dip::uint procDim = Framework::OptimalProcessingDim( bin ); // pick the best dimension for any of the 3 images, it does not matter which I think.
   NeighborList neighbors( { Metric::TypeCode::CONNECTED, 0 }, bin.Dimensionality() );
   IntegerArray weightsOffsets = neighbors.ComputeOffsets( weights.Strides() );
   IntegerArray outOffsets = neighbors.ComputeOffsets( out.Strides() );
   DIP_OVL_CALL_REAL( MoveToLocalMinimumInternal, ( bin, weights, out, neighbors, weightsOffsets, outOffsets, procDim ), weights.DataType() );
}

} // namespace dip
