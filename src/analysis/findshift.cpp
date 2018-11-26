/*
 * DIPlib 3.0
 * This file contains definitions for shift estimation
 *
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
#include "diplib/analysis.h"
#include "diplib/transform.h"
#include "diplib/math.h"
#include "diplib/statistics.h"
#include "diplib/linear.h"
#include "diplib/geometry.h"

namespace dip {

void CrossCorrelationFT(
      Image const& in1,
      Image const& in2,
      Image& out,
      String const& in1Representation,
      String const& in2Representation,
      String const& outRepresentation,
      String const& normalize
) {
   DIP_THROW_IF( !in1.IsForged() || !in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in1.IsScalar() || !in2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( in1.DataType().IsBinary() || in2.DataType().IsBinary(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( in1.Sizes() != in2.Sizes(), E::SIZES_DONT_MATCH );
   bool in1spatial;
   DIP_STACK_TRACE_THIS( in1spatial = BooleanFromString( in1Representation, S::SPATIAL, S::FREQUENCY ));
   bool in2spatial;
   DIP_STACK_TRACE_THIS( in2spatial = BooleanFromString( in2Representation, S::SPATIAL, S::FREQUENCY ));
   Image in1FT;
   if( in1spatial ) {
      DIP_THROW_IF( !in1.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_STACK_TRACE_THIS( FourierTransform( in1, in1FT ));
   } else {
      in1FT = in1.QuickCopy();
   }
   Image in2FT;
   if( in2spatial ) {
      DIP_THROW_IF( !in2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_STACK_TRACE_THIS( FourierTransform( in2, in2FT ));
   } else {
      in2FT = in2.QuickCopy();
   }
   DataType dt = in1FT.DataType();
   DIP_STACK_TRACE_THIS( MultiplyConjugate( in1FT, in2FT, out, dt ));
   if( normalize == S::NORMALIZE ) {
      if( in2FT.IsShared() ) {
         in2FT.Strip(); // make sure we don't write in any input data segments. Otherwise, we re-use the data segment.
      }
      SquareModulus( in1FT, in2FT );
      SafeDivide( out, in2FT, out, out.DataType() ); // Normalize by the square modulus of in1.
   } else if( normalize == S::PHASE ) {
      Image tmp;
      Modulus( in1FT, tmp );
      SafeDivide( out, tmp, out, out.DataType() );
      Modulus( in2FT, tmp );
      SafeDivide( out, tmp, out, out.DataType() );
   } else if( normalize != S::DONT_NORMALIZE ) {
      DIP_THROW_INVALID_FLAG( normalize );
   }
   if( BooleanFromString( outRepresentation, S::SPATIAL, S::FREQUENCY )) {
      DIP_STACK_TRACE_THIS( FourierTransform( out, out, { S::INVERSE, S::REAL } ));
   }
}

namespace {

FloatArray FindShift_CPF( Image const& in1, Image const& in2, dfloat maxFrequency ) {
   DIP_THROW_IF( in1.Dimensionality() != 2, E::DIMENSIONALITY_NOT_SUPPORTED );
   if( maxFrequency <= 0.0 ) {
      maxFrequency = 0.2;
   }
   // Do cross-correlation for sub-pixel shift, forcing dcomplex output
   Image cross{ in1.Sizes(), 1, DT_DCOMPLEX };
   cross.Protect();
   CrossCorrelationFT( in1, in2, cross, S::SPATIAL, S::SPATIAL, S::FREQUENCY, S::NORMALIZE );
   DIP_ASSERT( cross.DataType() == DT_DCOMPLEX );
   DIP_ASSERT( cross.Stride( 0 ) == 1 );
   // Do the least squares fit
   dip::sint center_x = static_cast< dip::sint >( cross.Size( 0 ) / 2 );
   dip::sint center_y = static_cast< dip::sint >( cross.Size( 1 ) / 2 );
   dip::uint amount = 0;
   dfloat sumuv = 0;
   dfloat sumuu = 0;
   dfloat sumvv = 0;
   dfloat sumAv = 0;
   dfloat sumAu = 0;
   dfloat radius = maxFrequency * maxFrequency;
   dfloat du = 2.0 * pi / static_cast< dfloat >( cross.Size( 0 ));
   dfloat dv = 2.0 * pi / static_cast< dfloat >( cross.Size( 1 ));
   dfloat uStart = static_cast< dfloat >( 0 - center_x ) * du;
   dfloat v = static_cast< dfloat >( 0 - center_y ) * dv;
   // For the sake of simplicity, we forgo the Framework
   for( dip::sint jj = 0; jj < static_cast< dip::sint >( cross.Size( 1 )); ++jj ) {
      dcomplex* ptr = static_cast< dcomplex* >( cross.Origin() ) + cross.Stride( 1 ) * jj; // just in case stride != width, which should not happen.
      dfloat vv = v * v;
      if( vv < radius ) {
         dfloat u = uStart;
         for( dip::sint ii = 0; ii < static_cast< dip::sint >( cross.Size( 0 )); ii++ ) {
            dfloat uu = u * u;
            if( uu + vv < radius ) {
               dfloat amplitude = std::abs( *ptr );
               if( std::abs( amplitude - 1 ) < 0.1 ) {
                  // Use this point
                  dfloat angle = std::arg( *ptr );
                  sumuv += u * v;
                  sumuu += uu;
                  sumvv += vv;
                  sumAv += angle * v;
                  sumAu += angle * u;
                  ++amount;
               }
            }
            ++ptr;
            u += du;
         }
      }
      v += dv;
   }
   DIP_THROW_IF( amount < 3, "Too few valid data points to do calculation" );
   dfloat value = sumuv * sumuv - sumvv * sumuu;
   DIP_THROW_IF ( value == 0, "Parameter 'value' is zero" );
   return FloatArray{
         ( sumAv * sumuv - sumAu * sumvv ) / value,
         ( sumAu * sumuv - sumAv * sumuu ) / value
   };
   // TODO: Add removal of outliers as an option to CPF.
   // TODO: CPF can probably be computed independently for each dimension by averaging fits along each line.
}

FloatArray FindShift_MTS( Image const& in1, Image const& in2, dip::uint iterations, dfloat accuracy, dfloat sigma ) {
   dip::uint nDims = in1.Dimensionality();
   FloatArray out( nDims, 0.0 );
   FloatArray shift( nDims, 0.0 );
   FloatArray previousShift( nDims, 0.0 );
   FloatArray previousPreviousShift( nDims, 0.0 );

   // Solve: sum( gradient * gradient' ) * shift = sum(( in1 - in2 ) * gradient )
   // Solve: M * shift = V
   Image in1g = Gauss( in1, { sigma } );
   Image in2g = Gauss( in2, { sigma } );
   Image gradient = Gradient( in1, { sigma } );
   Image M = Sum( Multiply( gradient, Transpose( gradient )));
   M.Convert( DT_DFLOAT );
   M.ExpandTensor(); // Multiply yields a symmetric tensor, here we force the storage to be normal

   // iterative Taylor with early break if accuracy is achieved
   dip::uint ii;
   for( ii = 0; ii < iterations; ii++ ) {
      Image tmp;
      FloatArray invShift = out;
      for( auto& s: invShift ) {
         s = -s;
      }
      if( ii == 0 ) {
         tmp.Copy( in2g ); // Subtract later will work in-place, we don't want to overwrite in2g.
      } else if(( ii == 1 ) || ( ii == 2 )) {
         // If ii > 0, we shift in2
         //std::cout << "[FindShift_MTS] shifting in2 by " << invShift << std::endl;
         tmp = Shift( in2g, invShift, "3-cubic" );
      } else {
         // Use non-smoothed image for iterations after the 3rd one.
         tmp = Shift( in2, invShift, "3-cubic" );
         in1g = in1.QuickCopy();
      }
      Subtract( in1g, tmp, tmp, tmp.DataType() );
      Image V = Sum( tmp * gradient );
      V.Convert( DT_DFLOAT );
      previousPreviousShift = previousShift;
      previousShift = shift;
      Solve( nDims, nDims, { static_cast< dfloat* >( M.Origin() ), M.TensorStride() },
                           { static_cast< dfloat* >( V.Origin() ), V.TensorStride() }, shift.begin() );
      out += shift;
      //std::cout << "[FindShift_MTS] iter = " << ii << ", shift = " << shift << ", out = " << out << std::endl;
      // break if desired accuracy achieved or good condition for bias correction
      if(( ii >= 2 ) && ( ii < iterations - 1 )) { // if ii == maxIter-1 => loop would end anyway
         bool done = false;
         bool small = true;
         for( dip::uint kk = 0; kk < nDims; ++kk ) {
            if( std::abs( shift[ kk ] ) > std::abs( previousShift[ kk ] )) {
               done = true;
               break;
            }
            if( accuracy > 0.0 ) {
               if( std::abs( shift[ kk ] ) > accuracy ) {
                  small = false;
               }
            } else {
               if( std::abs( previousPreviousShift[ kk ] * shift[ kk ] / previousShift[ kk ] / previousShift[ kk ] ) <= 1.05 ) {
                  small = false;
               }
            }
         }
         if( done || small ) {
            ++ii;
            //std::cout << "[FindShift_MTS] breaking from loop\n";
            break;
         }
      }
   }
   if( ii > 2 ) {
      // shortcut bias formula
      //std::cout << "[FindShift_MTS] using bias formula: bias = {";
      for( dip::uint kk = 0; kk < nDims; ++kk ) {
         dfloat bias = previousShift[ kk ] * previousShift[ kk ] /
                       ( previousPreviousShift[ kk ] - previousShift[ kk ] ); // beware /0 instability
         //std::cout << " " << bias;
         if( std::abs( bias ) < std::abs( previousShift[ kk ] )) { // bias should be smaller than last incremented shift
            out[ kk ] += bias;
         }
      }
      //std::cout << "}, out = " << out << std::endl;
   }
   return out;
}

FloatArray FindShift_PROJ( Image const& in1, Image const& in2, dip::uint iterations, dfloat accuracy, dfloat sigma ) {
   dip::uint nDims = in1.Dimensionality();
   FloatArray shift( nDims, 0.0 );
   BooleanArray ps( nDims, true );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      DIP_START_STACK_TRACE
         ps[ ii ] = false;
         Image line1 = Sum( in1, {}, ps );
         Image line2 = Sum( in2, {}, ps );
         ps[ ii ] = true;
         line1.Squeeze();
         line2.Squeeze();
         FloatArray res = FindShift_MTS( line1, line2, iterations, accuracy, sigma );
         shift[ ii ] = res[ 0 ];
      DIP_END_STACK_TRACE
   }
   return shift;
}

FloatArray FindShift_CC(
      Image const& in1,
      Image const& in2,
      UnsignedArray const& maxShift,
      String const& normalize = S::DONT_NORMALIZE,
      bool subpixelPrecision = false
) {
   dip::uint nDims = in1.Dimensionality();
   Image cross;
   DIP_STACK_TRACE_THIS( CrossCorrelationFT( in1, in2, cross, S::SPATIAL, S::SPATIAL, S::SPATIAL, normalize ));
   DIP_ASSERT( cross.DataType().IsReal() );
   UnsignedArray sizes = cross.Sizes();
   bool crop = false;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::uint maxSz = 2 * maxShift[ ii ] + 1;
      if( sizes[ ii ] > maxSz ) {
         sizes[ ii ] = maxSz;
         crop = true;
      }
   }
   if( crop ) {
      DIP_STACK_TRACE_THIS( cross.Crop( sizes ));
   }
   UnsignedArray maxPixel;
   DIP_STACK_TRACE_THIS( maxPixel = MaximumPixel( cross, {} ));
   FloatArray shift;
   if( subpixelPrecision ) {
      DIP_START_STACK_TRACE
         SubpixelLocationResult loc = SubpixelLocation( cross, maxPixel );
         shift = loc.coordinates;
      DIP_END_STACK_TRACE
   } else {
      shift = FloatArray{ maxPixel };
   }
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dfloat centerpixel = static_cast< dfloat >( sizes[ ii ] / 2 );
      shift[ ii ] = centerpixel - shift[ ii ]; // Reverse sign of shift
   }
   return shift;
}

FloatArray CorrectIntegerShift(
      Image& in1,
      Image& in2,
      UnsignedArray const& maxShift
) {
   dip::uint nDims = in1.Dimensionality();
   FloatArray shift;
   DIP_STACK_TRACE_THIS( shift = FindShift_CC( in1, in2, maxShift ));
   if( shift.any() ) {
      // Shift is non-zero along at least one dimension
      // Correct for this integer shift by cropping both images
      UnsignedArray sizes = in1.Sizes();
      UnsignedArray origin( nDims, 0 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] = shift[ ii ] > 0 ? static_cast< dip::uint >( shift[ ii ] ) : 0;
         sizes[ ii ] -= static_cast< dip::uint >( std::abs( shift[ ii ] ));
      }
      in2.dip__SetSizes( sizes );
      in2.dip__SetOrigin( in2.Pointer( origin ));
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         origin[ ii ] = shift[ ii ] < 0 ? static_cast< dip::uint >( -shift[ ii ] ) : 0;
      }
      in1.dip__SetSizes( sizes );
      in1.dip__SetOrigin( in1.Pointer( origin ));
   }
   return shift;
}

} // namespace

FloatArray FindShift(
      Image const& c_in1,
      Image const& c_in2,
      String const& method,
      dfloat parameter,
      UnsignedArray maxShift
) {
   DIP_THROW_IF( !c_in1.IsForged() || !c_in2.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in1.IsScalar() || !c_in2.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !c_in1.DataType().IsReal() || !c_in2.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( c_in1.Sizes() != c_in2.Sizes(), E::SIZES_DONT_MATCH );
   dip::uint nDims = c_in1.Dimensionality();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( maxShift, nDims, std::numeric_limits< dip::uint >::max() ));
   FloatArray shift( nDims, 0.0 );
   if( method == "integer only" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, S::DONT_NORMALIZE, false ));
   } else if( method == "CC" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, S::DONT_NORMALIZE, true ));
   } else if( method == "NCC" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, S::NORMALIZE, true ));
   } else if( method == "PC" ) {
      DIP_STACK_TRACE_THIS( shift = FindShift_CC( c_in1, c_in2, maxShift, S::PHASE, true ));
   } else {
      Image in1 = c_in1.QuickCopy();
      Image in2 = c_in2.QuickCopy();
      DIP_STACK_TRACE_THIS( shift = CorrectIntegerShift( in1, in2, maxShift )); // modifies in1 and in2
      if( method == "CPF" ) {
         DIP_STACK_TRACE_THIS( shift += FindShift_CPF( in1, in2, parameter ));
      } else if( method == "MTS" ) {
         if( parameter <= 0.0 ) {
            parameter = 1.0;
         }
         DIP_STACK_TRACE_THIS( shift += FindShift_MTS( in1, in2, 1, 0.0, parameter ));
      } else {
         dip::uint maxIter = 5;  // default number of iteration => accuracy ~ 1e-4
         dfloat accuracy = 0.0;  // signals early break if bias correction is possible
         if( parameter < 0.0 ) {
            maxIter = std::max( dip::uint{ 1 }, static_cast< dip::uint >( round_cast( -parameter )));
            accuracy = 1e-10;    // so small that maxIter would play its role
         } else if(( parameter > 0.0 ) && ( parameter <= 0.1 )) {
            maxIter = 20;        // NOTE: more iteration solution may end up very far from truth
            accuracy = parameter;
         }
         if( method == "ITER" ) {
            DIP_STACK_TRACE_THIS( shift += FindShift_MTS( in1, in2, maxIter, accuracy, 1.0 ));
         } else if( method == "PROJ" ) {
            DIP_STACK_TRACE_THIS( shift += FindShift_PROJ( in1, in2, maxIter, accuracy, 1.0 )); // calls FindShift_MTS
         } else {
            DIP_THROW_INVALID_FLAG( method );
         }
      }
   }
   return shift;
}

} // namespace dip

#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"

DOCTEST_TEST_CASE("[DIPlib] testing the FindShift fuction") {
   // Something to shift
   dip::Image in1( { 250, 261 }, 1, dip::DT_SFLOAT );
   dip::FillRadiusCoordinate( in1 );
   in1 -= 100;
   dip::Erf( in1, in1 );
   // A shift
   dip::FloatArray shift{ 10.27, 6.08 };
   dip::Image in2 = dip::Shift( in1, shift, "3-cubic" );
   // Add a little bit of noise
   dip::Random random( 0 );
   dip::GaussianNoise( in1, in1, random, 0.005 * 0.005 );
   dip::GaussianNoise( in2, in2, random, 0.005 * 0.005 );

   dip::FloatArray result;

   // Method: "integer only"
   result = FindShift( in1, in2, "integer only" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( result[ 0 ] == std::round( shift[ 0 ] ));
   DOCTEST_CHECK( result[ 1 ] == std::round( shift[ 1 ] ));

   // Method: "CC"
   result = FindShift( in1, in2, "CC" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.03 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.03 );

   // Method: "NCC"
   result = FindShift( in1, in2, "NCC" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.15 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.15 );

   // Method: "CPF"
   result = FindShift( in1, in2, "CPF" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.05 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.05 );

   // Method: "MTS"
   result = FindShift( in1, in2, "MTS" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.007 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.007 );

   // Method: "ITER"
   result = FindShift( in1, in2, "ITER" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.002 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.002 );

   // Method: "PROJ"
   result = FindShift( in1, in2, "PROJ" );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.004 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.004 );

   // Method: "CC", with max shift
   result = FindShift( in1, in2, "CC", 0, { 11 } );
   DOCTEST_REQUIRE( result.size() == 2 );
   DOCTEST_CHECK( std::abs( result[ 0 ] - shift[ 0 ] ) < 0.03 );
   DOCTEST_CHECK( std::abs( result[ 1 ] - shift[ 1 ] ) < 0.03 );
}

#endif // DIP__ENABLE_DOCTEST
