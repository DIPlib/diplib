/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the convolution.
 *
 * (c)2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include "diplib.h"
#include "diplib/linear.h"
#include "diplib/framework.h"
#include "diplib/overload.h"

namespace dip {


namespace {

// *grumble* multiplication of complex values with scalars is only defined when the types are identical
template< typename T >
static inline T multiply( dfloat const& lhs, T const& rhs ) {
   return lhs * rhs;
}
template<>
inline scomplex multiply( dfloat const& lhs, scomplex const& rhs ) {
   return static_cast< sfloat >( lhs ) * rhs;
}

struct InternOneDimensionalFilter{
   FloatArray const& filter;
   dip::uint origin;
   dip::uint symmetry;
   InternOneDimensionalFilter( OneDimensionalFilter const& in ) : filter(in.filter) {
      if( in.origin < 0 ) {
         origin = filter.size() / 2;
      } else {
         origin = static_cast< dip::uint >( in.origin );
         DIP_THROW_IF( origin >= filter.size(), "Origin outside of filter." );
      }
      if( in.symmetry.empty() || ( in.symmetry == "general" )) {
         symmetry = 0;
      } else if( in.symmetry == "even" ) {
         symmetry = 1;
      } else if( in.symmetry == "odd" ) {
         symmetry = 2;
      } else {
         DIP_THROW( "Symmetry string not recognized: " + in.symmetry );
      }
   }
};
using InternOneDimensionalFilterArray = std::vector< InternOneDimensionalFilter >;

template< typename TPI >
class SeparableConvolutionLineFilter : public Framework::SeparableLineFilter {
   public:
      SeparableConvolutionLineFilter( InternOneDimensionalFilterArray const& filter ) : filter_( filter ) {}
      virtual void Filter( Framework::SeparableLineFilterParameters& params ) {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint procDim = filter_.size() == 1 ? 1 : params.dimension;
         FloatArray const& filter = filter_[ procDim ].filter;
         dip::uint origin = filter_[ procDim ].origin;
         dip::uint filterSize = filter.size();
         dip::uint fsh = filterSize / 2;
         dip::sint fil = origin;
         dip::sint fir = origin - filterSize + 1;
         dip::sint fih = origin - fsh;
         bool odd = ( filterSize & 1 ) == 1;
         switch( filter_[ procDim ].symmetry ) {
            default:
            case 0: // "general"
               for( dip::uint ii = 0; ii < length; ++ii, ++fil ) {
                  TPI sum = 0;
                  dip::sint kk = fil;
                  for( dip::uint jj = 0; jj < filterSize; ++jj, --kk ) {
                     sum += multiply( filter[ jj ], in[ kk * inStride ] );
                  }
                  out[ ii * outStride ] = sum;
               }
               break;
            case 1: // "even"
               for( dip::uint ii = 0; ii < length; ++ii, ++fil, ++fir, ++fih ) {
                  TPI sum = 0;
                  if( odd ) {
                     sum = multiply( filter[ fsh ], in[ fih * inStride ] );
                  }
                  dip::sint kk = fil;
                  dip::sint ll = fir;
                  for( dip::uint jj = 0; jj < fsh; ++jj, --kk, ++ll ) {
                     sum += multiply( filter[ jj ], in[ kk * inStride ] + in[ ll * inStride ] );
                  }
                  out[ ii * outStride ] = sum;
               }
               break;
            case 2: // "odd"
               for( dip::uint ii = 0; ii < length; ++ii, ++fil, ++fir, ++fih ) {
                  TPI sum = 0;
                  if( odd ) {
                     sum = multiply( filter[ fsh ], in[ fih * inStride ] );
                  }
                  dip::sint kk = fil;
                  dip::sint ll = fir;
                  for( dip::uint jj = 0; jj < fsh; ++jj, --kk, ++ll ) {
                     sum += multiply( filter[ jj ], in[ kk * inStride ] - in[ ll * inStride ] );
                  }
                  out[ ii * outStride ] = sum;
               }
               break;
         }
      }
   private:
      InternOneDimensionalFilterArray const& filter_;
};

} // namespace


void SeparableConvolution(
      Image const& in,
      Image& out,
      OneDimensionalFilterArray const& filter,
      StringArray boundaryCondition,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint ndims = in.Dimensionality();
   DIP_THROW_IF( ndims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF(( filter.size() != 1 ) && ( filter.size() != ndims ), E::ARRAY_ILLEGAL_SIZE );
   DIP_START_STACK_TRACE
      InternOneDimensionalFilterArray filterData;
      for( auto const& f : filter ) {
         filterData.emplace_back( f );
      }
      UnsignedArray border( ndims );
      if( filterData.size() == 1 ) {
         dip::uint sz = filterData[ 0 ].filter.size();
         dip::uint b = static_cast< dip::uint >( filterData[ 0 ].origin );
         b = std::max( b, sz - b - 1 ); // note that b < sz.
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            border[ ii ] = b;
         }
      } else {
         for( dip::uint ii = 0; ii < ndims; ++ii ) {
            dip::uint sz = filterData[ ii ].filter.size();
            dip::uint b = static_cast< dip::uint >( filterData[ 0 ].origin );
            b = std::max( b, sz - b - 1 ); // note that b < sz.
            border[ ii ] = b;
         }
      }
      // TODO: set process = false for dimensions where filter is either {1} or size 0.
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLOAT_OR_COMPLEX( lineFilter, SeparableConvolutionLineFilter, ( filterData ), dtype );
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      Framework::Separable(
            in,
            out,
            dtype,
            dtype,
            process,
            border,
            bc,
            lineFilter.get(),
            Framework::Separable_AsScalarImage
      );
   DIP_END_STACK_TRACE
}


} // namespace dip
