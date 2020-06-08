/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the basic morphological operators.
 *
 * (c)2017-2018, Cris Luengo.
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
#include "diplib/morphology.h"
#include "diplib/math.h"
#include "diplib/mapping.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

// Prevent issues with in-place operations -- returns a new `in` image to use, and potentially strips `out`.
// This function is useful in the filters below where we do lots of things like:
//    Filter( in, out )
//    in - out;
// There, Filter() would invalidate `in` if they happen to be the same object or share data, making the second
// line do the wrong thing.
// NOTE! Presumes a scalar image -- color space information is not copied.
Image Separate( Image const& in, Image& out ) {
   Image tmp = in.QuickCopy(); // prevent `in` being overwritten if it's the same image as `out`.
   tmp.SetPixelSize( in.PixelSize() );
   if( out.Aliases( in )) {
      out.Strip(); // prevent `in` data being overwritten if `out` points to the same region.
   }
   return tmp;
}

// "texture", "object", "both", "dynamic"=="both"
enum class EdgeType {
      TEXTURE,
      OBJECT,
      BOTH
};

EdgeType GetEdgeType( String const& edgeType ) {
   if( edgeType == S::TEXTURE ) {
      return EdgeType::TEXTURE;
   } else if( edgeType == S::OBJECT ) {
      return EdgeType::OBJECT;
   } else if(( edgeType == S::BOTH ) || ( edgeType == S::DYNAMIC )) {
      return EdgeType::BOTH;
   } else {
      DIP_THROW_INVALID_FLAG( edgeType );
   }
}

} // namespace


void Tophat(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      String const& edgeType,
      String const& polarity,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      bool white = BooleanFromString( polarity, S::WHITE, S::BLACK );
      EdgeType decodedEdgeType = GetEdgeType( edgeType );
      switch( decodedEdgeType ) {
         case EdgeType::BOTH:
            if( white ) {
               Image c_in = Separate( in, out );
               Erosion( c_in, out, se, boundaryCondition );
               Subtract( c_in, out, out, out.DataType() );
            } else {
               Image c_in = Separate( in, out );
               Dilation( c_in, out, se, boundaryCondition );
               out -= c_in;
            }
            break;
         default:
         case EdgeType::TEXTURE:
            if( white ) {
               Image c_in = Separate( in, out );
               Opening( c_in, out, se, boundaryCondition );
               Subtract( c_in, out, out, out.DataType() );
            } else {
               Image c_in = Separate( in, out );
               Closing( c_in, out, se, boundaryCondition );
               out -= c_in;
            }
            break;
         case EdgeType::OBJECT:
            if( white ) {
               Image tmp = Erosion( in, se, boundaryCondition );
               Dilation( tmp, out, se, boundaryCondition );
               out -= tmp;
            } else {
               Image tmp = Dilation( in, se, boundaryCondition );
               Erosion( tmp, out, se, boundaryCondition );
               Subtract( tmp, out, out, out.DataType() );
            }
            break;
      }
   DIP_END_STACK_TRACE
}

void MorphologicalThreshold(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      String const& edgeType,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      EdgeType decodedEdgeType = GetEdgeType( edgeType );
      Image tmp;
      switch( decodedEdgeType ) {
         case EdgeType::BOTH:
            Dilation( in, tmp, se, boundaryCondition );
            Erosion( in, out, se, boundaryCondition );
            out += tmp;
            out /= 2;
            break;
         default:
         case EdgeType::TEXTURE:
            Closing( in, tmp, se, boundaryCondition );
            Opening( in, out, se, boundaryCondition );
            out += tmp;
            out /= 2;
            break;
         case EdgeType::OBJECT: {
            Image c_in = Separate( in, out );
            Dilation( c_in, tmp, se, boundaryCondition );
            Erosion( tmp, out, se, boundaryCondition );
            Subtract( tmp, out, out, out.DataType() );
            Erosion( c_in, tmp, se, boundaryCondition );
            out += tmp;
            Dilation( tmp, tmp, se, boundaryCondition );
            out -= tmp;
            out /= 2;
            out += c_in;
            break;
         }
      }
   DIP_END_STACK_TRACE
}

void MorphologicalGist(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      String const& edgeType,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      EdgeType decodedEdgeType = GetEdgeType( edgeType );
      Image tmp;
      Image c_in = Separate( in,  out );
      switch( decodedEdgeType ) {
         case EdgeType::BOTH:
            Dilation( c_in, tmp, se, boundaryCondition );
            Erosion( c_in, out, se, boundaryCondition );
            out += tmp;
            out /= 2;
            Subtract( c_in, out, out, out.DataType() );
            break;
         default:
         case EdgeType::TEXTURE:
            Closing( c_in, tmp, se, boundaryCondition );
            Opening( c_in, out, se, boundaryCondition );
            out += tmp;
            out /= 2;
            Subtract( c_in, out, out, out.DataType() );
            break;
         case EdgeType::OBJECT:
            Dilation( c_in, tmp, se, boundaryCondition );
            Erosion( tmp, out, se, boundaryCondition );
            out -= tmp;
            Erosion( c_in, tmp, se, boundaryCondition );
            out -= tmp;
            Dilation( tmp, tmp, se, boundaryCondition );
            out += tmp;
            out /= 2;
            break;
      }
   DIP_END_STACK_TRACE
}

void MorphologicalRange(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      String const& edgeType,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      EdgeType decodedEdgeType = GetEdgeType( edgeType );
      Image tmp;
      switch( decodedEdgeType ) {
         case EdgeType::BOTH:
            Dilation( in, tmp, se, boundaryCondition );
            Erosion( in, out, se, boundaryCondition );
            Subtract( tmp, out, out, out.DataType() );
            break;
         default:
         case EdgeType::TEXTURE:
            Closing( in, tmp, se, boundaryCondition );
            Opening( in, out, se, boundaryCondition );
            Subtract( tmp, out, out, out.DataType() );
            break;
         case EdgeType::OBJECT: {
            Image c_in = Separate( in,  out );
            Dilation( c_in, tmp, se, boundaryCondition );
            Erosion( tmp, out, se, boundaryCondition );
            Subtract( tmp, out, out, out.DataType() );
            Erosion( c_in, tmp, se, boundaryCondition );
            out -= tmp;
            Dilation( tmp, tmp, se, boundaryCondition );
            out += tmp;
            break;
         }
      }
   DIP_END_STACK_TRACE
}

void Lee(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      String const& edgeType,
      String const& sign,
      StringArray const& boundaryCondition
) {
   DIP_START_STACK_TRACE
      Image out2;
      EdgeType decodedEdgeType = GetEdgeType( edgeType );
      Image c_in = Separate( in,  out );
      switch( decodedEdgeType ) {
         case EdgeType::BOTH:
            Dilation( c_in, out, se, boundaryCondition );
            out -= c_in;
            Erosion( c_in, out2, se, boundaryCondition );
            Subtract( c_in, out2, out2, out2.DataType() );
            break;
         default:
         case EdgeType::TEXTURE:
            Closing( c_in, out, se, boundaryCondition );
            out -= c_in;
            Opening( c_in, out2, se, boundaryCondition );
            Subtract( c_in, out2, out2, out2.DataType() );
            break;
         case EdgeType::OBJECT: {
            Image tmp;
            Dilation( c_in, tmp, se, boundaryCondition );
            Erosion( tmp, out, se, boundaryCondition );
            Subtract( tmp, out, out, out.DataType() );
            Erosion( c_in, tmp, se, boundaryCondition );
            Dilation( tmp, out2, se, boundaryCondition );
            out2 -= tmp;
            break;
         }
      }
      if( BooleanFromString( sign, S::SIGNED, S::UNSIGNED )) {
         SignedInfimum( out, out2, out );
      } else {
         Infimum( out, out2, out );
      }
   DIP_END_STACK_TRACE
}

void MorphologicalSmoothing(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      String const& mode,
      StringArray const& boundaryCondition
) {
   if( mode == S::OPENCLOSE ) {
      Opening( in, out, se, boundaryCondition );
      Closing( out, out, se, boundaryCondition );
   } else if( mode == S::CLOSEOPEN ) {
      Closing( in, out, se, boundaryCondition );
      Opening( out, out, se, boundaryCondition );
   } else if( mode == S::AVERAGE ) {
      Image tmp;
      Opening( in, tmp, se, boundaryCondition );
      Closing( tmp, tmp, se, boundaryCondition );
      Closing( in, out, se, boundaryCondition );
      Opening( out, out, se, boundaryCondition );
      out += tmp;
      out /= 2;
   } else {
      DIP_THROW_INVALID_FLAG( mode );
   }
}

void MultiScaleMorphologicalGradient(
      Image const& in,
      Image& out,
      dip::uint upperSize,
      dip::uint lowerSize,
      String const& shape,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( lowerSize > upperSize, "lowerSize > upperSize" );
   bool first = true;
   Image dila;
   Image eros;
   for( dip::uint ii = lowerSize; ii <= upperSize; ++ii ) {
      StructuringElement se1( 2.0 * static_cast< dfloat >( ii ) + 1.0, shape );
      StructuringElement se2( 2.0 * static_cast< dfloat >( ii - 1 ) + 1.0, shape );
      Dilation( in, dila, se1, boundaryCondition );
      Erosion( in, eros, se1, boundaryCondition );
      Subtract( dila, eros, eros, dila.DataType() );
      if( first ) {
         Erosion( eros, out, se2, boundaryCondition );
         first = false;
      } else {
         Erosion( eros, eros, se2, boundaryCondition );
         out += eros;
      }
   }
   out /= upperSize - lowerSize + 1;
}

void MorphologicalLaplace(
      Image const& in,
      Image& out,
      StructuringElement const& se,
      StringArray const& boundaryCondition
) {
   Image c_in = Separate( in,  out );
   Image tmp = Dilation( c_in, se, boundaryCondition );
   Erosion( c_in, out, se, boundaryCondition );
   out += tmp;
   out /= 2;
   out -= c_in;
}

void RankMinClosing(
      Image const& in,
      Image& out,
      StructuringElement se,
      dip::uint rank,
      StringArray const& boundaryCondition
) {
   Image c_in = Separate( in,  out );
   RankFilter( c_in, out, se, rank + 1, S::DECREASING, boundaryCondition );
   se.Mirror();
   Erosion( out, out, se, boundaryCondition );
   Supremum( c_in, out, out );
}

void RankMaxOpening(
      Image const& in,
      Image& out,
      StructuringElement se,
      dip::uint rank,
      StringArray const& boundaryCondition
) {
   Image c_in = Separate( in,  out );
   RankFilter( c_in, out, se, rank + 1, S::INCREASING, boundaryCondition );
   se.Mirror();
   Dilation( out, out, se, boundaryCondition );
   Infimum( c_in, out, out );
}

namespace {

enum class AlternatingSequentialFilterMode {
      STRUCTURAL,
      RECONSTRUCTION,
      AREA
};

void AlternatingSequentialFilterInternal(
      Image const& in,
      Image& out,
      dip::uint size,
      String const& shape,
      AlternatingSequentialFilterMode mode,
      bool openingFirst,
      StringArray const& boundaryCondition
) {
   switch( mode ) {
      case AlternatingSequentialFilterMode::STRUCTURAL: {
         StructuringElement se( static_cast< dfloat >( size ), shape );
         if( openingFirst ) {
            Opening( in,  out, se, boundaryCondition );
            Closing( out, out, se, boundaryCondition );
         } else {
            Closing( in,  out, se, boundaryCondition );
            Opening( out, out, se, boundaryCondition );
         }
         break;
      }
      case AlternatingSequentialFilterMode::RECONSTRUCTION: {
         StructuringElement se( static_cast< dfloat >( size ), shape );
         if( openingFirst ) {
            OpeningByReconstruction( in,  out, se, 1, boundaryCondition ); // TODO: connectivity?
            ClosingByReconstruction( out, out, se, 1, boundaryCondition );
         } else {
            ClosingByReconstruction( in,  out, se, 1, boundaryCondition );
            OpeningByReconstruction( out, out, se, 1, boundaryCondition );
         }
         break;
      }
      case AlternatingSequentialFilterMode::AREA:
         if( openingFirst ) {
            AreaOpening( in,  {}, out, size, 1 ); // TODO: connectivity?
            AreaClosing( out, {}, out, size, 1 );
         } else {
            AreaClosing( in,  {}, out, size, 1 );
            AreaOpening( out, {}, out, size, 1 );
         }
         break;
   }
}

} // namespace

void AlternatingSequentialFilter(
      Image const& in,
      Image& out,
      Range const& sizes,
      String const& shape,
      String const& s_mode,
      String const& polarity,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF(( sizes.step < 1 ) || ( sizes.start < 2 ) || ( sizes.stop < sizes.start ), E::INVALID_PARAMETER );
   bool openingFirst;
   DIP_STACK_TRACE_THIS( openingFirst = BooleanFromString( polarity, S::OPENCLOSE, S::CLOSEOPEN ));
   AlternatingSequentialFilterMode mode;
   if( s_mode == S::STRUCTURAL ) {
      mode = AlternatingSequentialFilterMode::STRUCTURAL;
   } else if( s_mode == S::RECONSTRUCTION ) {
      mode = AlternatingSequentialFilterMode::RECONSTRUCTION;
   } else if( s_mode == S::AREA ) {
      mode = AlternatingSequentialFilterMode::AREA;
   } else {
      DIP_THROW_INVALID_FLAG( s_mode );
   }
   auto size = sizes.begin();
   DIP_STACK_TRACE_THIS( AlternatingSequentialFilterInternal( in, out, *size, shape, mode, openingFirst, boundaryCondition ));
   ++size;
   for( ; size != sizes.end(); ++size ) {
      DIP_STACK_TRACE_THIS( AlternatingSequentialFilterInternal( out, out, *size, shape, mode, openingFirst, boundaryCondition ));
   }
}

void HitAndMiss(
      Image const& in,
      Image& out,
      StructuringElement const& hit,
      StructuringElement const& miss,
      String const& mode,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   bool constrained = false;
   if( in.DataType().IsBinary() ) {
      // We use the unconstrained mode, ignore `mode`.
      // Note that the constrained mode would yield the same result, but it's more expensive.
   } else {
      DIP_STACK_TRACE_THIS( constrained = BooleanFromString( mode, S::CONSTRAINED, S::UNCONSTRAINED ));
   }
   if( constrained ) {
      // Constrained HMT
      Image ero;
      DIP_STACK_TRACE_THIS( ero = Erosion( in, hit, boundaryCondition ));
      Image dil;
      DIP_STACK_TRACE_THIS( dil = Dilation( in, miss, boundaryCondition ));
      DataType dt = in.DataType();
      std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
      DIP_OVL_CALL_ASSIGN_REAL( scanLineFilter, Framework::NewTriadicScanLineFilter, (
            []( auto its ) { //  -> std::remove_reference_t< decltype( *its[ 0 ] ) >
               auto in = *its[ 0 ];
               auto ero = *its[ 1 ];
               auto dil = *its[ 2 ];
               if(( in == ero ) && ( dil < in )) {
                  return static_cast< decltype( *its[ 0 ] ) >( in - dil );
               }
               if(( in == dil ) && ( ero > in )) {
                  return static_cast< decltype( *its[ 0 ] ) >( ero - in );
               }
               return static_cast< decltype( *its[ 0 ] ) >( 0 );
            } ), dt );
      ImageRefArray outar{ out };
      DIP_STACK_TRACE_THIS( Framework::Scan( { in, ero, dil }, outar, { dt, dt, dt }, { dt }, { dt }, { 1 }, *scanLineFilter ));
   } else {
      // Unconstrained HMT
      Image dil;
      DIP_STACK_TRACE_THIS( dil = Dilation( in, miss, boundaryCondition ));
      DIP_STACK_TRACE_THIS( Erosion( in, out, hit, boundaryCondition ));
      out -= dil;
      if( out.DataType().IsSigned() ) {
         DIP_STACK_TRACE_THIS( ClipLow( out, out, 0 )); // set negative values to 0.
      } // If `out` is an unsigned type, the subtractions above are saturated, negative values automatically become 0.
   }
}

} // namespace dip
