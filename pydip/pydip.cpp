#undef DIP__ENABLE_DOCTEST
#include "diplib.h"

#include <pybind11/pybind11.h>
#include <pybind11/complex.h>
#include <pybind11/stl.h>
#include <pybind11/operators.h>
using namespace pybind11::literals;
namespace py = pybind11;

namespace pybind11 {
namespace detail {
template< typename Type >
struct type_caster< dip::DimensionArray< Type >> : list_caster< dip::DimensionArray< Type >, Type > {};
}}

namespace {
// A deleter that doesn't delete.
void VoidStripHandler( void const* p ) {};
}

static_assert( sizeof( bool ) == sizeof( dip::bin ), "bool is not one byte, how can I work with logical Python buffers?" );

PYBIND11_PLUGIN( PyDIP ) {
   py::module m( "PyDIP", "DIPlib module" );

   py::class_< dip::DataType >( m, "DataType" )
         .def( py::init<>() )
         .def( py::self == py::self )
         .def( "SizeOf", &dip::DataType::SizeOf )
         .def( "IsBinary", &dip::DataType::IsBinary )
         .def( "IsUInt", &dip::DataType::IsUInt )
         .def( "IsSInt", &dip::DataType::IsSInt )
         .def( "IsInteger", &dip::DataType::IsInteger )
         .def( "IsFloat", &dip::DataType::IsFloat )
         .def( "IsReal", &dip::DataType::IsReal )
         .def( "IsComplex", &dip::DataType::IsComplex )
         .def( "IsUnsigned", &dip::DataType::IsUnsigned )
         .def( "IsSigned", &dip::DataType::IsSigned )
         .def( "Real", &dip::DataType::Real )
         .def( "__repr__", []( dip::DataType const& a ) { return std::string( "PyDIP.DT_" ) + a.Name(); } )
               ;
   m.attr( "DT_BIN" ) = dip::DT_BIN;
   m.attr( "DT_UINT8" ) = dip::DT_UINT8;
   m.attr( "DT_SINT8" ) = dip::DT_SINT8;
   m.attr( "DT_UINT16" ) = dip::DT_UINT16;
   m.attr( "DT_SINT16" ) = dip::DT_SINT16;
   m.attr( "DT_UINT32" ) = dip::DT_UINT32;
   m.attr( "DT_SINT32" ) = dip::DT_SINT32;
   m.attr( "DT_SFLOAT" ) = dip::DT_SFLOAT;
   m.attr( "DT_DFLOAT" ) = dip::DT_DFLOAT;
   m.attr( "DT_SCOMPLEX" ) = dip::DT_SCOMPLEX;
   m.attr( "DT_DCOMPLEX" ) = dip::DT_DCOMPLEX;

   py::class_< dip::Image >( m, "Image", py::buffer_protocol())
         // Constructor
         .def( py::init< dip::UnsignedArray const&, dip::uint, dip::DataType >(),
               "sizes"_a, "tensorElems"_a = 1, "dt"_a = dip::DT_SFLOAT )
         // Constructor that takes a Python raw buffer
         .def( "__init__", []( dip::Image& img, py::buffer buf ) {
            py::buffer_info info = buf.request();
            // Data type
            dip::DataType datatype;
            if( info.format == py::format_descriptor< bool >::format() ) {
               datatype = dip::DT_BIN;
            } else if( info.format == py::format_descriptor< dip::uint8 >::format() ) {
               datatype = dip::DT_UINT8;
            } else if( info.format == py::format_descriptor< dip::uint16 >::format() ) {
               datatype = dip::DT_UINT16;
            } else if( info.format == py::format_descriptor< dip::uint32 >::format() ) {
               datatype = dip::DT_UINT32;
            } else if( info.format == py::format_descriptor< dip::sint8 >::format() ) {
               datatype = dip::DT_SINT8;
            } else if( info.format == py::format_descriptor< dip::sint16 >::format() ) {
               datatype = dip::DT_SINT16;
            } else if( info.format == py::format_descriptor< dip::sint32 >::format() ) {
               datatype = dip::DT_SINT32;
            } else if( info.format == py::format_descriptor< dip::sfloat >::format() ) {
               datatype = dip::DT_SFLOAT;
            } else if( info.format == py::format_descriptor< dip::dfloat >::format() ) {
               datatype = dip::DT_DFLOAT;
            } else if( info.format == py::format_descriptor< dip::scomplex >::format() ) {
               datatype = dip::DT_SCOMPLEX;
            } else if( info.format == py::format_descriptor< dip::dcomplex >::format() ) {
               datatype = dip::DT_DCOMPLEX;
            } else {
               DIP_THROW( "Image data is not numeric" );
            }
            // Sizes
            DIP_ASSERT( info.ndim == info.shape.size() );
            dip::UnsignedArray sizes( info.ndim, 1 );
            for( dip::uint ii = 0; ii < info.ndim; ++ii ) {
               sizes[ ii ] = info.shape[ ii ];
            }
            // Strides
            dip::IntegerArray strides( info.ndim, 1 );
            if( info.strides.size() == info.ndim ) { // Should be always true
               for( dip::uint ii = 0; ii < info.ndim; ++ii ) {
                  dip::uint s = info.strides[ ii ] / info.itemsize;
                  DIP_THROW_IF( s * info.itemsize != info.strides[ ii ], "Cannot create image out of an array where strides are not in whole pixels" );
                  strides[ ii ] = s;
               }
            } else {
               // Assume contiguous layout -- this should not be necessary here
               dip::uint s = 1;
               for( dip::uint ii = 0; ii < info.ndim; ++ii ) {
                  strides[ ii ] = s;
                  s *= sizes[ ii ];
               }
            }
            new( &img ) dip::Image( std::shared_ptr< void >( info.ptr, VoidStripHandler ),
                                    datatype, sizes, strides, {}, 1, nullptr );
         } )
         // Export a Python raw buffer
         .def_buffer( []( dip::Image& img ) -> py::buffer_info {
            py::buffer_info info;
            info.ptr = img.Origin();
            info.itemsize = img.DataType().SizeOf();
            switch( img.DataType()) {
               case dip::DT_BIN:
                  info.format = py::format_descriptor< bool >::format();
                  break;
               case dip::DT_UINT8:
                  info.format = py::format_descriptor< dip::uint8 >::format();
                  break;
               case dip::DT_UINT16:
                  info.format = py::format_descriptor< dip::uint16 >::format();
                  break;
               case dip::DT_UINT32:
                  info.format = py::format_descriptor< dip::uint32 >::format();
                  break;
               case dip::DT_SINT8:
                  info.format = py::format_descriptor< dip::sint8 >::format();
                  break;
               case dip::DT_SINT16:
                  info.format = py::format_descriptor< dip::sint16 >::format();
                  break;
               case dip::DT_SINT32:
                  info.format = py::format_descriptor< dip::sint32 >::format();
                  break;
               case dip::DT_SFLOAT:
                  info.format = py::format_descriptor< dip::sfloat >::format();
                  break;
               case dip::DT_DFLOAT:
                  info.format = py::format_descriptor< dip::dfloat >::format();
                  break;
               case dip::DT_SCOMPLEX:
                  info.format = py::format_descriptor< dip::scomplex >::format();
                  break;
               case dip::DT_DCOMPLEX:
                  info.format = py::format_descriptor< dip::dcomplex >::format();
                  break;
               default:
                  DIP_THROW( "Image of unknown type" ); // should never happen
            }
            info.ndim = img.Dimensionality();
            info.shape.resize( info.ndim );
            info.strides.resize( info.ndim );
            for( dip::uint ii = 0; ii < info.ndim; ++ii ) {
               info.shape[ ii ] = img.Size( ii );
               DIP_THROW_IF( img.Stride( ii ) < 0, "pybind11 does not yet support negative strides" );
               info.strides[ ii ] = (size_t)img.Stride( ii );
            }
            return info;
         } )
         // Basic properties
         .def( "__repr__", []( dip::Image const& a ) { std::ostringstream os; os << a; return os.str(); } )
         .def( "IsEmpty", []( dip::Image const& a ) { return !a.IsForged(); } )
         .def( "Dimensionality", &dip::Image::Dimensionality )
         .def( "Sizes", &dip::Image::Sizes )
         .def( "Size", &dip::Image::Size, "dim"_a )
         .def( "NumberOfPixels", &dip::Image::NumberOfPixels )
         .def( "NumberOfSamples", &dip::Image::NumberOfSamples )
         .def( "Strides", &dip::Image::Strides )
         .def( "Stride", &dip::Image::Stride, "dim"_a )
         .def( "TensorStride", &dip::Image::TensorStride )
         .def( "HasContiguousData", &dip::Image::HasContiguousData )
         .def( "HasNormalStrides", &dip::Image::HasNormalStrides )
         .def( "IsSingletonExpanded", &dip::Image::IsSingletonExpanded )
         .def( "HasSimpleStride", &dip::Image::HasSimpleStride )
          //.def( "GetSimpleStrideAndOrigin", &dip::Image::GetSimpleStrideAndOrigin, "stride"_a, "origin"_a ) // both output parameters...
         .def( "HasSameDimensionOrder", &dip::Image::HasSameDimensionOrder, "other"_a )
         .def( "TensorSizes", &dip::Image::TensorSizes )
         .def( "TensorElements", &dip::Image::TensorElements )
         .def( "TensorColumns", &dip::Image::TensorColumns )
         .def( "TensorRows", &dip::Image::TensorRows )
         .def( "TensorShape", &dip::Image::TensorShape )
         .def( "Tensor", &dip::Image::Tensor )
         .def( "IsScalar", &dip::Image::IsScalar )
         .def( "IsVector", &dip::Image::IsVector )
         .def( "IsSquare", &dip::Image::IsSquare )
         .def( "DataType", &dip::Image::DataType )
         .def( "ColorSpace", &dip::Image::ColorSpace )
         .def( "IsColor", &dip::Image::IsColor )
         .def( "SetColorSpace", &dip::Image::SetColorSpace, "colorSpace"_a )
         .def( "ResetColorSpace", &dip::Image::ResetColorSpace )
         .def( "PixelSize", py::overload_cast<>( &dip::Image::PixelSize ))
         .def( "PixelSize", py::overload_cast<>( &dip::Image::PixelSize, py::const_ ))
         .def( "PixelSize", py::overload_cast< dip::uint >( &dip::Image::PixelSize, py::const_ ), "dim"_a )
         .def( "SetPixelSize", &dip::Image::SetPixelSize, "pixelSize"_a )
         .def( "HasPixelSize", &dip::Image::HasPixelSize )
         .def( "IsIsotropic", &dip::Image::IsIsotropic )
         .def( "PixelsToPhysical", &dip::Image::PixelsToPhysical, "array"_a )
         .def( "PhysicalToPixels", &dip::Image::PhysicalToPixels, "array"_a )
         // About the data segment
         .def( "IsShared", &dip::Image::IsShared )
         .def( "ShareCount", &dip::Image::ShareCount )
         .def( "SharesData", &dip::Image::SharesData, "other"_a )
         .def( "Aliases", &dip::Image::Aliases, "other"_a )
         .def( "IsIdenticalView", &dip::Image::IsIdenticalView, "other"_a )
         .def( "IsOverlappingView", py::overload_cast< dip::Image const& >( &dip::Image::IsOverlappingView, py::const_ ),
                                    "other"_a )
         .def( "Protect", &dip::Image::Protect, "set"_a = true )
         .def( "IsProtected", &dip::Image::IsProtected )
         // Modify image without copying pixel data
         .def( "PermuteDimensions", &dip::Image::PermuteDimensions, "order"_a )
         .def( "SwapDimensions", &dip::Image::SwapDimensions, "dim1"_a, "dim2"_a )
         .def( "Flatten", &dip::Image::Flatten )
         .def( "Squeeze", &dip::Image::Squeeze )
         .def( "AddSingleton", &dip::Image::AddSingleton, "dim"_a )
         .def( "ExpandDimensionality", &dip::Image::ExpandDimensionality, "dim"_a )
         .def( "ExpandSingletonDimension", &dip::Image::ExpandSingletonDimension, "dim"_a, "newSize"_a )
         .def( "ExpandSingletonDimensions", &dip::Image::ExpandSingletonDimensions, "newSizes"_a )
         .def( "UnexpandSingletonDimensions", &dip::Image::UnexpandSingletonDimensions)
         .def( "IsSingletonExpansionPossible", &dip::Image::IsSingletonExpansionPossible, "newSizes"_a )
         .def( "ExpandSingletonTensor", &dip::Image::ExpandSingletonTensor, "size"_a )
         .def( "Mirror", &dip::Image::Mirror, "process"_a )
         .def( "ReshapeTensor", py::overload_cast< dip::uint, dip::uint>( &dip::Image::ReshapeTensor ), "rows"_a,
                                "cols"_a )
         .def( "ReshapeTensorAsVector", &dip::Image::ReshapeTensorAsVector )
         .def( "ReshapeTensorAsDiagonal", &dip::Image::ReshapeTensorAsDiagonal )
         .def( "Transpose", &dip::Image::Transpose )
         .def( "TensorToSpatial", &dip::Image::TensorToSpatial, "dim"_a = -1 )
         .def( "SpatialToTensor", &dip::Image::SpatialToTensor, "dim"_a = -1, "rows"_a = 0, "cols"_a = 0 )
         .def( "SplitComplex", &dip::Image::SplitComplex, "dim"_a = -1 )
         .def( "MergeComplex", &dip::Image::MergeComplex, "dim"_a = -1 )
         .def( "SplitComplexToTensor", &dip::Image::SplitComplexToTensor )
         .def( "MergeTensorToComplex", &dip::Image::MergeTensorToComplex )
         // Create a new image with a view of another image
         .def( "Diagonal", &dip::Image::Diagonal )
         .def( "TensorRow", &dip::Image::TensorRow, "index"_a )
         .def( "TensorColumn", &dip::Image::TensorColumn, "index"_a )
         .def( "Crop", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::Image::Crop, py::const_ ),
                       "sizes"_a, "cropLocation"_a = "center" )
         .def( "Real", &dip::Image::Real )
         .def( "Imaginary", &dip::Image::Imaginary )
         .def( "QuickCopy", &dip::Image::QuickCopy )
               // TODO: indexing: [], At(), etc.
         // Copy or write data
         .def( "Pad", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::Image::Pad, py::const_ ),
                      "sizes"_a, "cropLocation"_a = "center" )
         .def( "Copy", &dip::Image::Copy, "src"_a )
         .def( "Convert", &dip::Image::Convert, "dataType"_a )
         .def( "ExpandTensor", &dip::Image::ExpandTensor )
         .def( "Fill", py::overload_cast< dip::dcomplex >( &dip::Image::Fill ), "value"_a )
         .def( "Fill", py::overload_cast< dip::dfloat >( &dip::Image::Fill ), "value"_a )
         .def( "Fill", py::overload_cast< dip::sint >( &dip::Image::Fill ), "value"_a )
         .def( "Fill", py::overload_cast< dip::uint >( &dip::Image::Fill ), "value"_a )
         .def( "Fill", py::overload_cast< int >( &dip::Image::Fill ), "value"_a )
         .def( "Fill", py::overload_cast< bool >( &dip::Image::Fill ), "value"_a )
               // TODO: CopyAt()
         // Operators
         .def( py::self += py::self )
         .def( py::self += float() )
         .def( py::self + py::self )
         .def( py::self + float() )
         .def( py::self -= py::self )
         .def( py::self -= float() )
         .def( py::self - py::self )
         .def( py::self - float() )
         .def( py::self *= py::self )
         .def( py::self *= float() )
         .def( py::self * py::self )
         .def( py::self * float() )
         .def( py::self /= py::self )
         .def( py::self /= float() )
         .def( py::self / py::self )
         .def( py::self / float() )
         .def( py::self %= py::self )
         .def( py::self %= float() )
         .def( py::self % py::self )
         .def( py::self % float() )
         .def( "__pow__", []( dip::Image const& a, dip::Image const& b ) { return dip::Power( a, b ); }, py::is_operator() )
         .def( "__pow__", []( dip::Image const& a, float b ) { return dip::Power( a, b ); }, py::is_operator() )
         .def( "__ipow__", []( dip::Image& a, dip::Image const& b ) { dip::Power( a, b, a ); return b; }, py::is_operator() )
         .def( "__ipow__", []( dip::Image& a, float b ) { dip::Power( a, b, a ); return b; }, py::is_operator() )
         .def( py::self == py::self )
         .def( py::self == float() )
         .def( py::self != py::self )
         .def( py::self != float() )
         .def( py::self > py::self )
         .def( py::self > float() )
         .def( py::self >= py::self )
         .def( py::self >= float() )
         .def( py::self < py::self )
         .def( py::self < float() )
         .def( py::self <= py::self )
         .def( py::self <= float() )
         .def( py::self & py::self )
         .def( py::self & int() )
         .def( py::self | py::self )
         .def( py::self | int() )
         .def( py::self ^ py::self )
         .def( py::self ^ int() )
         .def( !py::self )
         .def( -py::self )
         .def( ~py::self )
               ;

   return m.ptr();
}
