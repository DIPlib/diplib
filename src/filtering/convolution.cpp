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

enum class FilterSymmetry {
      GENERAL,
      EVEN,
      ODD,
      D_EVEN,
      D_ODD
};

struct InternOneDimensionalFilter {
   FloatArray const& filter;
   dip::uint size = 0;
   dip::uint origin = 0;
   FilterSymmetry symmetry = FilterSymmetry::GENERAL;
   InternOneDimensionalFilter( OneDimensionalFilter const& in ) : filter( in.filter ) {
      size = filter.size();
      if( size != 0 ) {
         if( in.symmetry.empty() || ( in.symmetry == "general" ) ) {
            symmetry = FilterSymmetry::GENERAL;
         } else if( in.symmetry == "even" ) {
            symmetry = FilterSymmetry::EVEN;
            size += size - 1;
         } else if( in.symmetry == "odd" ) {
            symmetry = FilterSymmetry::ODD;
            size += size - 1;
         } else if( in.symmetry == "d-even" ) {
            symmetry = FilterSymmetry::D_EVEN;
            size += size;
         } else if( in.symmetry == "d-odd" ) {
            symmetry = FilterSymmetry::D_ODD;
            size += size;
         } else {
            DIP_THROW( "Symmetry string not recognized: " + in.symmetry );
         }
         if( in.origin < 0 ) {
            origin = size / 2;
         } else {
            origin = static_cast< dip::uint >( in.origin );
            DIP_THROW_IF( origin >= size, "Origin outside of filter" );
         }
      }
   }
};

using InternOneDimensionalFilterArray = std::vector< InternOneDimensionalFilter >;

template< typename TPI >
class SeparableConvolutionLineFilter : public Framework::SeparableLineFilter {
   public:
      SeparableConvolutionLineFilter( InternOneDimensionalFilterArray const& filter ) : filter_( filter ) {}
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint procDim = filter_.size() == 1 ? 0 : params.dimension;
         FloatArray const& filter = filter_[ procDim ].filter;
         dip::uint origin = filter_[ procDim ].origin;
         dip::uint filterSize = filter_[ procDim ].size;
         dip::uint fsh = filter.size() - 1;
         switch( filter_[ procDim ].symmetry ) {
            case FilterSymmetry::GENERAL:
               in += origin * inStride;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = 0;
                  TPI* in_t = in;
                  for( dip::uint jj = 0; jj < filterSize; ++jj ) {
                     sum += static_cast< FloatType< TPI >>( filter[ jj ] ) * *in_t;
                     in_t -= inStride;
                  }
                  *out = sum;
                  in += inStride;
                  out += outStride;
               }
               break;
            case FilterSymmetry::EVEN: // Always an odd-sized filter
               in += ( origin - fsh ) * inStride;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI* in_r = in;
                  TPI sum = static_cast< FloatType< TPI >>( filter[ fsh ] ) * *in_r;
                  TPI* in_l = in_r - inStride;
                  in_r += inStride;
                  dip::uint jj = fsh;
                  while( jj > 0 ) {
                     --jj;
                     sum += static_cast< FloatType< TPI >>( filter[ jj ] ) * ( *in_r + *in_l );
                     in_l -= inStride;
                     in_r += inStride;
                  }
                  *out = sum;
                  in += inStride;
                  out += outStride;
               }
               break;
            case FilterSymmetry::ODD: // Always an odd-sized filter
               in += ( origin - fsh ) * inStride;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI* in_r = in;
                  TPI sum = static_cast< FloatType< TPI >>( filter[ fsh ] ) * *in_r;
                  TPI* in_l = in_r - inStride;
                  in_r += inStride;
                  dip::uint jj = fsh;
                  while( jj > 0 ) {
                     --jj;
                     sum += static_cast< FloatType< TPI >>( filter[ jj ] ) * ( *in_r - *in_l );
                     in_l -= inStride;
                     in_r += inStride;
                  }
                  *out = sum;
                  in += inStride;
                  out += outStride;
               }
               break;
            case FilterSymmetry::D_EVEN: // Always an even-sized filter
               in += ( origin - fsh ) * inStride;
               ++fsh;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI* in_r = in;
                  TPI sum = 0;
                  TPI* in_l = in_r - inStride;
                  dip::uint jj = fsh;
                  while( jj > 0 ) {
                     --jj;
                     sum += static_cast< FloatType< TPI >>( filter[ jj ] ) * ( *in_r + *in_l );
                     in_l -= inStride;
                     in_r += inStride;
                  }
                  *out = sum;
                  in += inStride;
                  out += outStride;
               }
               break;
            case FilterSymmetry::D_ODD: // Always an even-sized filter
               in += ( origin - fsh ) * inStride;
               ++fsh;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI* in_r = in;
                  TPI sum = 0;
                  TPI* in_l = in_r - inStride;
                  dip::uint jj = fsh;
                  while( jj > 0 ) {
                     --jj;
                     sum += static_cast< FloatType< TPI >>( filter[ jj ] ) * ( *in_r - *in_l );
                     in_l -= inStride;
                     in_r += inStride;
                  }
                  *out = sum;
                  in += inStride;
                  out += outStride;
               }
               break;
         }
      }
   private:
      InternOneDimensionalFilterArray const& filter_;
};

inline bool IsMeaninglessFilter( InternOneDimensionalFilter const& filter ) {
   return ( filter.size == 0 ) || (( filter.size == 1 ) && ( filter.filter[ 0 ] == 1.0 ));
}

} // namespace


void SeparableConvolution(
      Image const& in,
      Image& out,
      OneDimensionalFilterArray const& filterArray,
      StringArray const& boundaryCondition,
      BooleanArray process
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   dip::uint nDims = in.Dimensionality();
   DIP_THROW_IF( nDims < 1, E::DIMENSIONALITY_NOT_SUPPORTED );
   DIP_THROW_IF(( filterArray.size() != 1 ) && ( filterArray.size() != nDims ), E::ARRAY_ILLEGAL_SIZE );
   InternOneDimensionalFilterArray filterData;
   DIP_START_STACK_TRACE
      for( auto const& f : filterArray ) {
         filterData.emplace_back( f );
      }
   DIP_END_STACK_TRACE
   // Handle `filterArray` and create `border` array
   UnsignedArray border( nDims );
   if( filterData.size() == 1 ) {
      dip::uint sz = filterData[ 0 ].size;
      dip::uint b = filterData[ 0 ].origin;
      b = std::max( b, sz - b - 1 ); // note that b < sz.
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         border[ ii ] = b;
      }
   } else {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         dip::uint sz = filterData[ ii ].size;
         dip::uint b = filterData[ 0 ].origin;
         b = std::max( b, sz - b - 1 ); // note that b < sz.
         border[ ii ] = b;
      }
   }
   // Handle `process` array
   if( process.empty() ) {
      process.resize( nDims, true );
   } else {
      DIP_THROW_IF( process.size() != nDims, E::ARRAY_PARAMETER_WRONG_LENGTH );
   }
   if( filterData.size() == 1 ) {
      if( IsMeaninglessFilter( filterData[ 0 ] )) {
         // Nothing to do for this filter
         process.fill( false );
      }
   } else {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         if( IsMeaninglessFilter( filterData[ ii ] )) {
            process[ ii ] = false;
         }
      }
   }
   DIP_START_STACK_TRACE
      // handle boundary condition array (checks are made in Framework::Separable, no need to repeat them here)
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      // Get callback function
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, SeparableConvolutionLineFilter, ( filterData ), dtype );
      Framework::Separable(
            in,
            out,
            dtype,
            dtype,
            process,
            border,
            bc,
            *lineFilter,
            Framework::Separable_AsScalarImage
      );
   DIP_END_STACK_TRACE
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include <random>
#include "diplib/math.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the separable convolution") {
   dip::Image img{ dip::UnsignedArray{ 200, 50, 30 }, 1, dip::DT_UINT16 };
   {
      DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
      std::random_device rd;
      std::mt19937 gen( rd() );
      std::normal_distribution< float > normDist( 9563.0, 500.0 );
      dip::ImageIterator< dip::uint16 > it( img );
      do {
         * it = dip::clamp_cast< dip::uint16 >( normDist( gen ) );
      } while( false /* ++it */ );
   }
   // Comparing general to even
   dip::Image out1;
   dip::OneDimensionalFilterArray filterArray( 1 );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         6.0 / 49.0, 5.0 / 49.0, 4.0 / 49.0, 3.0 / 49.0, 2.0 / 49.0, 1.0 / 49.0
   };
   filterArray[ 0 ].origin = 0;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray );
   dip::Image out2;
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "even";
   dip::SeparableConvolution( img, out2, filterArray );
   DOCTEST_CHECK( dip::Count( out1 != out2 ) == 0 );

   // Comparing general to odd
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         -6.0 / 49.0, -5.0 / 49.0, -4.0 / 49.0, -3.0 / 49.0, -2.0 / 49.0, -1.0 / 49.0
   };
   filterArray[ 0 ].origin = 0;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "odd";
   dip::SeparableConvolution( img, out2, filterArray );
   DOCTEST_CHECK( dip::Count( out1 != out2 ) == 0 );

   // Comparing general to d-even
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         7.0 / 49.0, 6.0 / 49.0, 5.0 / 49.0, 4.0 / 49.0, 3.0 / 49.0, 2.0 / 49.0, 1.0 / 49.0
   };
   filterArray[ 0 ].origin = 0;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "d-even";
   dip::SeparableConvolution( img, out2, filterArray );
   DOCTEST_CHECK( dip::Count( out1 != out2 ) == 0 );

   // Comparing general to d-odd
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         -7.0 / 49.0, -6.0 / 49.0, -5.0 / 49.0, -4.0 / 49.0, -3.0 / 49.0, -2.0 / 49.0, -1.0 / 49.0
   };
   filterArray[ 0 ].origin = 0;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "d-odd";
   dip::SeparableConvolution( img, out2, filterArray );
   DOCTEST_CHECK( dip::Count( out1 != out2 ) == 0 );
}

#endif // DIP__ENABLE_DOCTEST
