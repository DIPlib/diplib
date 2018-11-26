/*
 * DIPlib 3.0
 * This file contains definitions of functions that implement the convolution.
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

#include <cstdlib>   // std::malloc, std::free

#include "diplib.h"
#include "diplib/linear.h"
#include "diplib/transform.h"
#include "diplib/framework.h"
#include "diplib/pixel_table.h"
#include "diplib/overload.h"

namespace dip {

namespace {

enum class FilterSymmetry {
      GENERAL,
      EVEN,
      ODD,
      CONJ,
      D_EVEN,
      D_ODD,
      D_CONJ
};

template< typename inT, typename outT >
void CopyReverseData( inT const* src, outT* dest, dip::uint n ) {
   for( dip::uint ii = n; ii > 0; ) {
      --ii;
      *dest = static_cast< outT >( src[ ii ] );
      ++dest;
   }
}
template<> // template specialization to avoid a compiler warning for scomplex(dfloat).
void CopyReverseData( dfloat const* src, scomplex* dest, dip::uint n ) {
   for( dip::uint ii = n; ii > 0; ) {
      --ii;
      *dest = static_cast< sfloat >( src[ ii ] );
      ++dest;
   }
}

struct InternOneDimensionalFilter {
   std::vector< uint8 > filter; // the data: pointer must be cast to correct type, see below
   dip::uint size = 0;     // the size of the filter
   dip::uint dataSize = 0; // the number of samples we store -- size can be larger if the filter is symmetric
   dip::uint origin = 0;   // filter origin, index
   bool isDouble = true;   // If this is false, filter is sfloat*, it it is true, filter is dfloat*
   bool isComplex = false; // If this is false, filter is as above. If it is true, it is scomplex* or dcomplex* instead
   FilterSymmetry symmetry = FilterSymmetry::GENERAL;

   InternOneDimensionalFilter( OneDimensionalFilter const& in, bool useDouble = true, bool useComplex = false )
         : isDouble( useDouble ), isComplex( useComplex ) {
      dataSize = size = in.filter.size();
      dip::uint sampleSize = isDouble ? sizeof( dfloat ) : sizeof( sfloat );
      if( in.isComplex ) {
         DIP_THROW_IF( dataSize & 1, "Complex filter must have an even number of values." );
         DIP_THROW_IF( !isComplex, "Found a complex filter where none was expected." );
         dataSize /= 2;
         size /= 2;
      }
      if( isComplex ) {
         sampleSize *= 2;
      }
      if( size != 0 ) {
         if( in.symmetry.empty() || ( in.symmetry == "general" )) {
            symmetry = FilterSymmetry::GENERAL;
         } else if( in.symmetry == S::EVEN ) {
            symmetry = FilterSymmetry::EVEN;
            size += size - 1;
         } else if( in.symmetry == S::ODD ) {
            symmetry = FilterSymmetry::ODD;
            size += size - 1;
         } else if( in.symmetry == S::CONJ ) {
            symmetry = isComplex ? FilterSymmetry::CONJ : FilterSymmetry::EVEN;
            size += size - 1;
         } else if( in.symmetry == "d-even" ) {
            symmetry = FilterSymmetry::D_EVEN;
            size += size;
         } else if( in.symmetry == "d-odd" ) {
            symmetry = FilterSymmetry::D_ODD;
            size += size;
         } else if( in.symmetry == "d-conj" ) {
            symmetry = isComplex ? FilterSymmetry::D_CONJ : FilterSymmetry::D_EVEN;
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
         // Allocate memory
         filter.resize( dataSize * sampleSize );
         // Copy filter and reverse it
         if( isComplex ) {
            if( isDouble ) {
               dcomplex* ptr = reinterpret_cast< dcomplex* >( filter.data() );
               if( in.isComplex ) {
                  dcomplex const* src = reinterpret_cast< dcomplex const* >( in.filter.data() );
                  CopyReverseData( src, ptr, dataSize );
               } else {
                  CopyReverseData( in.filter.data(), ptr, dataSize );
               }
            } else {
               scomplex* ptr = reinterpret_cast< scomplex* >( filter.data() );
               if( in.isComplex ) {
                  dcomplex const* src = reinterpret_cast< dcomplex const* >( in.filter.data() );
                  CopyReverseData( src, ptr, dataSize );
                  //for( dip::uint ii = dataSize; ii > 0; ) {
                  //   --ii;
                  //   *ptr = { static_cast< sfloat >( src[ ii ].real() ), static_cast< sfloat >( src[ ii ].imag() ) };
                  //   ++ptr;
                  //}
               } else {
                  CopyReverseData( in.filter.data(), ptr, dataSize );
               }
            }
         } else {
            if( isDouble ) {
               dfloat* ptr = reinterpret_cast< dfloat* >( filter.data() );
               CopyReverseData( in.filter.data(), ptr, dataSize );
            } else {
               sfloat* ptr = reinterpret_cast< sfloat* >( filter.data() );
               CopyReverseData( in.filter.data(), ptr, dataSize );
            }
         }
         // Reverse origin also
         origin = size - origin - 1;
      }
   }
   InternOneDimensionalFilter( const InternOneDimensionalFilter& ) = delete;
   InternOneDimensionalFilter& operator=( const InternOneDimensionalFilter& ) = delete;
   InternOneDimensionalFilter( InternOneDimensionalFilter&& other ) = default;
   InternOneDimensionalFilter& operator=( InternOneDimensionalFilter&& other ) = default;
};

using InternOneDimensionalFilterArray = std::vector< InternOneDimensionalFilter >;

template< typename T > struct IsComplexType { static constexpr bool value = false; };
template<> struct IsComplexType< scomplex > { static constexpr bool value = true; };
template<> struct IsComplexType< dcomplex > { static constexpr bool value = true; };

template< typename T >
T conjugate( T value ) { return value; }
template< typename T >
std::complex< T > conjugate( std::complex< T > value ) { return std::conj( value ); }

template< typename TPI, typename TPF >
class SeparableConvolutionLineFilter : public Framework::SeparableLineFilter {
   public:
      SeparableConvolutionLineFilter( InternOneDimensionalFilterArray const& filter ) : filter_( filter ) {
         // If one is single, so is the other.
         static_assert( std::is_same< RealType< TPI >, RealType< TPF >>::value, "Filter type and image type don't match" );
         // If TPF is complex, so is TPI.
         static_assert( !( IsComplexType< TPF >::value && !IsComplexType< TPI >::value ), "Complex filter applied to non-complex data" );
      }
      virtual void Filter( Framework::SeparableLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer.buffer );
         dip::uint length = params.inBuffer.length;
         DIP_ASSERT( params.inBuffer.stride == 1 );
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint procDim = 0;
         if( filter_.size() > 1 ) {
            procDim = params.dimension;
         }
         auto filter = reinterpret_cast< TPF const* >( filter_[ procDim ].filter.data() );
         DIP_ASSERT( DataType( TPF( 0 )).IsComplex() == filter_[ procDim ].isComplex );
         DIP_ASSERT(( DataType( RealType< TPF >( 0 )) == DT_DFLOAT ) == filter_[ procDim ].isDouble );
         dip::uint dataSize = filter_[ procDim ].dataSize;
         auto filterEnd = filter + dataSize;
         dip::uint origin = filter_[ procDim ].origin;
         in -= origin;
         switch( filter_[ procDim ].symmetry ) {
            case FilterSymmetry::GENERAL:
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = 0;
                  TPI const* in_t = in;
                  for( auto f = filter; f != filterEnd; ++f, ++in_t ) {
                     sum += *f * *in_t;
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
            case FilterSymmetry::EVEN: // Always an odd-sized filter
               in += dataSize - 1;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = *filter * *in;
                  TPI const* in_r = in + 1;
                  TPI const* in_l = in - 1;
                  for( auto f = filter + 1; f != filterEnd; ++f, --in_l, ++in_r ) {
                     sum += *f * ( *in_r + *in_l );
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
            case FilterSymmetry::ODD: // Always an odd-sized filter
               in += dataSize - 1;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = *filter * *in;
                  TPI const* in_r = in + 1;
                  TPI const* in_l = in - 1;
                  for( auto f = filter + 1; f != filterEnd; ++f, --in_l, ++in_r ) {
                     sum += *f * ( *in_r - *in_l );
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
            case FilterSymmetry::CONJ: // Always an odd-sized filter
               in += dataSize - 1;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = *filter * *in;
                  TPI const* in_r = in + 1;
                  TPI const* in_l = in - 1;
                  for( auto f = filter + 1; f != filterEnd; ++f, --in_l, ++in_r ) {
                     sum += *f * *in_r + conjugate( *f ) * *in_l; // hopefully the compiler will optimize this computation...
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
            case FilterSymmetry::D_EVEN: // Always an even-sized filter
               in += dataSize - 1;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = 0;
                  TPI const* in_r = in;
                  TPI const* in_l = in_r - 1;
                  for( auto f = filter; f != filterEnd; ++f, --in_l, ++in_r ) {
                     sum += *f * ( *in_r + *in_l );
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
            case FilterSymmetry::D_ODD: // Always an even-sized filter
               in += dataSize - 1;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = 0;
                  TPI const* in_r = in;
                  TPI const* in_l = in_r - 1;
                  for( auto f = filter; f != filterEnd; ++f, --in_l, ++in_r ) {
                     sum += *f * ( *in_r - *in_l );
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
            case FilterSymmetry::D_CONJ: // Always an even-sized filter
               in += dataSize - 1;
               for( dip::uint ii = 0; ii < length; ++ii ) {
                  TPI sum = 0;
                  TPI const* in_r = in;
                  TPI const* in_l = in_r - 1;
                  for( auto f = filter; f != filterEnd; ++f, --in_l, ++in_r ) {
                     sum += *f * *in_r + conjugate( *f ) * *in_l; // hopefully the compiler will optimize this computation...
                  }
                  *out = sum;
                  ++in;
                  out += outStride;
               }
               break;
         }
      }
   private:
      InternOneDimensionalFilterArray const& filter_;
};

inline bool IsMeaninglessFilter( InternOneDimensionalFilter const& filter ) {
   return ( filter.size == 0 ) || (( filter.size == 1 ) && (
         filter.isComplex
         ? ( filter.isDouble
             ? ( *reinterpret_cast< dcomplex const* >( filter.filter.data() ) == 1.0 )
             : ( *reinterpret_cast< scomplex const* >( filter.filter.data() ) == 1.0f ))
         : ( filter.isDouble
             ? ( *reinterpret_cast< dfloat const* >( filter.filter.data() ) == 1.0 )
             : ( *reinterpret_cast< sfloat const* >( filter.filter.data() ) == 1.0f ))
   ));
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
   DIP_THROW_IF(( filterArray.size() != 1 ) && ( filterArray.size() != nDims ), E::ARRAY_PARAMETER_WRONG_LENGTH );

   // Is it a complex or a real filter?
   bool isComplexFilter = false;
   for( auto const& f : filterArray ) {
      if( f.isComplex ) {
         isComplexFilter = true;
         break;
      }
   }

   // What is the data type we'll use?
   DataType dtype = isComplexFilter ? DataType::SuggestComplex( in.DataType() ) : DataType::SuggestFlex( in.DataType() );
   //std::cout << "dtype = " << dtype << std::endl;
   bool useDouble = dtype.IsA( DataType::Class_DComplex + DataType::Class_DFloat );
   //std::cout << "useDouble = " << useDouble << std::endl;

   // Copy filter data over to internal representation, using the correct types
   InternOneDimensionalFilterArray filterData;
   DIP_START_STACK_TRACE
      for( auto const& f : filterArray ) {
         filterData.emplace_back( f, useDouble, isComplexFilter );
      }
   DIP_END_STACK_TRACE

   // Create `border` array
   UnsignedArray border( nDims );
   if( filterData.size() == 1 ) {
      dip::uint sz = filterData[ 0 ].size;
      dip::uint b = filterData[ 0 ].origin;
      b = std::max( b, sz - b - 1 ); // note that b < sz.
      border.fill( b );
   } else {
      for( dip::uint ii = 0; ii < nDims; ++ii ) {
         dip::uint sz = filterData[ ii ].size;
         dip::uint b = filterData[ ii ].origin;
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
         if(( in.Size( ii ) <= 1 ) || IsMeaninglessFilter( filterData[ ii ] )) {
            process[ ii ] = false;
         }
      }
   }

   DIP_START_STACK_TRACE
      // handle boundary condition array (checks are made in Framework::Separable, no need to repeat them here)
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      // Get callback function
      std::unique_ptr< Framework::SeparableLineFilter > lineFilter;
      switch( dtype ) {
         case dip::DT_SFLOAT:
            lineFilter = static_cast< decltype( lineFilter ) >( new SeparableConvolutionLineFilter< dip::sfloat, dip::sfloat >( filterData ));
            break;
         case dip::DT_DFLOAT:
            lineFilter = static_cast< decltype( lineFilter ) >( new SeparableConvolutionLineFilter< dip::dfloat, dip::dfloat >( filterData ));
            break;
         case dip::DT_SCOMPLEX:
            if( isComplexFilter ) {
               lineFilter = static_cast< decltype( lineFilter ) >( new SeparableConvolutionLineFilter< dip::scomplex, dip::scomplex >( filterData ));
            } else {
               lineFilter = static_cast< decltype( lineFilter ) >( new SeparableConvolutionLineFilter< dip::scomplex, dip::sfloat >( filterData ));
            }
            break;
         case dip::DT_DCOMPLEX:
            if( isComplexFilter ) {
               lineFilter = static_cast< decltype( lineFilter ) >( new SeparableConvolutionLineFilter< dip::dcomplex, dip::dcomplex >( filterData ));
            } else {
               lineFilter = static_cast< decltype( lineFilter ) >( new SeparableConvolutionLineFilter< dip::dcomplex, dip::dfloat >( filterData ));
            }
            break;
         default:
            DIP_THROW( dip::E::DATA_TYPE_NOT_SUPPORTED ); // This will never happen
      }
      Framework::Separable( in, out, dtype, dtype, process, border, bc, *lineFilter, Framework::SeparableOption::AsScalarImage );
   DIP_END_STACK_TRACE
}


void ConvolveFT(
      Image const& in,
      Image const& filter,
      Image& out,
      String const& inRepresentation,
      String const& filterRepresentation,
      String const& outRepresentation
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !filter.IsForged(), E::IMAGE_NOT_FORGED );
   bool inSpatial;
   DIP_STACK_TRACE_THIS( inSpatial = BooleanFromString( inRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool filterSpatial;
   DIP_STACK_TRACE_THIS( filterSpatial = BooleanFromString( filterRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool outSpatial;
   DIP_STACK_TRACE_THIS( outSpatial = BooleanFromString( outRepresentation, S::SPATIAL, S::FREQUENCY ));
   bool real = true;
   Image inFT;
   if( inSpatial ) {
      real &= in.DataType().IsReal();
      DIP_STACK_TRACE_THIS( FourierTransform( in, inFT ));
   } else {
      real = false;
      inFT = in.QuickCopy();
   }
   Image filterFT = filter.QuickCopy();
   if( filterFT.Dimensionality() < in.Dimensionality() ) {
      filterFT.ExpandDimensionality( in.Dimensionality() );
   }
   DIP_THROW_IF( !( filterFT.Sizes() <= in.Sizes() ), E::SIZES_DONT_MATCH ); // Also throws if dimensionalities don't match
   filterFT = filterFT.Pad( in.Sizes() );
   if( filterSpatial ) {
      real &= filterFT.DataType().IsReal();
      DIP_STACK_TRACE_THIS( FourierTransform( filterFT, filterFT ));
   } else {
      real = false;
   }
   DataType dt = inFT.DataType();
   DIP_STACK_TRACE_THIS( MultiplySampleWise( inFT, filterFT, out, dt ));
   if( outSpatial ) {
      StringSet options{ S::INVERSE };
      if( real ) {
         options.insert( S::REAL );
      }
      DIP_STACK_TRACE_THIS( FourierTransform( out, out, options ));
   }
}


namespace {

template< typename TPI >
class GeneralConvolutionLineFilter : public Framework::FullLineFilter {
   public:
      virtual void SetNumberOfThreads( dip::uint, PixelTableOffsets const& pixelTable ) override {
         offsets_ = pixelTable.Offsets();
      }
      virtual void Filter( Framework::FullLineFilterParameters const& params ) override {
         TPI* in = static_cast< TPI* >( params.inBuffer.buffer );
         dip::sint inStride = params.inBuffer.stride;
         TPI* out = static_cast< TPI* >( params.outBuffer.buffer );
         dip::sint outStride = params.outBuffer.stride;
         dip::uint length = params.bufferLength;
         PixelTableOffsets const& pixelTable = params.pixelTable;
         std::vector< dfloat > const& weights = pixelTable.Weights();
         for( dip::uint ii = 0; ii < length; ++ii ) {
            TPI sum = 0;
            auto ito = offsets_.begin();
            auto itw = weights.begin();
            while( ito != offsets_.end() ) {
               sum += in[ *ito ] * static_cast< FloatType< TPI >>( *itw );
               ++ito;
               ++itw;
            }
            *out = sum;
            in += inStride;
            out += outStride;
         }
      }
   private:
      std::vector< dip::sint > offsets_;
};

} // namespace

void GeneralConvolution(
      Image const& in,
      Image const& c_filter,
      Image& out,
      StringArray const& boundaryCondition
) {
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !c_filter.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_START_STACK_TRACE
      Kernel filter{ c_filter };
      filter.Mirror();
      if( c_filter.DataType().IsBinary() ) {
         // For binary filters, apply a uniform filter.
         Uniform( in, out, filter, boundaryCondition );
         return;
      }
      BoundaryConditionArray bc = StringArrayToBoundaryConditionArray( boundaryCondition );
      DataType dtype = DataType::SuggestFlex( in.DataType() );
      std::unique_ptr< Framework::FullLineFilter > lineFilter;
      DIP_OVL_NEW_FLEX( lineFilter, GeneralConvolutionLineFilter, (), dtype );
      Framework::Full( in, out, dtype, dtype, dtype, 1, bc, filter, *lineFilter, Framework::FullOption::AsScalarImage );
   DIP_END_STACK_TRACE
}


} // namespace dip


#ifdef DIP__ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/statistics.h"
#include "diplib/generation.h"
#include "diplib/iterators.h"

DOCTEST_TEST_CASE("[DIPlib] testing the separable convolution") {
   dip::dfloat meanval = 9563.0;
   dip::Image img{ dip::UnsignedArray{ 80, 6, 5 }, 1, dip::DT_UINT16 };
   {
      DIP_THROW_IF( img.DataType() != dip::DT_UINT16, "Expecting 16-bit unsigned integer image" );
      img.Fill( meanval );
      dip::Random random( 0 );
      dip::GaussianNoise( img, img, random, 500.0 );
   }
   dip::Image out1;
   dip::Image out2;
   // Comparing general to even
   dip::OneDimensionalFilterArray filterArray( 3 );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         6.0 / 49.0, 5.0 / 49.0, 4.0 / 49.0, 3.0 / 49.0, 2.0 / 49.0, 1.0 / 49.0
   };
   filterArray[ 0 ].origin = -1;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray, { "periodic" } );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "even";
   dip::SeparableConvolution( img, out2, filterArray, { "periodic" } );
   DOCTEST_CHECK( dip::Mean( out1 - out2 ).As< dip::dfloat >() / meanval == doctest::Approx( 0.0 ));

   // Comparing general to odd
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         -6.0 / 49.0, -5.0 / 49.0, -4.0 / 49.0, -3.0 / 49.0, -2.0 / 49.0, -1.0 / 49.0
   };
   filterArray[ 0 ].origin = -1;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray, { "periodic" } );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "odd";
   dip::SeparableConvolution( img, out2, filterArray, { "periodic" } );
   DOCTEST_CHECK( dip::Mean( out1 - out2 ).As< dip::dfloat >() / meanval == doctest::Approx( 0.0 ));

   // Comparing general to d-even
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         7.0 / 49.0, 6.0 / 49.0, 5.0 / 49.0, 4.0 / 49.0, 3.0 / 49.0, 2.0 / 49.0, 1.0 / 49.0
   };
   filterArray[ 0 ].origin = -1;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray, { "periodic" } );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "d-even";
   dip::SeparableConvolution( img, out2, filterArray, { "periodic" } );
   DOCTEST_CHECK( dip::Mean( out1 - out2 ).As< dip::dfloat >() / meanval == doctest::Approx( 0.0 ));

   // Comparing general to d-odd
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0,
         -7.0 / 49.0, -6.0 / 49.0, -5.0 / 49.0, -4.0 / 49.0, -3.0 / 49.0, -2.0 / 49.0, -1.0 / 49.0
   };
   filterArray[ 0 ].origin = -1;
   filterArray[ 0 ].symmetry = "general";
   dip::SeparableConvolution( img, out1, filterArray, { "periodic" } );
   filterArray[ 0 ].filter = {
         1.0 / 49.0, 2.0 / 49.0, 3.0 / 49.0, 4.0 / 49.0, 5.0 / 49.0, 6.0 / 49.0, 7.0 / 49.0
   };
   filterArray[ 0 ].symmetry = "d-odd";
   dip::SeparableConvolution( img, out2, filterArray, { "periodic" } );
   DOCTEST_CHECK( dip::Mean( out1 - out2 ).As< dip::dfloat >() / meanval == doctest::Approx( 0.0 ));

   // Comparing that last filter to GeneralConvolution
   dip::Image filter{ dip::UnsignedArray{ 19, 1, 1 }, 1, dip::DT_DFLOAT };
   filter.Fill( 0 );
   filter.At( 19/2, 0, 0 ) = 1;
   dip::SeparableConvolution( filter, filter, filterArray, { "add zeros" } );
   dip::GeneralConvolution( img, filter, out2, { "periodic" } );
   DOCTEST_CHECK( dip::Mean( out1 - out2 ).As< dip::dfloat >() / meanval == doctest::Approx( 0.0 ));

   // Comparing that one again, against ConvolveFT
   // Note that we can do this because we've used "periodic" boundary condition everywhere else
   dip::ConvolveFT( img, filter, out2 );
   DOCTEST_CHECK( dip::Mean( out1 - out2 ).As< dip::dfloat >() / meanval == doctest::Approx( 0.0 ));
}

#endif // DIP__ENABLE_DOCTEST
