/*
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
#include "diplib/geometry.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {

namespace {

template< typename TPI >
class WrapLineFilter : public Framework::SeparableLineFilter {
   public:
      WrapLineFilter( UnsignedArray const& wrap ) : wrap_( wrap ) {}
      dip::uint GetNumberOfOperations( dip::uint lineLength, dip::uint, dip::uint, dip::uint ) override {
         return lineLength;
      }
      void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         SampleIterator< TPI > in{ static_cast< TPI* >( params.inBuffer.buffer ), params.inBuffer.stride };
         SampleIterator< TPI > out{ static_cast< TPI* >( params.outBuffer.buffer ), params.outBuffer.stride };
         dip::uint length = params.inBuffer.length;
         dip::uint wrap = wrap_[ params.dimension ]; // wrap > 0 && wrap < length
         std::copy( in, in + length - wrap, out + wrap );
         std::copy( in + length - wrap, in + length, out );
      }
   private:
      UnsignedArray const& wrap_;
};

} // namespace

void Wrap(
      Image const& in,
      Image& out,
      IntegerArray wrap
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims == 0, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_STACK_TRACE_THIS( ArrayUseParameter( wrap, nDims, dip::sint( 0 )));

   // Determine processing parameters
   BooleanArray process( nDims, false );
   UnsignedArray uWrap( nDims, 0 );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      dip::sint w = wrap[ ii ] % static_cast< dip::sint >( in.Size( ii ));
      if( w < 0 ) {
         w += static_cast< dip::sint >( in.Size( ii ));
      }
      process[ ii ] = w != 0;
      uWrap[ ii ] = static_cast< dip::uint >( w );
   }

   // Find line filter
   std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
   DIP_OVL_NEW_ALL( lineFilter, WrapLineFilter, ( uWrap ), in.DataType() );

   // Call line filter through framework
   Framework::Separable( in, out, in.DataType(), in.DataType(), process, {}, {}, *lineFilter,
                         Framework::SeparableOption::AsScalarImage );
}

void RotationMatrix2D( Image& out, dfloat angle ) {
   out.ReForge( {}, 4, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.ReshapeTensor( 2, 2 );
   dfloat cosA = std::cos( angle );
   dfloat sinA = std::sin( angle );
   // Note! column-major ordering, meaning the display here is transposed wrt what we actually build
   out.Fill( {  cosA, sinA,
                -sinA, cosA } );
}

void RotationMatrix3D( Image& out, dfloat alpha, dfloat beta, dfloat gamma ) {
   out.ReForge( {}, 9, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.ReshapeTensor( 3, 3 );
   dfloat cosA = std::cos( gamma );
   dfloat sinA = std::sin( gamma );
   // Note! column-major ordering, meaning the display here is transposed wrt what we actually build
   out.Fill( {  cosA, sinA, 0.0,
               -sinA, cosA, 0.0,
                0.0,  0.0,  1.0 } );
   // Now we have: out = Q_gamma
   Image tmp( {}, 9, DT_SFLOAT );
   tmp.ReshapeTensor( 3, 3 );
   cosA = std::cos( beta );
   sinA = std::sin( beta );
   // Note! column-major ordering, meaning the display here is transposed wrt what we actually build
   tmp.Fill( { cosA, 0.0, -sinA,
               0.0,  1.0,  0.0,
               sinA, 0.0,  cosA } );
   out *= tmp; // Now we have: out = Q_gamma * Q_beta
   cosA = std::cos( alpha );
   sinA = std::sin( alpha );
   // Note! column-major ordering, meaning the display here is transposed wrt what we actually build
   tmp.Fill( {  cosA, sinA, 0.0,
               -sinA, cosA, 0.0,
                0.0,  0.0,  1.0 } );
   out *= tmp; // Now we have: out = Q_gamma * Q_beta * Q_alpha
}

void RotationMatrix3D( Image& out, FloatArray const& vector, dfloat angle ) {
   DIP_THROW_IF( vector.size() != 3, E::ARRAY_PARAMETER_WRONG_LENGTH );
   out.ReForge( {}, 9, DT_SFLOAT, Option::AcceptDataTypeChange::DO_ALLOW );
   out.ReshapeTensor( 3, 3 );
   dfloat x = vector[ 0 ];
   dfloat y = vector[ 1 ];
   dfloat z = vector[ 2 ];
   dfloat norm = std::sqrt( x * x + y * y + z * z );
   x /= norm;
   y /= norm;
   z /= norm;
   dfloat cosA = std::cos( angle );
   dfloat sinA = std::sin( angle );
   dfloat ICosA = 1.0 - cosA;
   // Note! column-major ordering, meaning the display here is transposed wrt what we actually build
   out.Fill( { x * x * ICosA +     cosA, x * y * ICosA + z * sinA, x * z * ICosA - y * sinA,
               x * y * ICosA - z * sinA, y * y * ICosA +     cosA, y * z * ICosA + x * sinA,
               x * z * ICosA + y * sinA, y * z * ICosA - x * sinA, z * z * ICosA +     cosA } );
}

} // namespace dip
