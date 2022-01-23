/*
 * (c)2017, Cris Luengo.
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
#include "diplib/generation.h"
#include "diplib/overload.h"
#include "watershed_support.h"
#include "../binary/hilditch_condition_lut.h"

namespace dip {

namespace {

inline void SetBit( uint8& value, unsigned int bit ) {
   value = static_cast< uint8 >( value | ( 1u << bit ));
}
inline void ResetBit( uint8& value, unsigned int bit ) {
   value = static_cast< uint8 >( value & ~( 1u << bit ));
}
inline bool GetBit( uint8 value, unsigned int bit ) {
   return ( value & ( 1u << bit )) != 0;
}

dip::uint cast_with_check( dip::sint v ) {
   DIP_ASSERT( v >= 0 );
   return static_cast< dip::uint >( v );
}

template< typename TPI >
void UpperSkeleton2DInternal(
      Image& imgGrey,   // grey-value input image (TPI)
      Image& imgBin,  // skeleton output image (binary)
      std::vector< dip::sint >& offsets, // not sure why the algorithm writes to this list?
      uint8 const* lut,
      uint8 const* lut2
) {
   constexpr unsigned short nBuckets = 16; // unsigned short converts to dip::sint and dip::uint without warnings.
   constexpr bool goodMetric = true;       // code would be simpler with this set to false
   constexpr unsigned int dataBit = 0;     // the bit in `imgBin` that contains the skeleton
   constexpr unsigned int mtBit = 6;       // the bit in `imgBin` that is used internally

   TPI* pGrey = static_cast< TPI* >( imgGrey.Origin() );
   uint8* pBin = static_cast< uint8* >( imgBin.Origin() ); // even though it's a binary image, we want to access different bits easily.
   dip::sint* const indices = offsets.data();
   dip::sint* const indices_end = indices + offsets.size();

   dip::sint pdx = imgGrey.Stride( 0 );
   dip::sint pdy = imgGrey.Stride( 1 );
   DIP_ASSERT( imgBin.Stride( 0 ) == pdx );
   DIP_ASSERT( imgBin.Stride( 1 ) == pdy );

   // skeletonize greylevel in order of increasing distance to lower levels

   dip::sint* pir = indices;
   dip::sint* piw = indices;
   dip::sint* pir2 = nullptr;
   std::array< dip::sint*, nBuckets > pibuck{ 0 };
   while( pir < indices_end ) {
      TPI cb = pGrey[ *pir ];
      dip::sint* pibuck0 = pibuck[ 0 ] = piw;
      while( true ) {
         dip::sint ci = *pir;
         TPI* pc = pGrey + ci;
         uint8* pb = pBin + ci;
         TPI vc = *pc;
         if( vc != cb ) {
            break;
         }

         // 4-c neighbor of lower distance?

         // local minimum
         uint8 ee = 0;
         if( *( pc + pdx ) < vc ) {
            ee |= 1u;
            if( !GetBit( *( pb + pdx ), dataBit )) {
               ee |= 2u;
               goto skip;
            }
         }
         if( *( pc - pdx ) < vc ) {
            ee |= 1u;
            if( !GetBit( *( pb - pdx ), dataBit )) {
               ee |= 2u;
               goto skip;
            }
         }
         if( *( pc + pdy ) < vc ) {
            ee |= 1u;
            if( !GetBit( *( pb + pdy ), dataBit )) {
               ee |= 2u;
               goto skip;
            }
         }
         if( *( pc - pdy ) < vc ) {
            ee |= 1u;
            if( !GetBit( *( pb - pdy ), dataBit )) {
               ee |= 2u;
            }
         }

         skip:
         switch( ee ) {
            case 0:
               ResetBit( *pb, dataBit);
               break;
            case 1:
            case 3:
               *( piw++ ) = ci;
               break;
            default:
               break;

         }
         ++pir;
         if( pir >= indices_end ) {
            break;
         }
      }

      std::array< dip::uint, nBuckets > countbuck{};   // This zero-initializes the array, but is it guaranteed?
      countbuck.fill( 0 );                            // I'm not counting on it!
      countbuck[ 0 ] = cast_with_check( piw - pibuck[ 0 ] );
      std::vector< dip::uint > distc;
      distc.reserve( 5000 ); // This should be enough in most cases.
      distc.push_back( countbuck[ 0 ] );
      dip::sint go = 0;
      if( distc[ 0 ] ) {
         go = nBuckets;
      }

      dip::uint dist = 1;
      while( go > 0 ) {
         dist = distc.size();
         pibuck[ dist % nBuckets ] = piw;
         dip::uint count = 0;
         if( dist >= 5 ) {
            count = countbuck[ ( dist - 5 ) % nBuckets ];
            pir2 = pibuck[ ( dist - 5 ) % nBuckets ];
         }
         while( count-- > 0 ) {
            dip::sint ci = *( pir2++ );
            TPI* pc = pGrey + ci;
            TPI vc = *pc;
            uint8* pb = pBin + ci;
            if( !GetBit( *( pb + pdx ), dataBit ) && ( *( pc + pdx ) == vc )) {
               *( piw++ ) = ci + pdx;
               SetBit( *( pb + pdx ), dataBit );
            }
            if( !GetBit( *( pb - pdx ), dataBit ) && ( *( pc - pdx ) == vc )) {
               *( piw++ ) = ci - pdx;
               SetBit( *( pb - pdx ), dataBit );
            }
            if( !GetBit( *( pb + pdy ), dataBit ) && ( *( pc + pdy ) == vc )) {
               *( piw++ ) = ci + pdy;
               SetBit( *( pb + pdy ), dataBit );
            }
            if( !GetBit( *( pb - pdy ), dataBit ) && ( *( pc - pdy ) == vc )) {
               *( piw++ ) = ci - pdy;
               SetBit( *( pb - pdy ), dataBit );
            }
         }
         if(( dist >= 7 ) && goodMetric) {
            count = countbuck[ ( dist - 7 ) % nBuckets ];
            pir2 = pibuck[ ( dist - 7 ) % nBuckets ];
         } else {
            count = 0;
         }
         while( count-- > 0 ) {
            dip::sint ci = *( pir2++ );
            TPI* pc = pGrey + ci;
            TPI vc = *pc;
            uint8* pb = pBin + ci;
            if(( *( pc + pdx ) == vc || *( pc + pdy ) == vc ) && !GetBit( *( pb + pdx + pdy ), dataBit ) &&
               *( pc + pdx + pdy ) == vc ) {
               *( piw++ ) = ci + pdx + pdy;
               SetBit( *( pb + pdx + pdy ), dataBit );
            }
            if(( *( pc - pdx ) == vc || *( pc + pdy ) == vc ) && !GetBit( *( pb - pdx + pdy ), dataBit ) &&
               *( pc - pdx + pdy ) == vc ) {
               *( piw++ ) = ci - pdx + pdy;
               SetBit( *( pb - pdx + pdy ), dataBit );
            }
            if(( *( pc + pdx ) == vc || *( pc - pdy ) == vc ) && !GetBit( *( pb + pdx - pdy ), dataBit ) &&
               *( pc + pdx - pdy ) == vc ) {
               *( piw++ ) = ci + pdx - pdy;
               SetBit( *( pb + pdx - pdy ), dataBit );
            }
            if(( *( pc - pdx ) == vc || *( pc - pdy ) == vc ) && !GetBit( *( pb - pdx - pdy ), dataBit ) &&
               *( pc - pdx - pdy ) == vc ) {
               *( piw++ ) = ci - pdx - pdy;
               SetBit( *( pb - pdx - pdy ), dataBit );
            }
         }
         if(( dist >= 11 ) && goodMetric ) {
            count = countbuck[ ( dist - 11 ) % nBuckets ];
            pir2 = pibuck[ ( dist - 11 ) % nBuckets ];
         } else {
            count = 0;
         }
         while( count-- > 0 ) {
            dip::sint ci = *( pir2++ );
            TPI* pc = pGrey + ci;
            TPI vc = *pc;
            uint8* pb = pBin + ci;
            if((( *( pc + pdx ) == vc && *( pc + 2 * pdx ) == vc ) ||
                ( *( pc + pdx ) == vc && *( pc + pdx + pdy ) == vc ) ||
                ( *( pc + pdy ) == vc && *( pc + pdx + pdy ) == vc ))
               && !GetBit( *( pb + 2 * pdx + pdy ), dataBit )
               && *( pc + 2 * pdx + pdy ) == vc ) {
               *( piw++ ) = ci + 2 * pdx + pdy;
               SetBit( *( pb + 2 * pdx + pdy ), dataBit );
            }
            if((( *( pc + pdx ) == vc && *( pc + 2 * pdx ) == vc ) ||
                ( *( pc + pdx ) == vc && *( pc + pdx - pdy ) == vc ) ||
                ( *( pc - pdy ) == vc && *( pc + pdx - pdy ) == vc ))
               && !GetBit( *( pb + 2 * pdx - pdy ), dataBit )
               && *( pc + 2 * pdx - pdy ) == vc ) {
               *( piw++ ) = ci + 2 * pdx - pdy;
               SetBit( *( pb + 2 * pdx - pdy ), dataBit );
            }
            if((( *( pc - pdx ) == vc && *( pc - 2 * pdx ) == vc ) ||
                ( *( pc - pdx ) == vc && *( pc - pdx - pdy ) == vc ) ||
                ( *( pc - pdy ) == vc && *( pc - pdx - pdy ) == vc ))
               && !GetBit( *( pb - 2 * pdx - pdy ), dataBit )
               && *( pc - 2 * pdx - pdy ) == vc ) {
               *( piw++ ) = ci - 2 * pdx - pdy;
               SetBit( *( pb - 2 * pdx - pdy ), dataBit );
            }
            if((( *( pc - pdx ) == vc && *( pc - 2 * pdx ) == vc ) ||
                ( *( pc - pdx ) == vc && *( pc - pdx + pdy ) == vc ) ||
                ( *( pc + pdy ) == vc && *( pc - pdx + pdy ) == vc ))
               && !GetBit( *( pb - 2 * pdx + pdy ), dataBit )
               && *( pc - 2 * pdx + pdy ) == vc ) {
               *( piw++ ) = ci - 2 * pdx + pdy;
               SetBit( *( pb - 2 * pdx + pdy ), dataBit );
            }
            if((( *( pc + pdy ) == vc && *( pc + 2 * pdy ) == vc ) ||
                ( *( pc + pdy ) == vc && *( pc + pdx + pdy ) == vc ) ||
                ( *( pc + pdx ) == vc && *( pc + pdx + pdy ) == vc ))
               && !GetBit( *( pb + pdx + 2 * pdy ), dataBit )
               && *( pc + pdx + 2 * pdy ) == vc ) {
               *( piw++ ) = ci + pdx + 2 * pdy;
               SetBit( *( pb + pdx + 2 * pdy ), dataBit );
            }
            if((( *( pc - pdy ) == vc && *( pc - 2 * pdy ) == vc ) ||
                ( *( pc - pdy ) == vc && *( pc + pdx - pdy ) == vc ) ||
                ( *( pc + pdx ) == vc && *( pc + pdx - pdy ) == vc ))
               && !GetBit( *( pb + pdx - 2 * pdy ), dataBit )
               && *( pc + pdx - 2 * pdy ) == vc ) {
               *( piw++ ) = ci + pdx - 2 * pdy;
               SetBit( *( pb + pdx - 2 * pdy ), dataBit );
            }
            if((( *( pc + pdy ) == vc && *( pc + 2 * pdy ) == vc ) ||
                ( *( pc + pdy ) == vc && *( pc - pdx + pdy ) == vc ) ||
                ( *( pc - pdx ) == vc && *( pc - pdx + pdy ) == vc ))
               && !GetBit( *( pb - pdx + 2 * pdy ), dataBit )
               && *( pc - pdx + 2 * pdy ) == vc ) {
               *( piw++ ) = ci - pdx + 2 * pdy;
               SetBit( *( pb - pdx + 2 * pdy ), dataBit );
            }
            if((( *( pc - pdy ) == vc && *( pc - 2 * pdy ) == vc ) ||
                ( *( pc - pdy ) == vc && *( pc - pdx - pdy ) == vc ) ||
                ( *( pc - pdx ) == vc && *( pc - pdx - pdy ) == vc ))
               && !GetBit( *( pb - pdx - 2 * pdy ), dataBit )
               && *( pc - pdx - 2 * pdy ) == vc ) {
               *( piw++ ) = ci - pdx - 2 * pdy;
               SetBit( *( pb - pdx - 2 * pdy ), dataBit );
            }
         }
         distc.push_back( countbuck[ dist % nBuckets ] = cast_with_check( piw - pibuck[ dist % nBuckets ] ));
         if( distc.back() == 0 ) {
            --go;
         } else {
            go = nBuckets;
         }
      }

      // skeletonization

      pir2 = pibuck0;
      for( dip::uint cd = 0; cd < dist; ++cd ) {
         for( dip::uint count = distc[ cd ]; count > 0; --count ) {
            uint8* pb = pBin + *( pir2++ );
            uint8 ee = 0;
            if( GetBit( *( pb + 1 ), dataBit )) { SetBit( ee, 0 ); }
            if( GetBit( *( pb - pdy + 1 ), dataBit )) { SetBit( ee, 1 ); }
            if( GetBit( *( pb - pdy ), dataBit )) { SetBit( ee, 2 ); }
            if( GetBit( *( pb - pdy - 1 ), dataBit )) { SetBit( ee, 3 ); }
            if( GetBit( *( pb - 1 ), dataBit )) { SetBit( ee, 4 ); }
            if( GetBit( *( pb + pdy - 1 ), dataBit )) { SetBit( ee, 5 ); }
            if( GetBit( *( pb + pdy ), dataBit )) { SetBit( ee, 6 ); }
            if( GetBit( *( pb + pdy + 1 ), dataBit )) { SetBit( ee, 7 ); }
            if( !lut[ ee ] ) {
               uint8 e1 = ee;
               uint8 e2 = ee;
               uint8 e3 = ee;
               uint8 e4 = ee;
               if( GetBit( *( pb - pdy ), mtBit )) {
                  ResetBit( e1, 2 );
                  ResetBit( ee, 2 );
               }
               if( GetBit( *( pb - 1 ), mtBit )) {
                  ResetBit( e2, 4 );
                  ResetBit( ee, 4 );
               }
               if( GetBit( *( pb + 1 ), mtBit )) {
                  ResetBit( e3, 0 );
                  ResetBit( ee, 0 );
               }
               if( GetBit( *( pb + pdy ), mtBit )) {
                  ResetBit( e4, 6 );
                  ResetBit( ee, 6 );
               }
               if( !lut2[ ee ] && !lut2[ e1 ] && !lut2[ e2 ] && !lut2[ e3 ] && !lut2[ e4 ] ) {
                  SetBit( *pb, mtBit );
               }
            }
         }
         pir2 -= distc[ cd ];
         for( dip::uint count = distc[ cd ]; count > 0; --count ) {
            dip::sint ci = *( pir2++ );
            uint8* pb = pBin + ci;
            if( GetBit( *pb, mtBit )) {
               ResetBit( *pb, dataBit );
               ResetBit( *pb, mtBit );
               //TPI* pt = fim + ci;
               //TPI vn = *( pt + 1 );
               //if( *( pt - 1 ) < vn ) { vn = *( pt - 1 ); }
               //if( *( pt + pdy ) < vn ) { vn = *( pt + pdy ); }
               //if( *( pt - pdy ) < vn ) { vn = *( pt - pdy ); }
               //*pt = vn;
            }
         }

      }
   }
}

} // namespace

void UpperSkeleton2D(
      Image const& in,
      Image const& c_mask,
      Image& out,
      String const& s_endPixelCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );

   // Check mask, expand mask singleton dimensions if necessary
   Image mask;
   bool hasMask = false;
   if( c_mask.IsForged() ) {
      mask = c_mask.QuickCopy();
      DIP_START_STACK_TRACE
         mask.CheckIsMask( in.Sizes(), Option::AllowSingletonExpansion::DO_ALLOW, Option::ThrowException::DO_THROW );
         mask.ExpandSingletonDimensions( in.Sizes() );
      DIP_END_STACK_TRACE
      hasMask = true;
   }

   // End pixel condition
   uint8 const* lut;
   uint8 const* lut2 = luthil[ 0 ];
   if( s_endPixelCondition == S::NATURAL ) {
      lut = luthil[ 0 ];
   } else if( s_endPixelCondition == S::ONE_NEIGHBOR ) {
      lut = luthil[ 1 ];
   } else if( s_endPixelCondition == S::TWO_NEIGHBORS ) {
      lut = luthil[ 2 ];
   } else if( s_endPixelCondition == S::THREE_NEIGHBORS ) {
      lut = luthil[ 3 ];
   } else {
      DIP_THROW_INVALID_FLAG( s_endPixelCondition );
   }

   // Copy input to output. Operation takes place directly in the output.
   if( &in == &out ) {
      // No copy necessary, except we need the data to be contiguous
      DIP_STACK_TRACE_THIS( out.ForceContiguousData() );
   } else {
      DIP_START_STACK_TRACE
         out.ReForge( in ); // reforging first in case `out` is the right size but a different data type
         out.Copy( in );
      DIP_END_STACK_TRACE
   }
   SetBorder( out, { infinity }, { 1 } ); // set border to +infinity, or whatever the largest value is for the image's type

   Image skeleton;
   skeleton.SetSizes( out.Sizes() );
   skeleton.SetStrides( out.Strides() );
   skeleton.SetDataType( DT_BIN );
   skeleton.Forge();
   DIP_ASSERT( skeleton.Strides() == out.Strides() );
   skeleton.Fill( 1 );

   // Create sorted offsets array (skipping border)
   std::vector< dip::sint > offsets;
   if( hasMask ) {
      offsets = CreateOffsetsArray( mask, out.Strides() );
   } else {
      offsets = CreateOffsetsArray( out.Sizes(), out.Strides() );
   }
   if( offsets.empty() ) {
      // This can happen if `mask` is empty.
      return;
   }
   SortOffsets( out, offsets, true );

   // Compute the skeleton
   DIP_START_STACK_TRACE
      DIP_OVL_CALL_REAL( UpperSkeleton2DInternal, ( out, skeleton, offsets, lut, lut2 ), out.DataType() );
   DIP_END_STACK_TRACE
   offsets = {};

   // Set the non-skeleton pixels to whatever the lowest value is for the image's type
   Not( skeleton, skeleton );
   SetBorder( skeleton, { 1 }, { 1 } ); // select border pixels too
   DIP_STACK_TRACE_THIS( out.At( skeleton ) = -infinity );
}

} // namespace dip
