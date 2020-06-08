/*
 * DIPlib 3.0
 * This file contains functions to apply attenuation correction
 *
 * (c)2018, Cris Luengo.
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
#include "diplib/microscopy.h"
#include "diplib/generation.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/transform.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/multithreading.h"

namespace dip {

// --- ExponentialFitCorrection ---

namespace {

struct ExponentialParams {
   dfloat a;
   dfloat b;
};

ExponentialParams ExponentialFit(
      dfloat* y,
      dfloat const* var,
      dip::uint start,
      dip::uint stop
) {
   // Determine weights
   std::vector< dfloat > weights( stop, 1 );
   if( var ) {
      for( dip::uint ii = start; ii < stop; ++ii ) {
         dfloat v = std::max( var[ ii ], 1e-3 );
         weights[ ii ] = 1.0 / ( v * v );
      }
   }
   // Convert y to ln(y)
   for( dip::uint ii = start; ii < stop; ++ii ) {
      if( y[ ii ] > 0 ) {
         y[ ii ] = std::log( y[ ii ] );
      } else {
         y[ ii ] = 0;
         weights[ ii ] = 0; // Ignore this one
      }
   }
   dfloat sum_w = 0.0;
   dfloat sum_x = 0.0;
   dfloat sum_y = 0.0;
   for( dip::uint ii = start; ii < stop; ++ii ) {
      sum_w += weights[ ii ];
      sum_x += static_cast< dfloat >( ii ) * weights[ ii ];
      sum_y += y[ ii ] * weights[ ii ];
   }
   dfloat mean_x = sum_w == 0 ? 0 : sum_x / sum_w;
   dfloat sum_t2 = 0.0;
   ExponentialParams out{ 0.0, 0.0 };
   for( dip::uint ii = start; ii < stop; ++ii ) {
      dfloat w = std::sqrt( weights[ ii ] );
      dfloat t = ( static_cast< dfloat >( ii ) - mean_x ) * w;
      sum_t2 += t * t;
      out.b += t * y[ ii ] * w;
   }
   out.b = sum_t2 == 0 ? 0 : ( out.b / sum_t2 );
   out.a = sum_w == 0 ? 0 : (( sum_y - sum_x * ( out.b )) / sum_w );
   return out;
}

} // namespace

void ExponentialFitCorrection(
      Image const& in,
      Image const& mask,
      Image& out,
      dfloat percentile,
      String const& fromWhere,
      dfloat hysteresis,
      String const& weighting
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   if( in.Dimensionality() < 3 ) {
      Copy( in, out );
      return;
   }
   DIP_THROW_IF( in.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Compute mean or percentile projections
   Image fitData( { 1, 1, in.Size( 2 ) }, 1, DT_DFLOAT ); // Allocated to make sure `Mean` copies data, even if `in` has the same size and data type.
   fitData.Protect();
   Image fitVariance;
   if(( percentile < 0 ) || ( percentile > 100 )) {
      DIP_STACK_TRACE_THIS( Mean( in, mask, fitData, "", { true, true, false } ));
      if(( weighting == "variance" ) && ( in.Size( 0 ) * in.Size( 1 ) > 1 )) { // Need at least 2 values to compute variance
         fitVariance.SetDataType( DT_DFLOAT );
         fitVariance.Protect();
         DIP_STACK_TRACE_THIS( StandardDeviation( in, mask, fitVariance, S::FAST, { true, true, false } ));
      } else if( weighting != "none" ) {
         DIP_THROW_INVALID_FLAG( weighting );
      }
   } else {
      DIP_STACK_TRACE_THIS( Percentile( in, mask, fitData, percentile, { true, true, false } ));
   }

   DIP_ASSERT( fitData.DataType() == DT_DFLOAT );
   DIP_ASSERT( !fitVariance.IsForged() || ( fitVariance.DataType() == DT_DFLOAT ));
   DIP_ASSERT( fitData.Stride( 2 ) == 1 );
   DIP_ASSERT( !fitVariance.IsForged() || fitVariance.Stride( 2 ) == 1 );

   // Find start point
   dip::uint depth = in.Size( 2 );
   dip::uint corStart = 0;
   dfloat* fit = static_cast< dfloat* >( fitData.Origin() );
   while(( corStart < depth ) && ( fit[ corStart ] == 0 )) {
      ++corStart;
   }
   if( fromWhere == "first plane" ) {
      // OK.
   } else if( fromWhere == "global max" ) {
      dfloat max = fit[ corStart ];
      for( dip::uint zz = corStart + 1; zz < depth; ++zz ) {
         if( fit[ zz ] > max ) {
            max = fit[ zz ];
            corStart = zz;
         }
      }
   } else if( fromWhere == "first max" ) {
      dip::uint zz = corStart;
      while(( fit[ zz + 1 ] > ( hysteresis * fit[ zz ] )) && ( zz < ( depth - 1 ))) {
         ++zz;
      }
      if( zz != ( depth - 1 )) {
         corStart = zz;
      }
   } else {
      DIP_THROW_INVALID_FLAG( fromWhere );
   }

   // Do the fit
   auto fitParams = ExponentialFit(
         fit, // Modified!!!
         fitVariance.IsForged() ?  static_cast< dfloat* >( fitVariance.Origin() ) + corStart : nullptr,
         corStart, depth);
   //std::cout << "a = " << fitParams.a << ", b = " << fitParams.b << '\n';

   // Calculate the correction values
   for( dip::uint zz = 0; zz < corStart; ++zz ) {
      fit[ zz ] = 1.0;
   }
   dfloat first = std::exp( fitParams.b * static_cast< dfloat >( corStart ) + fitParams.a );
   for( dip::uint zz = corStart; zz < depth; ++zz ) {
      fit[ zz ] = first / std::exp( fitParams.b * static_cast< dfloat >( zz ) + fitParams.a );
   }

   // Correct the data
   MultiplySampleWise( in, fitData, out, DataType::SuggestFlex( in.DataType() ));
}

// --- AttenuationCorrection ---

namespace {

constexpr dfloat attenuationNormalization = 100.0;

void NormalizedCosineKernel(
      Image& im,
      dfloat theta,
      dfloat ratio,
      bool cosSquare
) {
   DIP_ASSERT( im.HasNormalStrides() );
   DIP_ASSERT( im.DataType() == DT_SFLOAT );

   // Constants
   dfloat rr = ratio * std::tan( theta );
   dip::sint r = round_cast( std::abs( rr ));
   ratio *= ratio;      // ratio to the second power
   rr *= rr;
   dip::sint width = static_cast< dip::sint >( im.Size( 0 ));
   dip::sint height = static_cast< dip::sint >( im.Size( 1 ));
   dip::sint center = ( width / 2 ) + ( height / 2 ) * width;

   // Initialize image to zero
   im.Fill( 0 );

   // Get data pointer to the image data
   sfloat* data = static_cast< sfloat* >( im.Origin() );

   // Fill the center circle of the image
   dfloat sum = 0.0;
   for( dip::sint yy = -r; yy <= r; ++yy ) {
      for( dip::sint xx = -r; xx <= r; ++xx ) {
         dfloat value = static_cast< dfloat >( xx * xx + yy * yy );
         if( value <= rr ) {
            value = ratio / ( value + ratio );
            sum += value;
            data[ center + xx + yy * width ] = static_cast< sfloat >( cosSquare ? value : std::sqrt( value ));
         }
      }
   }

   // Normalize data
   im /= sum;
}

void LightDistribution(
      Image& light,
      Image const& slice,
      sfloat threshold,
      sfloat attenuation,
      sfloat background
) {
   JointImageIterator< sfloat, sfloat > it( { slice, light } );
   do {
      it.Out() -= ( it.In() > threshold ) ? attenuation * it.In() : background;
   } while( ++it );
}

void RecursiveAttenuationCorrectionLT1(
      Image const& in,
      Image& out,
      dfloat attenuation,
      dfloat background,
      dfloat threshold,
      dfloat theta,
      dfloat ratio
) {
   // Normalize input values
   attenuation *= ( ratio / attenuationNormalization );
   background *= ( ratio / attenuationNormalization );

   // Make sure output is separated from input
   Image inc = in.QuickCopy();
   if( out.Aliases( inc )) {
      out.Strip();
   }
   out.ReForge( inc, DT_SFLOAT );

   // Initialize slices
   Image inSlice = inc.At( RangeArray{ Range{}, Range{}, Range{ 0 }} );
   dip::sint inZOffset = inc.Stride( 2 );
   Image outSlice = out.At( RangeArray{ Range{}, Range{}, Range{ 0 }} );
   outSlice.Protect();
   dip::sint outZOffset = out.Stride( 2 );

   // Copy the first layer: no attenuation
   outSlice.Copy( inSlice );

   // Initialize kernel
   Image kernel( inSlice.Sizes(), 1, DT_SFLOAT );
   DIP_STACK_TRACE_THIS( NormalizedCosineKernel( kernel, theta, ratio, true ));
   DIP_STACK_TRACE_THIS( FourierTransform( kernel, kernel ));

   // Initialize light
   Image light( inSlice.Sizes(), 1, DT_SFLOAT );
   light.Protect();
   light.Fill( 1.0 );

   // Create memory for prevSlice
   Image prevSlice( inSlice.Sizes(), 1, DT_SFLOAT ); // uninitialized
   prevSlice.Protect();

   Image f;
   for( dip::uint zz = 1; zz < inc.Size( 2 ); ++zz ) {
      // Copy inSlice to prevSlice and get new inSlice
      prevSlice.Copy( inSlice );
      inSlice.ShiftOriginUnsafe( inZOffset );
      outSlice.ShiftOriginUnsafe( outZOffset );

      // Update light distribution
      DIP_STACK_TRACE_THIS( LightDistribution( light, prevSlice, static_cast< sfloat >( threshold ),
            static_cast< sfloat >( attenuation ), static_cast< sfloat >( background )));

      // Convolve light distribution with cone kernel
      DIP_START_STACK_TRACE
         FourierTransform( light, f );
         f *= kernel;
         FourierTransform( f, light, { S::INVERSE, S::REAL } );
      DIP_END_STACK_TRACE

      // Correct inSlice and put result in outSlice
      DIP_STACK_TRACE_THIS( Divide( inSlice, light, outSlice ));
   }
}

void LightCorrection (
            Image const& light,
            Image const& slice,
            Image& out,
            sfloat threshold,
            sfloat attenuation,
            sfloat background
) {
   JointImageIterator< sfloat, sfloat, sfloat > it( { slice, out, light } );
   do {
      it.Out() = (( it.In() > threshold ) ? attenuation * it.In() : background ) * it.Sample< 2 >();
   } while( ++it );
}

void RecursiveAttenuationCorrectionLT2(
      Image const& in,
      Image& out,
      dfloat attenuation,
      dfloat background,
      dfloat threshold,
      dfloat theta,
      dfloat ratio
) {
   // Normalize input values
   attenuation *= ( ratio / attenuationNormalization );
   background *= ( ratio / attenuationNormalization );

   // Make sure output is separated from input
   Image inc = in.QuickCopy();
   if( out.Aliases( inc )) {
      out.Strip();
   }
   out.ReForge( inc, DT_SFLOAT );

   // Initialize slices
   Image inSlice = inc.At( RangeArray{ Range{}, Range{}, Range{ 0 }} );
   dip::sint inZOffset = inc.Stride( 2 );
   Image outSlice = out.At( RangeArray{ Range{}, Range{}, Range{ 0 }} );
   outSlice.Protect();
   dip::sint outZOffset = out.Stride( 2 );

   // Copy the first layer: no attenuation
   outSlice.Copy( inSlice );

   // Initialize kernels
   Image cKernel( inSlice.Sizes(), 1, DT_SFLOAT );
   DIP_STACK_TRACE_THIS( NormalizedCosineKernel( cKernel, theta, ratio, false ));
   DIP_STACK_TRACE_THIS( FourierTransform( cKernel, cKernel ));
   Image csqKernel( inSlice.Sizes(), 1, DT_SFLOAT );
   DIP_STACK_TRACE_THIS( NormalizedCosineKernel( csqKernel, theta, ratio, true ));
   DIP_STACK_TRACE_THIS( FourierTransform( csqKernel, csqKernel ));

   // Initialize light
   Image light( inSlice.Sizes(), 1, DT_SFLOAT );
   light.Protect();
   light.Fill( 1.0 );

   // Initialize correction
   Image correction( inSlice.Sizes(), 1, DT_SFLOAT );
   correction.Protect();

   // Create memory for prevSlice
   Image prevSlice( inSlice.Sizes(), 1, DT_SFLOAT ); // uninitialized
   prevSlice.Protect();

   Image f;
   for( dip::uint zz = 1; zz < inc.Size( 2 ); ++zz ) {
      // Copy inSlice to prevSlice and get new inSlice
      prevSlice.Copy( inSlice );
      inSlice.ShiftOriginUnsafe( inZOffset );
      outSlice.ShiftOriginUnsafe( outZOffset );

      // Update light distribution
      DIP_STACK_TRACE_THIS( LightCorrection( light, prevSlice, correction, static_cast< sfloat >( threshold ),
            static_cast< sfloat >( attenuation ), static_cast< sfloat >( background )));

      // Convolve light distribution with cone kernel
      DIP_START_STACK_TRACE
         FourierTransform( light, f );
         f *= csqKernel;
         FourierTransform( f, light, { S::INVERSE, S::REAL } );
         FourierTransform( correction, f );
         f *= cKernel;
         FourierTransform( f, correction, { S::INVERSE, S::REAL } );
      DIP_END_STACK_TRACE

      // Correct inSlice and put result in outSlice
      DIP_STACK_TRACE_THIS( Subtract( light, correction, light ));
      DIP_STACK_TRACE_THIS( Divide( inSlice, light, outSlice ));
   }
}

// Allocating all lookup tables:
//  cosine = cos term for every voxel in the light cone
//  cossq  = cos term squared
//  pos    = relative position from the center of the cone in the layer
//  fAtten = the forward attenuation weight for contributing voxel
//  bAtten = the backward att. weight.
void AbsCorAllocateTables(
      FloatArray& cosine,
      FloatArray& cossq,
      FloatArray& fAtten,
      FloatArray& bAtten,
      dfloat& ncos,
      dfloat& ncossq,
      FloatArray& attBuff1,
      FloatArray& attBuff2,
      FloatArray& attBuff3,
      IntegerArray& pos,
      dip::sint width,
      dip::sint height,
      dip::sint rad,
      dfloat apertureAngle,
      dfloat ratio,
      dfloat fa,
      dfloat ba,
      dip::sint& coneSize
) {
   dip::sint widthp = width + 1;
   dip::sint widthm = width - 1;

   dip::sint size = 2 * rad + 1;
   dip::uint arraySize = static_cast< dip::uint >( size * size );
   ncos = 1.0;
   ncossq = 1.0;

   // Allocate arrays
   cosine.resize( arraySize, 1.0 );
   cossq.resize( arraySize, 1.0 );
   fAtten.resize( arraySize, -fa );
   bAtten.resize( arraySize, -ba );
   pos.resize( arraySize, 0 );

   dip::uint ii = 1;
   for( dip::sint x = 1; x <= rad; ++x ) {
      dfloat theta = std::atan( std::sqrt( x * x ) / ratio );
      if( theta <= apertureAngle ) {
         dfloat cosn = std::cos( theta );
         dfloat cosnsq = ( cosn * cosn );
         dfloat la = -fa / cosn;
         dfloat lb = -ba / cosn;

         pos[ ii ] = x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         pos[ ii ] = -x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         pos[ ii ] = width * x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         pos[ ii ] = -width * x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         ncos += ( 4 * cosn );
         ncossq += ( 4 * cosnsq );
      }
      theta = std::atan( std::sqrt( 2.0 * static_cast< dfloat >( x * x )) / ratio );
      if( theta <= apertureAngle ) {
         dfloat cosn = std::cos( theta );
         dfloat cosnsq = ( cosn * cosn );
         dfloat la = -fa / cosn;
         dfloat lb = -ba / cosn;

         pos[ ii ] = x * widthp;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         pos[ ii ] = -widthp * x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         pos[ ii ] = widthm * x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         pos[ ii ] = -widthm * x;
         cosine[ ii ] = cosn;
         cossq[ ii ] = cosnsq;
         fAtten[ ii ] = la;
         bAtten[ ii++ ] = lb;

         ncos += ( 4 * cosn );
         ncossq += ( 4 * cosnsq );
      }
   }

   for( dip::sint y = 1; y <= rad; ++y ) {
      for( dip::sint x = y + 1; x <= rad; ++x ) {
         dfloat theta = std::atan( std::sqrt( x * x + y * y ) / ratio );
         if( theta <= apertureAngle ) {
            dfloat cosn = std::cos( theta );
            dfloat cosnsq = ( cosn * cosn );
            dfloat la = -fa / cosn;
            dfloat lb = -ba / cosn;
            dip::sint wx = width * x;
            dip::sint wy = width * y;

            pos[ ii ] = x + wy;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = -x + wy;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = x - wy;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = -x - wy;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = y + wx;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = -y + wx;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = y - wx;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            pos[ ii ] = -y - wx;
            cosine[ ii ] = cosn;
            cossq[ ii ] = cosnsq;
            fAtten[ ii ] = la;
            bAtten[ ii++ ] = lb;

            ncos += ( 8 * cosn );
            ncossq += ( 8 * cosnsq );
         }
      }
   }

   coneSize = static_cast< dip::sint >( ii );
   while( ii-- > 0 ) {
      pos[ ii ] = pos[ ii ] * coneSize + static_cast< dip::sint >( ii );
   }

   dip::uint totalSize = static_cast< dip::uint >( width * height * coneSize );
   attBuff1.resize( totalSize, 0 );
   attBuff2.resize( totalSize, 0 );
   attBuff3.resize( totalSize, 0 );
}

void RecursiveAttenuationCorrectionDET(
      Image const& in,
      Image& out,
      dfloat fAttenuation,
      dfloat bAttenuation,
      dfloat theta,
      dfloat ratio
) {
   // Normalize input values
   fAttenuation *= ( ratio / attenuationNormalization );
   bAttenuation *= ( ratio / attenuationNormalization );

   // We create an output image of type SFLOAT, which we'll later copy over to `c_out`, unless `c_out` is all we need
   Image inc = in.QuickCopy(); // prevent losing this if &in==&out
   DIP_STACK_TRACE_THIS( out.ReForge( inc.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   Image sepOut;
   if(( out.DataType() != DT_SFLOAT ) || ( !out.HasNormalStrides() )) {
      DIP_STACK_TRACE_THIS( sepOut.ReForge( inc.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DONT_ALLOW ));
      DIP_ASSERT( sepOut.DataType() == DT_SFLOAT );
      DIP_ASSERT( sepOut.HasNormalStrides() );
   } else {
      sepOut = out.QuickCopy();
   }
   sepOut.Protect();
   sepOut.Copy( inc );

   // Get pointer to the data of out
   sfloat* pOut = static_cast< sfloat* >( sepOut.Origin() );

   // Allocate tables
   // Values below are all non-negative, but we use signed integers because the original code did, and there
   // are many comparisons >= 0.
   dip::sint width = static_cast< dip::sint >( sepOut.Size( 0 ));
   dip::sint height = static_cast< dip::sint >( sepOut.Size( 1 ));
   dip::sint depth = static_cast< dip::sint >( sepOut.Size( 2 ));
   dip::sint layer = width * height;
   //dip::sint size = layer * depth;
   dip::sint rad = round_cast( ratio * std::tan( theta )); // guaranteed positive...
   dip::sint trad = rad * 2;
   dip::sint attWidth = width + trad;
   dip::sint attHeight = height + trad;

   FloatArray	cosine_a, cossq_a, fAtten_a, bAtten_a;
   dfloat ncossq, ncos;
   FloatArray attBuff1, attBuff2, attBuff3;
   IntegerArray pos_a;
   dip::sint coneSize;
   AbsCorAllocateTables( cosine_a, cossq_a, fAtten_a, bAtten_a, ncos, ncossq, attBuff1, attBuff2, attBuff3, pos_a,
         attWidth, attHeight, rad, theta, ratio, fAttenuation, bAttenuation, coneSize );
   // The following lines added to avoid compiler warnings when indexing using a signed integer:
   dip::sint* pos = pos_a.data();
   dfloat* cosine = cosine_a.data();
   dfloat* cossq = cossq_a.data();
   dfloat* fAtten = fAtten_a.data();
   dfloat* bAtten = bAtten_a.data();

   dip::sint tradcone = trad * coneSize;
   dip::sint layerOffset = ( rad * attWidth + rad ) * coneSize;
   dfloat* cumfa = attBuff1.data() + layerOffset;
   dfloat* cumba = attBuff2.data() + layerOffset;
   dfloat* cumc = attBuff3.data() + layerOffset;

   sfloat* imp = pOut;
   sfloat* p = imp;
   dfloat* af = cumfa;
   dfloat* ab = cumba;
   dip::sint lines = height;
   dip::sint index = 0;

   // First layer is unattenuated and directly copied to the output
   // Then the accumulation terms are initialized with the first layer
   while( --lines >= 0 ) {
      dip::sint voxels = width;
      while( --voxels >= 0 ) {
         dip::sint cone = coneSize;
         while( --cone >= 0 ) {
            af[ index + cone ] = ( *p ) * fAtten[ cone ];
            ab[ index + cone ] = ( *p ) * bAtten[ cone ];
         }
         ++p;
         index += coneSize;
      }
      index += tradcone;
   }

   // Main loop
   dip::sint d = depth - 1;
   imp = pOut + layer;
   while( --d > 0 ) {
      p = imp;
      af = cumfa;
      ab = cumba;
      index = 0;
      lines = height;
      while( --lines >= 0 ) {
         dip::sint voxels = width;
         while( --voxels >= 0 ) {
            dip::sint cone = coneSize;
            dfloat gForward = 0.0;
            dfloat gBackward = 0.0;
            while( --cone >= 0 ) {
               gForward += cossq[ cone ] * std::exp( af[ index + pos[ cone ]] );
               gBackward += cosine[ cone ] * std::exp( ab[ index + pos[ cone ]] );
            }
            gForward /= ncossq;
            gBackward /= ncos;
            *( p++ ) /= static_cast< sfloat >( gForward * gBackward );
            index += coneSize;
         }
         index += tradcone;
      }

      p = imp;
      af = cumfa;
      dfloat* at = cumc;
      index = 0;
      lines = height;
      while( --lines >= 0 ) {
         dip::sint voxels = width;
         while( --voxels >= 0 ) {
            dip::sint cone = coneSize;
            while( --cone >= 0 ) {
               at[ index + cone ] = af[ index + pos[ cone ]] + ( *p ) * fAtten[ cone ];
            }
            ++p;
            index += coneSize;
         }
         index += tradcone;
      }

      //dfloat* tmp = cumc;
      //cumc = cumfa;
      //cumfa = tmp;
      std::swap( cumc, cumfa );
      p = imp;
      ab = cumba;
      at = cumc;
      index = 0;
      lines = height;
      while( --lines >= 0 ) {
         dip::sint voxels = width;
         while( --voxels >= 0 ) {
            dip::sint cone = coneSize;
            while( --cone >= 0 ) {
               at[ index + cone ] = ab[ index + pos[ cone ]] + ( *p ) * bAtten[ cone ];
            }
            ++p;
            index += coneSize;
         }
         index += tradcone;
      }
      //tmp = cumc;
      //cumc = cumba;
      //cumba = tmp;
      std::swap( cumc, cumba );
      imp += layer;
   }

   p = imp;
   af = cumfa;
   ab = cumba;
   index = 0;
   lines = height;
   while( --lines >= 0 ) {
      dip::sint voxels = width;
      while( --voxels >= 0 ) {
         dip::sint cone = coneSize;
         dfloat gForward = 0.0;
         dfloat gBackward = 0.0;
         while( --cone >= 0 ) {
            gForward += cossq[ cone ] * std::exp( af[ index + pos[ cone ]] );
            gBackward += cosine[ cone ] * std::exp( ab[ index + pos[ cone ]] );
         }
         gForward /= ncossq;
         gBackward /= ncos;
         *( p++ ) /= static_cast< sfloat >( gForward * gBackward );
         index += coneSize;
      }
      index += tradcone;
   }

   // Copy data over to `out` if `out` wasn't DT_SFLOAT
   if( sepOut.Origin() != out.Origin() ) {
      out.Copy( sepOut );
   }
}

} // namespace

void AttenuationCorrection(
      Image const& in,
      Image& out,
      dfloat fAttenuation,
      dfloat bAttenuation,
      dfloat background,
      dfloat threshold,
      dfloat NA,
      dfloat refIndex,
      String const& method
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   if( in.Dimensionality() < 3 ) {
      out = in;
      return;
   }
   DIP_THROW_IF( in.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Check pixel sizes and compute ratio
   DIP_THROW_IF( in.PixelSize( 0 ) != in.PixelSize( 1 ), "X and Y pixel sizes must be identical" );
   DIP_THROW_IF( in.PixelSize( 0 ).units != in.PixelSize( 2 ).units, "Z pixel size must have same units as X and Y pixel size" );
   dfloat ratio = in.PixelSize( 2 ).magnitude / in.PixelSize( 0 ).magnitude;

   // Compute theta
   dfloat theta = ( NA / refIndex ) >= 1.0 ? pi / 2.0 : std::asin( NA / refIndex );

   if( method == "LT1" ) {
      DIP_STACK_TRACE_THIS( RecursiveAttenuationCorrectionLT1( in, out, fAttenuation, background, threshold, theta, ratio ));
   } else if( method == "LT2" ) {
      DIP_STACK_TRACE_THIS( RecursiveAttenuationCorrectionLT2( in, out, fAttenuation, background, threshold, theta, ratio ));
   } else if( method == "DET" ) {
      DIP_STACK_TRACE_THIS( RecursiveAttenuationCorrectionDET( in, out, fAttenuation, bAttenuation, theta, ratio ));
   } else {
      DIP_THROW_INVALID_FLAG( method );
   }
   out.CopyNonDataProperties( in );
}

// --- AttenuationCorrection ---

namespace {

using AttSimArray_pt1 = std::vector< std::array< sfloat, 8 >>;
using AttSimArray_pt2 = std::vector< AttSimArray_pt1 >;
using AttSimArray = std::vector< AttSimArray_pt2 >;

AttSimArray AttSimInitInterpol(
      dip::uint oversample,
      dip::uint zxratio
) {
   dip::uint zoversample = zxratio * oversample;
   AttSimArray array( oversample + 1, AttSimArray_pt2( oversample + 1, AttSimArray_pt1( zoversample + 1 )));
   sfloat norm = static_cast< sfloat >( oversample * oversample * zoversample );
   for( dip::uint a = 0; a <= oversample; ++a ) {
      for( dip::uint b = 0; b <= oversample; ++b ) {
         for( dip::uint c = 0; c <= zoversample; ++c ) {
            array[ a ][ b ][ c ] =
                  {{
                         static_cast< sfloat >(( oversample - a ) * ( oversample - b ) * ( zoversample - c )) / norm,
                         static_cast< sfloat >( a * ( oversample - b ) * ( zoversample - c )) / norm,
                         static_cast< sfloat >(( oversample - a ) * b * ( zoversample - c )) / norm,
                         static_cast< sfloat >( a * b * ( zoversample - c )) / norm,
                         static_cast< sfloat >(( oversample - a ) * ( oversample - b ) * c ) / norm,
                         static_cast< sfloat >( a * ( oversample - b ) * c ) / norm,
                         static_cast< sfloat >(( oversample - a ) * b * c ) / norm,
                         static_cast< sfloat >( a * b * c ) / norm,
                   }};
         }
      }
   }
   return array;
}

struct AttSimTrace {
   dip::sint MaxSub2Int;
   dip::sint ZMaxSub2Int;
   dfloat MaxSub2Double;
   dfloat ZMaxSub2Double;
   dfloat RayStep;
   AttSimTrace( dip::uint oversample, dfloat rayStep, dip::uint intzxratio )
         : MaxSub2Int( static_cast< dip::sint >( 2 * oversample )),
           ZMaxSub2Int( static_cast< dip::sint >( 2 * oversample * intzxratio )),
           MaxSub2Double( static_cast< dfloat >( MaxSub2Int )),
           ZMaxSub2Double( static_cast< dfloat >( ZMaxSub2Int )),
           RayStep( rayStep ) {}
};

std::tuple< dip::sint, sfloat, sfloat > AttSimDrawLightCone(
      FloatArray& cosine,
      dfloat zxratio,
      dfloat theta,
      dip::sint z
) {
   zxratio *= static_cast< dfloat >( z );
   dip::sint rad = static_cast< dip::sint >( zxratio * std::tan( theta ));
   zxratio *= zxratio;     // ratio to the second power
   dip::sint width = 2 * rad + 1;
   dfloat ncos = 0;
   dfloat ncossq = 0;
   for( dip::sint y = -rad; y <= rad; ++y ) {
      for( dip::sint x = -rad; x <= rad; ++x ) {
         dfloat value = 0;
         dip::sint dist2 = ( x * x + y * y );
         if( dist2 <= rad * rad ) {
            value = zxratio / ( static_cast< dfloat >( dist2 ) + zxratio );
            ncossq += value;
            value = std::sqrt( value );
            ncos += value;
         }
         cosine[ static_cast< dip::uint >(( x + rad ) + ( y + rad ) * width ) ] = value;
      }
   }
   return std::make_tuple( rad, static_cast< sfloat >( ncos ), static_cast< sfloat >( ncossq ));
}

struct Vector {
   dfloat x;
   dfloat y;
   dfloat z;
};

dfloat VecDotproduct(
      Vector const& p1,
      Vector const& p2
) {
   return p1.x * p2.x + p1.y * p2.y + p1.z * p2.z;
}

Vector VecSub(
      Vector const& p1,
      Vector const& p2
) {
   return { p1.x - p2.x, p1.y - p2.y, p1.z - p2.z };
}

Vector VecFactor(
      Vector const& p,
      dfloat factor
) {
   return { p.x * factor, p.y * factor, p.z * factor };
}

dfloat AttSimArbTrace(
      Vector const& vec0,           // end vectors
      Vector const& vec1,           // end vectors
      sfloat const* dataBuf,        // pointer to 3D original buffer
      dip::sint width,              // moduli of a line and a plane
      dip::sint height,             // moduli of a line and a plane
      IntegerArray const& stride,
      AttSimArray const& interArr,  // interpolation array
      dfloat zAspectRatio,
      std::array< dip::sint, 8 > const& pixAdd, // Offsets for neighbours
      AttSimTrace const& trace
) {
   constexpr bool interplate = true; // set to false to use nearest neighbor instead of interpolating

   // Calculate trace increment vector
   Vector CurrVec = vec0;
   Vector DispVec = VecSub( vec1, vec0 );
   dfloat Length = std::floor( std::sqrt( VecDotproduct( DispVec, DispVec )));
   dip::uint nPixels = static_cast< dip::uint >( Length / trace.RayStep );
   if( nPixels > 1 ) {
      DispVec = VecFactor( DispVec, trace.RayStep / Length );
   }
   DispVec.z /= zAspectRatio;

   while((
           ( CurrVec.x < 0 )
        || ( CurrVec.y < 0 )
        || ( round_cast( CurrVec.x ) >= width )
        || ( round_cast( CurrVec.y ) >= height )
        ) && ( nPixels > 0 )) {
      CurrVec.x += DispVec.x;
      CurrVec.y += DispVec.y;
      CurrVec.z += DispVec.z;
      --nPixels;
   }

   dfloat exponent = 0.0;
   if( interplate ) {
      while( nPixels-- > 0 ) {
         dip::sint xrest = round_cast( CurrVec.x * trace.MaxSub2Double );
         dip::sint xx = xrest / trace.MaxSub2Int;
         xrest = (( xrest - xx * trace.MaxSub2Int ) + 1 ) / 2;
         dip::sint yrest = round_cast( CurrVec.y * trace.MaxSub2Double );
         dip::sint yy = yrest / trace.MaxSub2Int;
         yrest = (( yrest - yy * trace.MaxSub2Int ) + 1 ) / 2;
         dip::sint zrest = round_cast( CurrVec.z * trace.ZMaxSub2Double );
         dip::sint zz = zrest / trace.ZMaxSub2Int;
         zrest = (( zrest - zz * trace.ZMaxSub2Int ) + 1 ) / 2;
         dip::sint PixBase0 = xx * stride[ 0 ] + yy * stride[ 1 ] + zz * stride[ 2 ];
         auto& subArr = interArr[ static_cast< dip::uint >( xrest ) ][ static_cast< dip::uint >( yrest ) ][ static_cast< dip::uint >( zrest ) ];
         for( dip::uint PixNr = 0; PixNr < 8; ++PixNr ) {
            dip::sint PixBase1 = PixBase0 + pixAdd[ PixNr ];
            exponent += static_cast< dfloat >( dataBuf[ PixBase1 ] ) * static_cast< dfloat >( subArr[ PixNr ] );
         }
         CurrVec.x += DispVec.x;
         CurrVec.y += DispVec.y;
         CurrVec.z += DispVec.z;
      }
   } else {    // nearest neighbour
      while( nPixels-- > 0 ) {
         dip::sint PixBase0 = round_cast( CurrVec.x ) * stride[ 0 ] +
                              round_cast( CurrVec.y ) * stride[ 1 ] +
                              round_cast( CurrVec.z ) * stride[ 2 ];
         exponent += dataBuf[ PixBase0 ];
         CurrVec.x += DispVec.x;
         CurrVec.y += DispVec.y;
         CurrVec.z += DispVec.z;
      }
   }
   return -exponent;
}

} // namespace

void SimulatedAttenuation(
      Image const& in,
      Image& out,
      dfloat fAttenuation,
      dfloat bAttenuation,
      dfloat NA,
      dfloat refIndex,
      dip::uint oversample,
      dfloat rayStep
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   if( in.Dimensionality() < 3 ) {
      out = in;
      return;
   }
   DIP_THROW_IF( in.Dimensionality() != 3, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Check pixel sizes and compute ratio
   DIP_THROW_IF( in.PixelSize( 0 ) != in.PixelSize( 1 ), "X and Y pixel sizes must be identical" );
   DIP_THROW_IF( in.PixelSize( 0 ).units != in.PixelSize( 2 ).units, "Z pixel size must have same units as X and Y pixel size" );
   dfloat zxratio = in.PixelSize( 2 ).magnitude / in.PixelSize( 0 ).magnitude;

   // Convert `in` to `sfloat`
   Image fIn;
   if( in.DataType() != DT_SFLOAT ) {
      Convert( in, fIn, DT_SFLOAT );
   } else {
      fIn = in.QuickCopy();
   }

   // Prepare `out`
   DIP_STACK_TRACE_THIS( out.ReForge( fIn.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW ));
   out.CopyNonDataProperties( in );
   Image fOut;
   if( out.DataType() != DT_SFLOAT ) {
      DIP_STACK_TRACE_THIS( fOut.ReForge( fIn.Sizes(), 1, DT_SFLOAT, Option::AcceptDataTypeChange::DONT_ALLOW ));
      DIP_ASSERT( fOut.DataType() == DT_SFLOAT );
   } else {
      fOut = out.QuickCopy();
   }

   // Copy first slice of `in` to `out`
   fOut.At( RangeArray{ Range{}, Range{}, Range{ 0 }} ).Copy( fIn.At( RangeArray{ Range{}, Range{}, Range{ 0 }} ));

   // Get data pointers
   sfloat* src = static_cast< sfloat* >( fIn.Origin() );
   sfloat* dst = static_cast< sfloat* >( fOut.Origin() );

   // Compute constants
   dfloat theta = ( NA / refIndex ) >= 1.0 ? pi / 2.0 : std::asin( NA / refIndex );
   //theta = theta / 180.0 * pi; // Conversion degrees to radian (??? Is this necessary? Is pi/2 not radian already?)
   dip::uint intzxratio = static_cast< dip::uint >( round_cast( zxratio ));
   fAttenuation *= ( rayStep / attenuationNormalization );
   bAttenuation *= ( rayStep / attenuationNormalization );

   std::array< dip::sint, 8 > pixAdd{{
         0,
         fIn.Stride( 0 ),
         fIn.Stride( 1 ),
         fIn.Stride( 1 ) + fIn.Stride( 0 ),
         fIn.Stride( 2 ),
         fIn.Stride( 2 ) + fIn.Stride( 0 ),
         fIn.Stride( 2 ) + fIn.Stride( 1 ),
         fIn.Stride( 2 ) + fIn.Stride( 1 ) + fIn.Stride( 0 ),
   }};

   AttSimArray interArr = AttSimInitInterpol( oversample, intzxratio );
   AttSimTrace trace( oversample, rayStep, intzxratio );

   dip::sint width = static_cast< dip::sint >( fIn.Size( 0 ));
   dip::sint height = static_cast< dip::sint >( fIn.Size( 1 ));
   dip::sint depth = static_cast< dip::sint >( fIn.Size( 2 ));
   dip::uint rad = 2 * static_cast< dip::uint >( zxratio * static_cast< dfloat >( depth - 1 ) * std::tan( theta )) + 1;

#ifdef _OPENMP
   dip::uint nThreads = std::min( GetNumberOfThreads(), fIn.Size( 2 ) /* ==depth */ );
#endif
   #pragma omp parallel num_threads( static_cast< int >( nThreads ))
   {
      FloatArray cosine( rad * rad );
      #pragma omp for schedule( dynamic, 1 )
      for( dip::sint z = 1; z < depth; ++z ) {
         dip::sint radius;
         sfloat norm_cos, norm_cossq;
         std::tie( radius, norm_cos, norm_cossq ) = AttSimDrawLightCone( cosine, zxratio, theta, z );
         for( dip::sint y = 0; y < height; ++y ) {
            for( dip::sint x = 0; x < width; ++x ) {
               dip::sint srcPos = x * fIn.Stride( 0 ) + y * fIn.Stride( 1 ) + z * fIn.Stride( 2 );
               dip::sint dstPos = x * fOut.Stride( 0 ) + y * fOut.Stride( 1 ) + z * fOut.Stride( 2 );
               if( src[ srcPos ] != 0.0 ) {
                  sfloat g1 = 0.0;
                  sfloat g2 = 0.0;
                  for( dip::sint yy = -radius; yy <= radius; ++yy ) {
                     for( dip::sint xx = -radius; xx <= radius; ++xx ) {
                        dfloat factor = cosine[ static_cast< dip::uint >( xx + radius + ( yy + radius ) * ( 2 * radius + 1 )) ];
                        if( factor != 0.0 ) {
                           Vector vecStart{
                                 static_cast< dfloat >( x + xx ),
                                 static_cast< dfloat >( y + yy ),
                                 0.0,
                           };
                           Vector vecEnd{
                                 static_cast< dfloat >( x ),
                                 static_cast< dfloat >( y ),
                                 static_cast< dfloat >( z ) * zxratio,
                           };
                           dfloat rayTotal = AttSimArbTrace( vecStart, vecEnd, src, width - 1, height - 1, fIn.Strides(), interArr, zxratio, pixAdd, trace );
                           g1 += static_cast< sfloat >( factor * factor * std::exp( fAttenuation * rayTotal ));
                           g2 += static_cast< sfloat >( factor * std::exp( bAttenuation * rayTotal ));
                        }
                     }
                  }
                  g1 /= norm_cossq;
                  g2 /= norm_cos;
                  dst[ dstPos ] = src[ srcPos ] * g1 * g2;
               } else {
                  dst[ dstPos ] = 0.0;
               }
            }
         }
      }
   }
   // Copy data over to `out` if `out` wasn't DT_SFLOAT
   if( fOut.Origin() != out.Origin() ) {
      out.Copy( fOut );
   }
}

} // namespace dip
