/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the Gaussian filter.
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
#include "diplib/linear.h"
#include "diplib/math.h"
#include "diplib/generic_iterators.h"

namespace dip {

namespace {

void GaussDispatch(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& derivativeOrder,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   // If any( sigmas < 0.8 ) || any( derivativeOrder > 3 )  ==>  FT
   // Else if any( sigmas > 10 )  ==>  IIR
   // Else ==>  FIR
   for( dip::uint ii = 0; ii < derivativeOrder.size(); ++ii ) { // We can't fold this loop in with the next one, the two arrays might be of different size
      if( derivativeOrder[ ii ] > 3 ) {
         GaussFT( in, out, sigmas, derivativeOrder, truncation ); // ignores boundaryCondition
         return;
      }
   }
   for( dip::uint ii = 0; ii < sigmas.size(); ++ii ) {
      if(( sigmas[ ii ] < 0.8 ) && ( sigmas[ ii ] > 0.0 )) {
         GaussFT( in, out, sigmas, derivativeOrder, truncation ); // ignores boundaryCondition
         return;
      }
   }
   for( dip::uint ii = 0; ii < sigmas.size(); ++ii ) {
      if( sigmas[ ii ] > 10 ) {
         GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, {}, S::DISCRETE_TIME_FIT, truncation );
         return;
      }
   }
   GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, truncation );
}

} // namespace

void Gauss(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      UnsignedArray const& derivativeOrder,
      String method,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   if( method.substr( 0, 5 ) == "gauss" ) {
      method = method.substr( 5, String::npos );
   }
   if( method == S::BEST ) {
      DIP_STACK_TRACE_THIS( GaussDispatch( in, out, sigmas, derivativeOrder, boundaryCondition, truncation ));
   } else if( ( method == "FIR" ) || ( method == "fir" ) ) {
      DIP_STACK_TRACE_THIS( GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, truncation ));
   } else if( ( method == "FT" ) || ( method == "ft" ) ) {
      DIP_STACK_TRACE_THIS( GaussFT( in, out, sigmas, derivativeOrder, truncation )); // ignores boundaryCondition
   } else if( ( method == "IIR" ) || ( method == "iir" ) ) {
      DIP_STACK_TRACE_THIS( GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, {}, S::DISCRETE_TIME_FIT, truncation ));
   } else {
      DIP_THROW( "Unknown Gauss filter method" );
   }
}

void Derivative(
      Image const& in,
      Image& out,
      UnsignedArray const& derivativeOrder,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   if( method == S::FINITEDIFF ) {
      dip::uint nDims = in.Dimensionality();
      BooleanArray process( nDims, true );
      FloatArray ss = sigmas;
      DIP_STACK_TRACE_THIS( ArrayUseParameter( ss, nDims, 1.0 ));
      // Set process to false where sigma <= 0
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if(( ss[ ii ] <= 0.0 ) || ( in.Size( ii ) == 1 )) {
            process[ ii ] = 0;
         }
      }
      DIP_STACK_TRACE_THIS( FiniteDifference( in, out, derivativeOrder, S::SMOOTH, boundaryCondition, process ));
   } else if( ( method == S::BEST ) || ( method == "gauss" ) ) {
      DIP_STACK_TRACE_THIS( GaussDispatch( in, out, sigmas, derivativeOrder, boundaryCondition, truncation ));
   } else if( ( method == "gaussFIR" ) || ( method == "gaussfir" ) ) {
      DIP_STACK_TRACE_THIS( GaussFIR( in, out, sigmas, derivativeOrder, boundaryCondition, truncation ));
   } else if( ( method == "gaussFT" ) || ( method == "gaussft" ) ) {
      DIP_STACK_TRACE_THIS( GaussFT( in, out, sigmas, derivativeOrder, truncation )); // ignores boundaryCondition
   } else if( ( method == "gaussIIR" ) || ( method == "gaussiir" ) ) {
      DIP_STACK_TRACE_THIS( GaussIIR( in, out, sigmas, derivativeOrder, boundaryCondition, {}, S::DISCRETE_TIME_FIT, truncation ));
   } else {
      DIP_THROW( "Unknown derivative method" );
   }
}

namespace {

UnsignedArray FindGradientDimensions(
      UnsignedArray const& sizes,
      FloatArray& sigmas, // adjusted to nDims
      BooleanArray process // by copy
) {
   dip::uint nDims = sizes.size();
   DIP_START_STACK_TRACE
      ArrayUseParameter( process, nDims, true );
      ArrayUseParameter( sigmas, nDims, 1.0 );
   DIP_END_STACK_TRACE
   UnsignedArray dims;
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      if( process[ ii ] && ( sizes[ ii ] > 1 ) && ( sigmas[ ii ] > 0.0 )) {
         dims.push_back( ii );
      }
   }
   return dims;
}

} // namespace

void Gradient(
      Image const& c_in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   UnsignedArray dims;
   DIP_STACK_TRACE_THIS( dims = FindGradientDimensions( c_in.Sizes(), sigmas, process ));
   dip::uint nDims = dims.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image in = c_in.QuickCopy();
   PixelSize pxsz = c_in.PixelSize();
   if( in.Aliases( out )) {
      out.Strip();
   }
   out.ReForge( in.Sizes(), nDims, DataType::SuggestFlex( in.DataType() ));
   UnsignedArray order( in.Dimensionality(), 0 );
   auto it = ImageTensorIterator( out );
   for( dip::uint ii = 0; ii < nDims; ++ii ) {
      order[ dims[ ii ]] = 1;
      DIP_STACK_TRACE_THIS( Derivative( in, *it, order, sigmas, method, boundaryCondition, truncation ));
      order[ dims[ ii ]] = 0;
      ++it;
   }
   out.SetPixelSize( pxsz );
}

void GradientMagnitude(
      Image const& c_in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray dims;
   DIP_STACK_TRACE_THIS( dims = FindGradientDimensions( c_in.Sizes(), sigmas, process ));
   dip::uint nDims = dims.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image in = c_in;
   if( in.Aliases( out ) ) {
      out.Strip();
   }
   UnsignedArray order( in.Dimensionality(), 0 );
   order[ dims[ 0 ]] = 1;
   Derivative( in, out, order, sigmas, method, boundaryCondition, truncation );
   if( nDims > 1 ) {
      MultiplySampleWise( out, out, out, out.DataType() );
      Image tmp;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         order[ dims[ ii - 1 ]] = 0;
         order[ dims[ ii ]] = 1;
         DIP_STACK_TRACE_THIS( Derivative( in, tmp, order, sigmas, method, boundaryCondition, truncation ));
         MultiplySampleWise( tmp, tmp, tmp, tmp.DataType() );
         Add( out, tmp, out, out.DataType() );
      }
      Sqrt( out, out );
   } else {
      Abs( out, out );
   }
}

void GradientDirection(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_START_STACK_TRACE
      Image tmp = Gradient( in, sigmas, method, boundaryCondition, process, truncation );
      Angle( tmp, out );
   DIP_END_STACK_TRACE
}

void Curl(
      Image const& c_in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = c_in.TensorElements();
   DIP_THROW_IF( !c_in.IsVector() || ( nDims < 2 ) || ( nDims > 3 ), E::TENSOR_NOT_2_OR_3 );
   UnsignedArray dims;
   DIP_STACK_TRACE_THIS( dims = FindGradientDimensions( c_in.Sizes(), sigmas, process ));
   DIP_THROW_IF( dims.size() != nDims, E::NTENSORELEM_DONT_MATCH );
   if( nDims == 2 ) {
      DIP_START_STACK_TRACE
         UnsignedArray order( c_in.Dimensionality(), 0 );
         order[ dims[ 1 ]] = 1;
         Image dy = Derivative( c_in[ 0 ], order, sigmas, method, boundaryCondition, truncation );
         order[ dims[ 0 ]] = 1;
         order[ dims[ 1 ]] = 0;
         Derivative( c_in[ 1 ], out, order, sigmas, method, boundaryCondition, truncation );
         out -= dy;
      DIP_END_STACK_TRACE
   } else { // nDims == 3
      DIP_START_STACK_TRACE
         Image in = c_in.QuickCopy();
         PixelSize pxsz = c_in.PixelSize();
         out.ReForge( in.Sizes(), 3, DataType::SuggestFlex( in.DataType() ));
         UnsignedArray order( in.Dimensionality(), 0 );
         Image d;

         auto it = ImageTensorIterator( out );

         order[ dims[ 1 ]] = 1;
         Derivative( in[ 2 ], *it, order, sigmas, method, boundaryCondition, truncation );
         order[ dims[ 1 ]] = 0;
         order[ dims[ 2 ]] = 1;
         Derivative( in[ 1 ], d, order, sigmas, method, boundaryCondition, truncation );
         order[ dims[ 2 ]] = 0;
         *it -= d;

         ++it;

         order[ dims[ 2 ]] = 1;
         Derivative( in[ 0 ], *it, order, sigmas, method, boundaryCondition, truncation );
         order[ dims[ 2 ]] = 0;
         order[ dims[ 0 ]] = 1;
         Derivative( in[ 2 ], d, order, sigmas, method, boundaryCondition, truncation );
         order[ dims[ 0 ]] = 0;
         *it -= d;

         ++it;

         order[ dims[ 0 ]] = 1;
         Derivative( in[ 1 ], *it, order, sigmas, method, boundaryCondition, truncation );
         order[ dims[ 0 ]] = 0;
         order[ dims[ 1 ]] = 1;
         Derivative( in[ 0 ], d, order, sigmas, method, boundaryCondition, truncation );
         *it -= d;

         out.SetPixelSize( pxsz );
      DIP_END_STACK_TRACE
   }
}

void Divergence(
      Image const& c_in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = c_in.TensorElements();
   DIP_THROW_IF( !c_in.IsVector(), E::IMAGE_NOT_VECTOR );
   UnsignedArray dims;
   DIP_STACK_TRACE_THIS( dims = FindGradientDimensions( c_in.Sizes(), sigmas, process ));
   DIP_THROW_IF( dims.size() != nDims, E::NTENSORELEM_DONT_MATCH );
   Image in = c_in.QuickCopy();
   PixelSize pxsz = c_in.PixelSize();
   if( in.Aliases( out ) ) {
      out.Strip();
   }
   UnsignedArray order( in.Dimensionality(), 0 );
   order[ dims[ 0 ]] = 1;
   auto it = ImageTensorIterator( in );
   DIP_STACK_TRACE_THIS( Derivative( *it, out, order, sigmas, method, boundaryCondition, truncation ));
   if( nDims > 1 ) {
      Image tmp;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         ++it;
         order[ dims[ ii - 1 ]] = 0;
         order[ dims[ ii ]] = 1;
         DIP_STACK_TRACE_THIS( Derivative( *it, tmp, order, sigmas, method, boundaryCondition, truncation ));
         Add( out, tmp, out, out.DataType() );
      }
   }
   out.SetPixelSize( pxsz );
}

void Hessian (
      Image const& c_in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_in.IsScalar(), E::IMAGE_NOT_SCALAR );
   UnsignedArray dims;
   DIP_STACK_TRACE_THIS( dims = FindGradientDimensions( c_in.Sizes(), sigmas, process ));
   dip::uint nDims = dims.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   Image in = c_in.QuickCopy();
   PixelSize pxsz = c_in.PixelSize();
   if( in.Aliases( out )) {
      out.Strip();
   }
   Tensor tensor( Tensor::Shape::SYMMETRIC_MATRIX, nDims, nDims );
   out.ReForge( in.Sizes(), tensor.Elements(), DataType::SuggestFlex( in.DataType() ));
   out.ReshapeTensor( tensor );
   UnsignedArray order( in.Dimensionality(), 0 );
   auto it = ImageTensorIterator( out );
   for( dip::uint ii = 0; ii < nDims; ++ii ) { // Symmetric matrix stores diagonal elements first
      order[ dims[ ii ]] = 2;
      DIP_STACK_TRACE_THIS( Derivative( in, *it, order, sigmas, method, boundaryCondition, truncation ));
      order[ dims[ ii ]] = 0;
      ++it;
   }
   for( dip::uint jj = 1; jj < nDims; ++jj ) { // Elements above diagonal stored column-wise
      for( dip::uint ii = 0; ii < jj; ++ii ) {
         order[ dims[ ii ]] = 1;
         order[ dims[ jj ]] = 1;
         DIP_STACK_TRACE_THIS( Derivative( in, *it, order, sigmas, method, boundaryCondition, truncation ));
         order[ dims[ ii ]] = 0;
         order[ dims[ jj ]] = 0;
         ++it;
      }
   }
   out.SetPixelSize( pxsz );
}

void Laplace (
      Image const& c_in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_THROW_IF( !c_in.IsForged(), E::IMAGE_NOT_FORGED );
   UnsignedArray dims;
   DIP_STACK_TRACE_THIS( dims = FindGradientDimensions( c_in.Sizes(), sigmas, process ));
   dip::uint nDims = dims.size();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   if( method == S::FINITEDIFF ) {
      UnsignedArray ksz( c_in.Dimensionality(), 1 );
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
            ksz[ dims[ ii ]] = 3;
      }
      Image kernel{ ksz, 1, DT_DFLOAT };
      kernel = -1.0;
      // Get position of central pixel
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         ksz[ ii ] /= 2; // 1/2==0, 3/2==1
      }
      kernel.At( ksz ) = static_cast< dfloat >( kernel.NumberOfPixels() ) - 1.0;
      DIP_STACK_TRACE_THIS( GeneralConvolution( c_in, kernel, out, boundaryCondition ));
   } else {
      Image in = c_in;
      if( in.Aliases( out ) ) {
         out.Strip();
      }
      UnsignedArray order( in.Dimensionality(), 0 );
      order[ dims[ 0 ]] = 2;
      DIP_STACK_TRACE_THIS( Derivative( in, out, order, sigmas, method, boundaryCondition, truncation ));
      Image tmp;
      for( dip::uint ii = 1; ii < nDims; ++ii ) {
         order[ dims[ ii - 1 ]] = 0;
         order[ dims[ ii ]] = 2;
         DIP_STACK_TRACE_THIS( Derivative( in, tmp, order, sigmas, method, boundaryCondition, truncation ));
         out += tmp;
      }
   }
}

namespace {

enum class DggFamilyVersion { Dgg, LaplacePlusDgg, LaplaceMinusDgg };

void DggFamily(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation,
      DggFamilyVersion version
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );

   Image g, H;
   DIP_STACK_TRACE_THIS( Gradient( in, g, sigmas, method, boundaryCondition, process, truncation ));
   DIP_STACK_TRACE_THIS( Hessian( in, H, sigmas, method, boundaryCondition, process, truncation ));
   DIP_ASSERT( g.TensorElements() == H.TensorRows() );

   // The easy way to compute this:
   //    out = Transpose( g ) * H * g;
   //    out /= Transpose( g ) * g;
   // But that duplicates some computations, so we write it out by hand.
   // Note that computing the elements of the gradient and Hessian as they are needed, instead of
   // computing them all upfront, would reduce temporary memory usage. But it's too attractive to
   // re-use those functions here.

   // 1. The first diagonal element, to initialize `out` and `gradSum`.
   Image gradSum = MultiplySampleWise( g[ 0 ], g[ 0 ] );
   MultiplySampleWise( gradSum, H[ 0 ], out );
   // 2. The rest of the diagonal elements
   Image tmp;
   for( dip::uint ii = 1; ii < g.TensorElements(); ++ii ) {
      MultiplySampleWise( g[ ii ], g[ ii ], tmp );
      Add( gradSum, tmp, gradSum );
      MultiplySampleWise( tmp, H[ ii ], tmp );
      Add( out, tmp, out );
   }
   // 3. The off-diagonal elements
   for( dip::uint ii = 1; ii < g.TensorElements() - 1; ++ii ) {
      for( dip::uint jj = ii + 1; jj < g.TensorElements(); ++jj ) {
         MultiplySampleWise( g[ ii ], g[ jj ], tmp );
         MultiplySampleWise( tmp, H[ UnsignedArray{ ii, jj } ], tmp );
         LinearCombination( out, tmp, out, 1.0, 2.0 );
      }
   }
   // 4. The division
   SafeDivide( out, gradSum, out );
   // 5. Compute Laplace if necessary.
   if( version == DggFamilyVersion::Dgg ) {
      return;
   }
   Trace( H, tmp );
   if( version == DggFamilyVersion::LaplaceMinusDgg ) {
      Subtract( tmp, out, out );
   } else { // DggFamilyVersion::LaplacePlusDgg
      Add( tmp, out, out );
   }
}

} // namespace

void Dgg(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_STACK_TRACE_THIS( DggFamily( in, out, sigmas, method, boundaryCondition, process, truncation, DggFamilyVersion::Dgg ));
}

void LaplacePlusDgg(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_STACK_TRACE_THIS( DggFamily( in, out, sigmas, method, boundaryCondition, process, truncation, DggFamilyVersion::LaplacePlusDgg ));
}

void LaplaceMinusDgg(
      Image const& in,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      BooleanArray const& process,
      dfloat truncation
) {
   DIP_STACK_TRACE_THIS( DggFamily( in, out, sigmas, method, boundaryCondition, process, truncation, DggFamilyVersion::LaplaceMinusDgg ));
}

void NormalizedConvolution(
      Image const& in,
      Image const& mask,
      Image& out,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged() || !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !mask.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( mask.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( mask.Sizes() != in.Sizes(), E::SIZES_DONT_MATCH );
   Image denominator;
   DIP_STACK_TRACE_THIS( Gauss( mask, denominator, sigmas, { 0 }, method, boundaryCondition, truncation ));
   DIP_STACK_TRACE_THIS( MultiplySampleWise( in, mask, out ));
   DIP_STACK_TRACE_THIS( Gauss( out, out, sigmas, { 0 }, method, boundaryCondition, truncation ));
   DIP_STACK_TRACE_THIS( SafeDivide( out, denominator, out, out.DataType() ));
}

void NormalizedDifferentialConvolution(
      Image const& in,
      Image const& mask,
      Image& out,
      dip::uint dimension,
      FloatArray const& sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged() || !mask.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !mask.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( mask.DataType().IsComplex(), E::DATA_TYPE_NOT_SUPPORTED );
   DIP_THROW_IF( mask.Sizes() != in.Sizes(), E::SIZES_DONT_MATCH );
   DataType dt = DataType::SuggestFlex( in.DataType() );
   DIP_THROW_IF( dimension >= in.Dimensionality(), E::INVALID_PARAMETER );

   // We compute here:
   //    out = SafeDivide( Derivative( a * m ), Gauss( m )) - SafeDivide( Gauss( a * m ), Gauss( m )) * SafeDivide( Derivative( m ), Gauss( m ))
   //        = SafeDivide( Derivative( a * m ) - SafeDivide( Gauss( a * m ), Gauss( m )) * Derivative( m ), Gauss( m ))

   Image denominator;
   DIP_STACK_TRACE_THIS( Gauss( mask, denominator, sigmas, { 0 }, method, boundaryCondition, truncation ));
   Image weighted;
   DIP_STACK_TRACE_THIS( MultiplySampleWise( in, mask, weighted, dt ));
   // NC = SafeDivide( Gauss( a * m ), Gauss( m ));
   Image NC;
   DIP_STACK_TRACE_THIS( Gauss( weighted, NC, sigmas, { 0 }, method, boundaryCondition, truncation ));
   SafeDivide( NC, denominator, NC, dt ); // NC.DataType() == dt
   // out = SafeDivide( Derivative( a * m ) - NC * Derivative( m ), Gauss( m ));
   UnsignedArray derivativeOrder( in.Dimensionality(), 0 );
   derivativeOrder[ dimension ] = 1;
   Image tmp;
   DIP_STACK_TRACE_THIS( Derivative( mask, tmp, derivativeOrder, sigmas, method, boundaryCondition, truncation ));
   DIP_STACK_TRACE_THIS( Derivative( weighted, out, derivativeOrder, sigmas, method, boundaryCondition, truncation ));
   weighted.Strip();
   DIP_STACK_TRACE_THIS( MultiplySampleWise( NC, tmp, NC, dt ));
   tmp.Strip();
   DIP_STACK_TRACE_THIS( Subtract( out, NC, out, dt ));
   NC.Strip();
   DIP_STACK_TRACE_THIS( SafeDivide( out, denominator, out, dt ));
}

void MeanShiftVector(
      Image const& in,
      Image& out,
      FloatArray sigmas,
      String const& method,
      StringArray const& boundaryCondition,
      dfloat truncation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
   dip::uint nDims = in.Dimensionality();
   DIP_STACK_TRACE_THIS( ArrayUseParameter( sigmas, nDims, 1.0 ));
   Image denominator;
   DIP_STACK_TRACE_THIS( Gauss( in, denominator, sigmas, { 0 }, method, boundaryCondition, truncation ));
   DIP_STACK_TRACE_THIS( Gradient( in, out, sigmas, method, boundaryCondition, {}, truncation ));
   DIP_STACK_TRACE_THIS( SafeDivide( out, denominator, out, out.DataType() ));
   for( auto& s : sigmas ) {
      s *= s;
   }
   Image normalization( sigmas, out.DataType() );
   DIP_STACK_TRACE_THIS( MultiplySampleWise( out, normalization, out, out.DataType() ));
}

} // namespace dip
