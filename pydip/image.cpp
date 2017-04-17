/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017, Flagship Biosciences, Inc., written by Cris Luengo.
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

#include "pydip.h"

void init_image( py::module& m ) {
   py::class_< dip::Image >( m, "Image", py::buffer_protocol() )
         // Constructor
         .def( py::init< dip::UnsignedArray const&, dip::uint, dip::DataType >(),
               "sizes"_a, "tensorElems"_a = 1, "dt"_a = dip::DT_SFLOAT )
         // Constructor that takes a Python raw buffer
         .def( "__init__", []( dip::Image& img, py::buffer buf ) {
            py::buffer_info info = buf.request();
            /*
            std::cout << "--Constructing dip::Image from Python buffer.\n";
            std::cout << "   info.ptr = " << info.ptr << std::endl;
            std::cout << "   info.format = " << info.format << std::endl;
            std::cout << "   info.ndims = " << info.shape.size() << std::endl;
            std::cout << "   info.size = " << info.size << std::endl;
            std::cout << "   info.itemsize = " << info.itemsize << std::endl;
            std::cout << "   info.shape[0] = " << info.shape[0] << std::endl;
            std::cout << "   info.shape[1] = " << info.shape[1] << std::endl;
            std::cout << "   info.strides[0] = " << info.strides[0] << std::endl;
            std::cout << "   info.strides[1] = " << info.strides[1] << std::endl;
            */
            // Data type
            dip::DataType datatype;
            switch( info.format[ 0 ] ) {
               case py::format_descriptor< bool >::c:
                  datatype = dip::DT_BIN;
                  break;
               case py::format_descriptor< dip::uint8 >::c:
                  datatype = dip::DT_UINT8;
                  break;
               case py::format_descriptor< dip::uint16 >::c:
                  datatype = dip::DT_UINT16;
                  break;
               case py::format_descriptor< dip::uint32 >::c:
                  datatype = dip::DT_UINT32;
                  break;
               case py::format_descriptor< dip::sint8 >::c:
                  datatype = dip::DT_SINT8;
                  break;
               case py::format_descriptor< dip::sint16 >::c:
                  datatype = dip::DT_SINT16;
                  break;
               case py::format_descriptor< dip::sint32 >::c:
                  datatype = dip::DT_SINT32;
                  break;
               case py::format_descriptor< dip::sfloat >::c:
                  datatype = dip::DT_SFLOAT;
                  break;
               case py::format_descriptor< dip::dfloat >::c:
                  datatype = dip::DT_DFLOAT;
                  break;
               case py::format_descriptor< dip::scomplex >::c:
                  datatype = dip::DT_SCOMPLEX;
                  break;
               case py::format_descriptor< dip::dcomplex >::c:
                  datatype = dip::DT_DCOMPLEX;
                  break;
               default:
                  DIP_THROW( "Image data is not numeric" );
            }
            // Sizes
            dip::uint ndim = static_cast< dip::uint >( info.ndim );
            DIP_ASSERT( ndim == info.shape.size() );
            dip::UnsignedArray sizes( ndim, 1 );
            for( dip::uint ii = 0; ii < ndim; ++ii ) {
               sizes[ ii ] = static_cast< dip::uint >( info.shape[ ii ] );
            }
            // Strides
            dip::IntegerArray strides( ndim, 1 );
            for( dip::uint ii = 0; ii < ndim; ++ii ) {
               dip::sint s = info.strides[ ii ] / static_cast< dip::sint >( info.itemsize );
               DIP_THROW_IF( s * static_cast< dip::sint >( info.itemsize ) != info.strides[ ii ], "Cannot create image out of an array where strides are not in whole pixels" );
               strides[ ii ] = s;
            }
            new( &img ) dip::Image( dip::NonOwnedRefToDataSegment( info.ptr ), info.ptr, datatype, sizes, strides, {}, 1 );
         } )
         // Export a Python raw buffer
         .def_buffer( []( dip::Image& img ) -> py::buffer_info {
            dip::String format;
            switch( img.DataType()) {
               case dip::DT_BIN:
                  format = py::format_descriptor< bool >::format();
                  break;
               case dip::DT_UINT8:
                  format = py::format_descriptor< dip::uint8 >::format();
                  break;
               case dip::DT_UINT16:
                  format = py::format_descriptor< dip::uint16 >::format();
                  break;
               case dip::DT_UINT32:
                  format = py::format_descriptor< dip::uint32 >::format();
                  break;
               case dip::DT_SINT8:
                  format = py::format_descriptor< dip::sint8 >::format();
                  break;
               case dip::DT_SINT16:
                  format = py::format_descriptor< dip::sint16 >::format();
                  break;
               case dip::DT_SINT32:
                  format = py::format_descriptor< dip::sint32 >::format();
                  break;
               case dip::DT_SFLOAT:
                  format = py::format_descriptor< dip::sfloat >::format();
                  break;
               case dip::DT_DFLOAT:
                  format = py::format_descriptor< dip::dfloat >::format();
                  break;
               case dip::DT_SCOMPLEX:
                  format = py::format_descriptor< dip::scomplex >::format();
                  break;
               case dip::DT_DCOMPLEX:
                  format = py::format_descriptor< dip::dcomplex >::format();
                  break;
               default:
                  DIP_THROW( "Image of unknown type" ); // should never happen
            }
            dip::sint itemsize = static_cast< dip::sint >( img.DataType().SizeOf() );
            dip::IntegerArray strides = img.Strides();
            for( dip::sint& s : strides ) {
               s *= itemsize;
            }
            dip::UnsignedArray sizes = img.Sizes();
            if( !img.IsScalar() ) {
               sizes.push_back( img.TensorElements() );
               strides.push_back( img.TensorStride() );
            }
            py::buffer_info info{ img.Origin(), itemsize, format, static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
            /*
            std::cout << "--Constructed Python buffer for dip::Image object.\n";
            std::cout << "   info.ptr = " << info.ptr << std::endl;
            std::cout << "   info.format = " << info.format << std::endl;
            std::cout << "   info.ndims = " << info.ndims << std::endl;
            std::cout << "   info.size = " << info.size << std::endl;
            std::cout << "   info.itemsize = " << info.itemsize << std::endl;
            std::cout << "   info.shape[0] = " << info.shape[0] << std::endl;
            std::cout << "   info.shape[1] = " << info.shape[1] << std::endl;
            std::cout << "   info.strides[0] = " << info.strides[0] << std::endl;
            std::cout << "   info.strides[1] = " << info.strides[1] << std::endl;
            */
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
         //.def( "DataType", &dip::Image::DataType )
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
         .def( "Squeeze", py::overload_cast<>( &dip::Image::Squeeze ))
         .def( "AddSingleton", &dip::Image::AddSingleton, "dim"_a )
         .def( "ExpandDimensionality", &dip::Image::ExpandDimensionality, "dim"_a )
         .def( "ExpandSingletonDimension", &dip::Image::ExpandSingletonDimension, "dim"_a, "newSize"_a )
         .def( "ExpandSingletonDimensions", &dip::Image::ExpandSingletonDimensions, "newSizes"_a )
         .def( "UnexpandSingletonDimensions", &dip::Image::UnexpandSingletonDimensions)
         .def( "IsSingletonExpansionPossible", &dip::Image::IsSingletonExpansionPossible, "newSizes"_a )
         .def( "ExpandSingletonTensor", &dip::Image::ExpandSingletonTensor, "size"_a )
         .def( "Mirror", &dip::Image::Mirror, "process"_a )
         .def( "ReshapeTensor", py::overload_cast< dip::uint, dip::uint >( &dip::Image::ReshapeTensor ), "rows"_a,
               "cols"_a )
         .def( "ReshapeTensorAsVector", &dip::Image::ReshapeTensorAsVector )
         .def( "ReshapeTensorAsDiagonal", &dip::Image::ReshapeTensorAsDiagonal )
         .def( "Transpose", &dip::Image::Transpose )
         .def( "TensorToSpatial", py::overload_cast<>( &dip::Image::TensorToSpatial ))
         .def( "TensorToSpatial", py::overload_cast< dip::uint >( &dip::Image::TensorToSpatial ), "dim"_a )
         .def( "SpatialToTensor", py::overload_cast<>( &dip::Image::SpatialToTensor ))
         .def( "SpatialToTensor", py::overload_cast< dip::uint >( &dip::Image::SpatialToTensor ), "dim"_a )
         .def( "SpatialToTensor", py::overload_cast< dip::uint, dip::uint >( &dip::Image::SpatialToTensor ), "rows"_a, "cols"_a )
         .def( "SpatialToTensor", py::overload_cast< dip::uint, dip::uint, dip::uint >( &dip::Image::SpatialToTensor ), "dim"_a, "rows"_a, "cols"_a )
         .def( "SplitComplex", py::overload_cast<>( &dip::Image::SplitComplex ))
         .def( "SplitComplex", py::overload_cast< dip::uint >( &dip::Image::SplitComplex ), "dim"_a )
         .def( "MergeComplex", py::overload_cast<>( &dip::Image::MergeComplex ))
         .def( "MergeComplex", py::overload_cast< dip::uint >( &dip::Image::MergeComplex ), "dim"_a )
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
         .def( "__ipow__", []( dip::Image& a, dip::Image const& b ) { dip::Power( a, b, a ); return a; }, py::is_operator() )
         .def( "__ipow__", []( dip::Image& a, float b ) { dip::Power( a, b, a ); return a; }, py::is_operator() )
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
}
