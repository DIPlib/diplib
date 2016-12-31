/*
 * DIPlib 3.0
 * This file contains the definition the arithmetic operators.
 *
 * (c)2016, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <array>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/overload.h"
#include "diplib/saturated_arithmetic.h"

namespace dip {

//
template< typename TPI >
class dip__Add : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = saturated_add(
                     lhs[ jj * lhsTensorStride ],
                     rhs[ jj * rhsTensorStride ] );
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Add(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__Add, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Sub : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = saturated_sub(
                     lhs[ jj * lhsTensorStride ],
                     rhs[ jj * rhsTensorStride ] );
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Subtract(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__Sub, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Mul : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         // This function is only called for two non-scalar images.
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint row = 0; row < nRows; ++row ) {
               for( dip::uint col = 0; col < nColumns; ++col ) {
                  TPI v = 0;
                  for( dip::uint jj = 0; jj < nInner; ++jj ) {
                     v = saturated_add(
                           v, saturated_mul(
                                 lhs[ ( row + jj * nRows ) * lhsTensorStride ],
                                 rhs[ ( jj + col * nInner ) * rhsTensorStride ] ) );
                  }
                  out[ ( row + col * nRows ) * outTensorStride ] = v;
               }
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
      dip__Mul( dip::uint nRows, dip::uint nColumns, dip::uint nInner )
            : nRows( nRows ), nColumns( nColumns ), nInner( nInner ) {}
   private:
      dip::uint nRows;     // == lhs.TensorRows
      dip::uint nColumns;  // == rhs.TensorColumns
      dip::uint nInner;    // == lhs.TensorColumns == rhs.TensorRows
};

template< typename TPI >
class dip__MulSamples : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = saturated_mul(
                     lhs[ jj * lhsTensorStride ],
                     rhs[ jj * rhsTensorStride ] );
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Multiply(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   // TODO: if lhs == rhs, we can optimize this somehow?
   Tensor outTensor;
   Framework::ScanOptions opts;
   bool samplewise = true;
   if( lhs.IsScalar() ) {
      outTensor = rhs.Tensor();
      opts += Framework::Scan_TensorAsSpatialDim;
      //samplewise = true;
   } else if( rhs.IsScalar() ) {
      outTensor = lhs.Tensor();
      opts += Framework::Scan_TensorAsSpatialDim;
      //samplewise = true;
   } else if( lhs.TensorColumns() == rhs.TensorRows() ) {
      // TODO: a special case could be for diagonal matrices, where each row/column of the other matrix is multiplied by the same value.
      outTensor = Tensor( lhs.TensorRows(), rhs.TensorColumns() );
      opts += Framework::Scan_ExpandTensorInBuffer;
      samplewise = false;
   } else {
      DIP_THROW( "Inner tensor dimensions must match in multiplication" );
   }
   ImageConstRefArray inar{ lhs, rhs };
   ImageRefArray outar{ out };
   DataTypeArray inBufT{ dt, dt };
   DataTypeArray outBufT{ dt };
   DataTypeArray outImT{ dt };
   UnsignedArray nElem{ outTensor.Elements() };
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   if( samplewise ) {
      DIP_OVL_NEW_ALL( scanLineFilter, dip__MulSamples, (), dt );
   } else {
      DIP_OVL_NEW_ALL( scanLineFilter, dip__Mul, ( lhs.TensorRows(), rhs.TensorColumns(), lhs.TensorColumns() ), dt );
   }
   Framework::Scan( inar, outar, inBufT, outBufT, outImT, nElem, scanLineFilter.get(), opts );
   out.ReshapeTensor( outTensor );
}

void MultiplySampleWise(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__MulSamples, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Div : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = saturated_div(
                     lhs[ jj * lhsTensorStride ],
                     rhs[ jj * rhsTensorStride ] );
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Divide(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   DIP_THROW_IF( rhs.TensorElements() != 1, "Divisor must be scalar image" );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__Div, (), dt );
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Mod : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* lhs = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI const* rhs = static_cast< TPI const* >( params.inBuffer[ 1 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint lhsStride = params.inBuffer[ 0 ].stride;
         dip::sint rhsStride = params.inBuffer[ 1 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::sint lhsTensorStride = params.inBuffer[ 0 ].tensorStride;
         dip::sint rhsTensorStride = params.inBuffer[ 1 ].tensorStride;
         dip::sint outTensorStride = params.outBuffer[ 0 ].tensorStride;
         dip::uint bufferLength = params.bufferLength;
         dip::uint tensorLength = params.outBuffer[ 0 ].tensorLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            for( dip::uint jj = 0; jj < tensorLength; ++jj ) { // all 3 buffers have same number of tensor elements
               out[ jj * outTensorStride ] = static_cast< TPI >( std::fmod(
                     lhs[ jj * lhsTensorStride ],
                     rhs[ jj * rhsTensorStride ] ));
               //*out = *lhs % *rhs; // this only works for integer types.
            }
            lhs += lhsStride;
            rhs += rhsStride;
            out += outStride;
         }
      }
};

void Modulo(
      Image const& lhs,
      Image const& rhs,
      Image& out,
      DataType dt
) {
   DIP_THROW_IF( rhs.TensorElements() != 1, "Divisor must be scalar image" );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_REAL( scanLineFilter, dip__Mod, (), dt ); // NOTE: non-binary and non-complex.
   Framework::ScanOptions opts;
   Framework::ScanDyadic( lhs, rhs, out, dt, dt, scanLineFilter.get(), opts );
}

//
template< typename TPI >
class dip__Invert : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         TPI* out = static_cast< TPI* >( params.outBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            // Tensor dimension is 1 because we request `Scan_TensorAsSpatialDim`
            *out = saturated_inv( *in );
            in += inStride;
            out += outStride;
         }
      }
};

void Invert(
      Image const& in,
      Image& out
) {
   DataType dt = in.DataType();
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_ALL( scanLineFilter, dip__Invert, (), dt );
   Framework::ScanOptions opts = Framework::Scan_TensorAsSpatialDim;
   Framework::ScanMonadic( in, out, dt, dt, 1, scanLineFilter.get(), opts );
}


} // namespace dip
