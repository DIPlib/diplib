/*
 * PyDIP 3.0, Python bindings for DIPlib 3.0
 *
 * (c)2017-2019, Flagship Biosciences, Inc., written by Cris Luengo.
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
#include "diplib/generic_iterators.h"

namespace {

dip::Image BufferToImage( py::buffer& buf ) {
   py::buffer_info info = buf.request();
   //std::cout << "--Constructing dip::Image from Python buffer.\n";
   //std::cout << "   info.ptr = " << info.ptr << std::endl;
   //std::cout << "   info.format = " << info.format << std::endl;
   //std::cout << "   info.ndims = " << info.shape.size() << std::endl;
   //std::cout << "   info.size = " << info.size << std::endl;
   //std::cout << "   info.itemsize = " << info.itemsize << std::endl;
   //std::cout << "   info.shape[0] = " << info.shape[0] << std::endl;
   //std::cout << "   info.shape[1] = " << info.shape[1] << std::endl;
   //std::cout << "   info.strides[0] = " << info.strides[0] << std::endl;
   //std::cout << "   info.strides[1] = " << info.strides[1] << std::endl;
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
      case py::format_descriptor< dip::uint64 >::c:
         datatype = dip::DT_UINT64;
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
      case py::format_descriptor< dip::sint64 >::c:
         datatype = dip::DT_SINT64;
         break;
      case py::format_descriptor< dip::sfloat >::c:
         datatype = dip::DT_SFLOAT;
         break;
      case py::format_descriptor< dip::dfloat >::c:
         datatype = dip::DT_DFLOAT;
         break;
      case 'Z':
         switch( info.format[ 1 ] ) {
            case py::format_descriptor< dip::scomplex >::c:
               datatype = dip::DT_SCOMPLEX;
               break;
            case py::format_descriptor< dip::dcomplex >::c:
               datatype = dip::DT_DCOMPLEX;
               break;
            default:
               std::cout << "Attempted to convert buffer to dip.Image object: data type not compatible." << std::endl;
               DIP_THROW( "Buffer data type not compatible with class Image" );
         }
         break;
      default:
         std::cout << "Attempted to convert buffer to dip.Image object: data type not compatible." << std::endl;
         DIP_THROW( "Buffer data type not compatible with class Image" );
   }
   // Sizes, reversed
   dip::uint ndim = static_cast< dip::uint >( info.ndim );
   DIP_ASSERT( ndim == info.shape.size() );
   dip::UnsignedArray sizes( ndim, 1 );
   for( dip::uint ii = 0; ii < ndim; ++ii ) {
      sizes[ ii ] = static_cast< dip::uint >( info.shape[ ndim - ii - 1 ] );
   }
   // Strides, also reversed
   dip::IntegerArray strides( ndim, 1 );
   for( dip::uint ii = 0; ii < ndim; ++ii ) {
      dip::sint s = info.strides[ ndim - ii - 1 ] / static_cast< dip::sint >( info.itemsize );
      DIP_THROW_IF( s * static_cast< dip::sint >( info.itemsize ) != info.strides[ ndim - ii - 1 ],
                    "Cannot create image out of an array where strides are not in whole pixels" );
      strides[ ii ] = s;
   }
   // The containing Python object. We increase its reference count, and create a unique_ptr that decreases
   // its reference count again.
   PyObject* pyObject = buf.ptr();
   //std::cout << "   *** Incrementing ref count for pyObject " << pyObject << std::endl;
   Py_XINCREF( pyObject );
   dip::DataSegment dataSegment{ pyObject, []( void* obj ) {
      //std::cout << "   *** Decrementing ref count for pyObject " << obj << std::endl;
      Py_XDECREF( static_cast< PyObject* >( obj ));
   }};
   // Create an image with all of this.
   dip::Image out( dataSegment, info.ptr, datatype, sizes, strides, {}, 1 );
   // If it's a 3D image and the first dimension has <10 pixels, let's assume it's a tensor dimension:
   if(( sizes.size() > 2 ) && ( sizes[ 0 ] < 10 )) {
      out.SpatialToTensor( 0 );
   }
   return out;
}

py::buffer_info ImageToBuffer( dip::Image const& image ) {
   dip::String format;
   switch( image.DataType() ) {
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
      case dip::DT_UINT64:
         format = py::format_descriptor< dip::uint64 >::format();
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
      case dip::DT_SINT64:
         format = py::format_descriptor< dip::sint64 >::format();
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
   dip::sint itemsize = static_cast< dip::sint >( image.DataType().SizeOf() );
   dip::IntegerArray strides = image.Strides();
   for( dip::sint& s : strides ) {
      s *= itemsize;
   }
   dip::UnsignedArray sizes = image.Sizes();
   // Reverse sizes and strides arrays
   dip::uint nDims = sizes.size();
   for( dip::uint ii = 0; ii < nDims / 2; ++ii ) {
      std::swap( sizes[ ii ], sizes[ nDims - ii - 1 ] );
      std::swap( strides[ ii ], strides[ nDims - ii - 1 ] );
   }
   if( !image.IsScalar() ) {
      sizes.push_back( image.TensorElements() );
      strides.push_back( image.TensorStride() * itemsize );
   }
   py::buffer_info info{ image.Origin(), itemsize, format, static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
   //std::cout << "--Constructed Python buffer for dip::Image object.\n";
   //std::cout << "   info.ptr = " << info.ptr << std::endl;
   //std::cout << "   info.format = " << info.format << std::endl;
   //std::cout << "   info.ndims = " << info.ndims << std::endl;
   //std::cout << "   info.size = " << info.size << std::endl;
   //std::cout << "   info.itemsize = " << info.itemsize << std::endl;
   //std::cout << "   info.shape[0] = " << info.shape[0] << std::endl;
   //std::cout << "   info.shape[1] = " << info.shape[1] << std::endl;
   //std::cout << "   info.strides[0] = " << info.strides[0] << std::endl;
   //std::cout << "   info.strides[1] = " << info.strides[1] << std::endl;
   return info;
}

dip::String ImageRepr( dip::Image const& image ) {
   if( !image.IsForged() ) {
      return "<Empty image>";
   }
   std::ostringstream os;
   if( image.IsColor() ) {
      os << "<Color image (" << image.Tensor() << ", " << image.ColorSpace() << ")";
   } else if( !image.IsScalar() ) {
      os << "<Tensor image (" << image.Tensor() << ")";
   } else {
      os << "<Scalar image";
   }
   os << ", " << image.DataType();
   if( image.Dimensionality() == 0 ) {
      os << ", 0D";
   } else {
      os << ", sizes " << image.Sizes();
   }
   os << ">";
   return os.str();
}

} // namespace

void init_image( py::module& m ) {
   auto img = py::class_< dip::Image >( m, "Image", py::buffer_protocol(), "The class that encapsulates DIPlib images of all types." );
   // Constructor for raw (unforged) image, to be used e.g. when no mask input argument is needed
   img.def( py::init<>() );
   // Constructor, generic
   img.def( py::init< dip::UnsignedArray const&, dip::uint, dip::DataType >(), "sizes"_a, "tensorElems"_a = 1, "dt"_a = dip::DT_SFLOAT );
   // Constructor that takes a Sample
   img.def( py::init< dip::Image::Sample const& >(), "sample"_a );
   img.def( py::init< dip::Image::Sample const&, dip::DataType >(), "sample"_a, "dt"_a );
   // Constructor that takes a Pixel
   img.def( py::init< dip::Image::Pixel const& >(), "pixel"_a );
   img.def( py::init< dip::Image::Pixel const&, dip::DataType >(), "pixel"_a, "dt"_a );
   // Create new similar image
   img.def( "Similar", py::overload_cast< >( &dip::Image::Similar, py::const_ ));
   img.def( "Similar", py::overload_cast< dip::DataType >( &dip::Image::Similar, py::const_ ), "dt"_a );
   // Constructor that takes a Python raw buffer
   img.def( py::init([]( py::buffer& buf ) { return BufferToImage( buf ); } ));
   py::implicitly_convertible< py::buffer, dip::Image >();
   // Export a Python raw buffer
   img.def_buffer( []( dip::Image& self ) -> py::buffer_info { return ImageToBuffer( self ); } );
   // Basic properties
   img.def( "__repr__", &ImageRepr );
   img.def( "__str__", []( dip::Image const& self ) { std::ostringstream os; os << self; return os.str(); } );
   img.def( "__len__", []( dip::Image const& self ) { return self.NumberOfPixels(); } );
   img.def( "IsEmpty", []( dip::Image const& self ) { return !self.IsForged(); } );
   img.def( "Dimensionality", &dip::Image::Dimensionality );
   img.def( "Sizes", &dip::Image::Sizes );
   img.def( "Size", &dip::Image::Size, "dim"_a );
   img.def( "NumberOfPixels", &dip::Image::NumberOfPixels );
   img.def( "NumberOfSamples", &dip::Image::NumberOfSamples );
   img.def( "Strides", &dip::Image::Strides );
   img.def( "Stride", &dip::Image::Stride, "dim"_a );
   img.def( "TensorStride", &dip::Image::TensorStride );
   img.def( "HasContiguousData", &dip::Image::HasContiguousData );
   img.def( "HasNormalStrides", &dip::Image::HasNormalStrides );
   img.def( "IsSingletonExpanded", &dip::Image::IsSingletonExpanded );
   img.def( "HasSimpleStride", &dip::Image::HasSimpleStride );
   img.def( "HasSameDimensionOrder", &dip::Image::HasSameDimensionOrder, "other"_a );
   img.def( "TensorSizes", &dip::Image::TensorSizes );
   img.def( "TensorElements", &dip::Image::TensorElements );
   img.def( "TensorColumns", &dip::Image::TensorColumns );
   img.def( "TensorRows", &dip::Image::TensorRows );
   img.def( "TensorShape", &dip::Image::TensorShape );
   img.def( "Tensor", &dip::Image::Tensor );
   img.def( "IsScalar", &dip::Image::IsScalar );
   img.def( "IsVector", &dip::Image::IsVector );
   img.def( "IsSquare", &dip::Image::IsSquare );
   img.def( "DataType", &dip::Image::DataType );
   img.def( "ColorSpace", &dip::Image::ColorSpace );
   img.def( "IsColor", &dip::Image::IsColor );
   img.def( "SetColorSpace", &dip::Image::SetColorSpace, "colorSpace"_a );
   img.def( "ResetColorSpace", &dip::Image::ResetColorSpace );
   img.def( "PixelSize", py::overload_cast<>( &dip::Image::PixelSize ));
   img.def( "PixelSize", py::overload_cast<>( &dip::Image::PixelSize, py::const_ ));
   img.def( "PixelSize", py::overload_cast< dip::uint >( &dip::Image::PixelSize, py::const_ ), "dim"_a );
   img.def( "SetPixelSize", py::overload_cast< dip::PixelSize const& >( &dip::Image::SetPixelSize ), "pixelSize"_a );
   img.def( "SetPixelSize", py::overload_cast< dip::uint, dip::PhysicalQuantity >( &dip::Image::SetPixelSize ), "dim"_a, "sz"_a );
   img.def( "HasPixelSize", &dip::Image::HasPixelSize );
   img.def( "IsIsotropic", &dip::Image::IsIsotropic );
   img.def( "PixelsToPhysical", &dip::Image::PixelsToPhysical, "array"_a );
   img.def( "PhysicalToPixels", &dip::Image::PhysicalToPixels, "array"_a );
   // About the data segment
   img.def( "IsShared", &dip::Image::IsShared );
   img.def( "ShareCount", &dip::Image::ShareCount );
   img.def( "SharesData", &dip::Image::SharesData, "other"_a );
   img.def( "Aliases", &dip::Image::Aliases, "other"_a );
   img.def( "IsIdenticalView", &dip::Image::IsIdenticalView, "other"_a );
   img.def( "IsOverlappingView", py::overload_cast< dip::Image const& >( &dip::Image::IsOverlappingView, py::const_ ), "other"_a );
   img.def( "Protect", &dip::Image::Protect, "set"_a = true );
   img.def( "IsProtected", &dip::Image::IsProtected );
   // Modify image without copying pixel data
   img.def( "PermuteDimensions", &dip::Image::PermuteDimensions, "order"_a, py::return_value_policy::reference_internal );
   img.def( "SwapDimensions", &dip::Image::SwapDimensions, "dim1"_a, "dim2"_a, py::return_value_policy::reference_internal );
   img.def( "Flatten", &dip::Image::Flatten, py::return_value_policy::reference_internal );
   img.def( "FlattenAsMuchAsPossible", &dip::Image::FlattenAsMuchAsPossible, py::return_value_policy::reference_internal );
   img.def( "Squeeze", py::overload_cast<>( &dip::Image::Squeeze ), py::return_value_policy::reference_internal );
   img.def( "AddSingleton", &dip::Image::AddSingleton, "dim"_a, py::return_value_policy::reference_internal );
   img.def( "ExpandDimensionality", &dip::Image::ExpandDimensionality, "dim"_a, py::return_value_policy::reference_internal );
   img.def( "ExpandSingletonDimension", &dip::Image::ExpandSingletonDimension, "dim"_a, "newSize"_a, py::return_value_policy::reference_internal );
   img.def( "ExpandSingletonDimensions", &dip::Image::ExpandSingletonDimensions, "newSizes"_a, py::return_value_policy::reference_internal );
   img.def( "UnexpandSingletonDimensions", &dip::Image::UnexpandSingletonDimensions, py::return_value_policy::reference_internal );
   img.def( "IsSingletonExpansionPossible", &dip::Image::IsSingletonExpansionPossible, "newSizes"_a );
   img.def( "ExpandSingletonTensor", &dip::Image::ExpandSingletonTensor, "size"_a, py::return_value_policy::reference_internal );
   img.def( "Mirror", &dip::Image::Mirror, "process"_a, py::return_value_policy::reference_internal );
   img.def( "Rotation90", py::overload_cast< dip::sint, dip::uint, dip::uint >( &dip::Image::Rotation90 ), "n"_a, "dimension1"_a, "dimension2"_a, py::return_value_policy::reference_internal );
   img.def( "Rotation90", py::overload_cast< dip::sint, dip::uint >( &dip::Image::Rotation90 ), "n"_a, "axis"_a, py::return_value_policy::reference_internal );
   img.def( "Rotation90", py::overload_cast< dip::sint >( &dip::Image::Rotation90 ), "n"_a, py::return_value_policy::reference_internal );
   img.def( "StandardizeStrides", py::overload_cast<>( &dip::Image::StandardizeStrides ), py::return_value_policy::reference_internal );
   img.def( "ReshapeTensor", py::overload_cast< dip::uint, dip::uint >( &dip::Image::ReshapeTensor ), "rows"_a, "cols"_a, py::return_value_policy::reference_internal );
   img.def( "ReshapeTensor", py::overload_cast< dip::Tensor const& >( &dip::Image::ReshapeTensor ), "example"_a, py::return_value_policy::reference_internal );
   img.def( "ReshapeTensorAsVector", &dip::Image::ReshapeTensorAsVector, py::return_value_policy::reference_internal );
   img.def( "ReshapeTensorAsDiagonal", &dip::Image::ReshapeTensorAsDiagonal, py::return_value_policy::reference_internal );
   img.def( "Transpose", &dip::Image::Transpose, py::return_value_policy::reference_internal );
   img.def( "TensorToSpatial", py::overload_cast<>( &dip::Image::TensorToSpatial ), py::return_value_policy::reference_internal );
   img.def( "TensorToSpatial", py::overload_cast< dip::uint >( &dip::Image::TensorToSpatial ), "dim"_a, py::return_value_policy::reference_internal  );
   img.def( "SpatialToTensor", py::overload_cast<>( &dip::Image::SpatialToTensor ), py::return_value_policy::reference_internal );
   img.def( "SpatialToTensor", py::overload_cast< dip::uint >( &dip::Image::SpatialToTensor ), "dim"_a, py::return_value_policy::reference_internal );
   img.def( "SpatialToTensor", py::overload_cast< dip::uint, dip::uint >( &dip::Image::SpatialToTensor ), "rows"_a, "cols"_a, py::return_value_policy::reference_internal );
   img.def( "SpatialToTensor", py::overload_cast< dip::uint, dip::uint, dip::uint >( &dip::Image::SpatialToTensor ), "dim"_a, "rows"_a, "cols"_a, py::return_value_policy::reference_internal );
   img.def( "SplitComplex", py::overload_cast<>( &dip::Image::SplitComplex ), py::return_value_policy::reference_internal );
   img.def( "SplitComplex", py::overload_cast< dip::uint >( &dip::Image::SplitComplex ), "dim"_a, py::return_value_policy::reference_internal );
   img.def( "MergeComplex", py::overload_cast<>( &dip::Image::MergeComplex ), py::return_value_policy::reference_internal );
   img.def( "MergeComplex", py::overload_cast< dip::uint >( &dip::Image::MergeComplex ), "dim"_a, py::return_value_policy::reference_internal );
   img.def( "SplitComplexToTensor", &dip::Image::SplitComplexToTensor, py::return_value_policy::reference_internal );
   img.def( "MergeTensorToComplex", &dip::Image::MergeTensorToComplex, py::return_value_policy::reference_internal );
   img.def( "ReinterpretCast", &dip::Image::ReinterpretCast, py::return_value_policy::reference_internal );
   img.def( "ReinterpretCastToSignedInteger", &dip::Image::ReinterpretCastToSignedInteger, py::return_value_policy::reference_internal );
   img.def( "ReinterpretCastToUnsignedInteger", &dip::Image::ReinterpretCastToUnsignedInteger, py::return_value_policy::reference_internal );
   img.def( "Crop", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::Image::Crop ),  "sizes"_a, "cropLocation"_a = "center", py::return_value_policy::reference_internal );
   // Create a view of another image.
   img.def( "Diagonal", []( dip::Image const& image ) -> dip::Image { return image.Diagonal(); } );
   img.def( "TensorRow", []( dip::Image const& image, dip::uint index ) -> dip::Image { return image.TensorRow( index ); }, "index"_a );
   img.def( "TensorColumn", []( dip::Image const& image, dip::uint index ) -> dip::Image { return image.TensorColumn( index ); }, "index"_a );
   img.def( "At", []( dip::Image const& image, dip::uint index ) -> dip::Image::Pixel { return image.At( index ); }, "index"_a );
   img.def( "At", []( dip::Image const& image, dip::uint x_index, dip::uint y_index ) -> dip::Image::Pixel { return image.At( x_index, y_index ); }, "x_index"_a, "y_index"_a );
   img.def( "At", []( dip::Image const& image, dip::uint x_index, dip::uint y_index, dip::uint z_index ) -> dip::Image::Pixel { return image.At( x_index, y_index, z_index ); }, "x_index"_a, "y_index"_a, "z_index"_a );
   img.def( "At", []( dip::Image const& image, dip::UnsignedArray const& coords ) -> dip::Image::Pixel { return image.At( coords ); }, "coords"_a );
   img.def( "At", []( dip::Image const& image, dip::Range x_range ) -> dip::Image { return image.At( x_range ); }, "x_range"_a );
   img.def( "At", []( dip::Image const& image, dip::Range x_range, dip::Range y_range ) -> dip::Image { return image.At( x_range, y_range ); }, "x_range"_a, "y_range"_a );
   img.def( "At", []( dip::Image const& image, dip::Range x_range, dip::Range y_range, dip::Range z_range ) -> dip::Image { return image.At( x_range, y_range, z_range ); }, "x_range"_a, "y_range"_a, "z_range"_a );
   img.def( "At", []( dip::Image const& image, dip::RangeArray ranges ) -> dip::Image { return image.At( ranges ); }, "ranges"_a );
   img.def( "At", []( dip::Image const& image, dip::Image const& mask ) -> dip::Image { return image.At( mask ); }, "mask"_a );
   img.def( "At", []( dip::Image const& image, dip::CoordinateArray const& coordinates ) -> dip::Image { return image.At( coordinates ); }, "coordinates"_a );
   img.def( "Cropped", []( dip::Image const& image, dip::UnsignedArray const& sizes, dip::String const& cropLocation ) -> dip::Image { return image.Cropped( sizes, cropLocation ); },
            "sizes"_a, "cropLocation"_a = "center" );
   img.def( "Real", []( dip::Image const& image ) -> dip::Image { return image.Real(); } );
   img.def( "Imaginary", []( dip::Image const& image ) -> dip::Image { return image.Imaginary(); } );
   img.def( "QuickCopy", &dip::Image::QuickCopy );
   // These don't exist, but we need to have a function for operation[] too
   img.def( "TensorElement", []( dip::Image const& image, dip::sint index ) -> dip::Image { return image[ index ]; }, "index"_a );
   img.def( "TensorElement", []( dip::Image const& image, dip::uint i, dip::uint j ) -> dip::Image { return image[ dip::UnsignedArray{ i, j } ]; }, "i"_a, "j"_a );
   img.def( "TensorElement", []( dip::Image const& image, dip::Range const& range ) -> dip::Image { return image[ range ]; }, "range"_a );
   // Copy or write data
   img.def( "Pad", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::Image::Pad, py::const_ ), "sizes"_a, "cropLocation"_a = "center" );
   img.def( "Copy", py::overload_cast<>( &dip::Image::Copy, py::const_ ));
   img.def( "Copy", py::overload_cast< dip::Image const& >( &dip::Image::Copy ), "src"_a );
   img.def( "Convert", &dip::Image::Convert, "dataType"_a );
   img.def( "ExpandTensor", &dip::Image::ExpandTensor );
   img.def( "Fill", py::overload_cast< dip::Image::Sample const& >( &dip::Image::Fill ), "sample"_a );
   img.def( "Fill", py::overload_cast< dip::Image::Pixel const& >( &dip::Image::Fill ), "pixel"_a );
   // Indexing using a mask image
   img.def( "__getitem__", []( dip::Image const& image, dip::Image const& mask ) -> dip::Image { return image.At( mask ); } );
   // Indexing into single pixel using coordinates
   img.def( "__getitem__", []( dip::Image const& image, dip::uint index ) -> dip::Image::Pixel { return image.At( index ); } );
   img.def( "__getitem__", []( dip::Image const& image, dip::UnsignedArray const& coords ) -> dip::Image::Pixel { return image.At( coords ); } );
   // Indexing into slice for 1D image
   img.def( "__getitem__", []( dip::Image const& image, dip::Range const& range ) -> dip::Image { return image.At( range ); } );
   // Indexing into slice for nD image
   img.def( "__getitem__", []( dip::Image const& image, dip::RangeArray const& rangeArray ) -> dip::Image { return image.At( rangeArray ); } );
   // Assignment using a mask image
   img.def( "__setitem__", []( dip::Image& image, dip::Image const& mask, dip::Image const& source ) { image.At( mask ).Copy( source ); } );
   img.def( "__setitem__", []( dip::Image& image, dip::Image const& mask, dip::Image::Sample const& sample ) { image.At( mask ).Fill( sample ); } );
   img.def( "__setitem__", []( dip::Image& image, dip::Image const& mask, dip::Image::Pixel const& pixel ) { image.At( mask ).Fill( pixel ); } );
   // Assignment into single pixel using linear index
   img.def( "__setitem__", []( dip::Image& image, dip::uint index, dip::Image::Sample const& v ) { image.At( index ) = v; } );
   img.def( "__setitem__", []( dip::Image& image, dip::uint index, dip::Image::Pixel const& v ) { image.At( index ) = v; } );
   // Indexing into single pixel using coordinates
   img.def( "__setitem__", []( dip::Image& image, dip::UnsignedArray const& coords, dip::Image::Sample const& v ) { image.At( coords ) = v; } );
   img.def( "__setitem__", []( dip::Image& image, dip::UnsignedArray const& coords, dip::Image::Pixel const& v ) { image.At( coords ) = v; } );
   // Assignment into slice for 1D image
   img.def( "__setitem__", []( dip::Image& image, dip::Range const& range, dip::Image const& source ) { image.At( range ).Copy( source ); } );
   img.def( "__setitem__", []( dip::Image& image, dip::Range const& range, dip::Image::Sample const& sample ) { image.At( range ).Fill( sample ); } );
   img.def( "__setitem__", []( dip::Image& image, dip::Range const& range, dip::Image::Pixel const& pixel ) { image.At( range ).Fill( pixel ); } );
   // Assignment into slice for nD image
   img.def( "__setitem__", []( dip::Image& image, dip::RangeArray const& rangeArray, dip::Image const& source ) { image.At( rangeArray ).Copy( source ); } );
   img.def( "__setitem__", []( dip::Image& image, dip::RangeArray const& rangeArray, dip::Image::Sample const& sample ) { image.At( rangeArray ).Fill( sample ); } );
   img.def( "__setitem__", []( dip::Image& image, dip::RangeArray const& rangeArray, dip::Image::Pixel const& value ) { image.At( rangeArray ).Fill( value ); } );
   // Operators
   img.def( py::self += py::self );
   img.def( py::self += dip::dfloat() );
   img.def( py::self + py::self );
   img.def( py::self + dip::dfloat() );
   img.def( py::self -= py::self );
   img.def( py::self -= dip::dfloat() );
   img.def( py::self - py::self );
   img.def( py::self - dip::dfloat() );
   img.def( py::self *= py::self );
   img.def( py::self *= dip::dfloat() );
   img.def( py::self * py::self );
   img.def( py::self * dip::dfloat() );
   img.def( py::self /= py::self );
   img.def( py::self /= dip::dfloat() );
   img.def( py::self / py::self );
   img.def( py::self / dip::dfloat() );
   img.def( py::self %= py::self );
   img.def( py::self %= dip::dfloat() );
   img.def( py::self % py::self );
   img.def( py::self % dip::dfloat() );
   img.def( "__pow__", []( dip::Image const& a, dip::Image const& b ) { return dip::Power( a, b ); }, py::is_operator() );
   img.def( "__pow__", []( dip::Image const& a, dip::dfloat b ) { return dip::Power( a, b ); }, py::is_operator() );
   img.def( "__pow__", []( dip::dfloat a, dip::Image const& b ) { return dip::Power( dip::Image{ a }, b ); }, py::is_operator() );
   img.def( "__ipow__", []( dip::Image& a, dip::Image const& b ) { dip::Power( a, b, a ); return a; }, py::is_operator() );
   img.def( "__ipow__", []( dip::Image& a, dip::dfloat b ) { dip::Power( a, b, a ); return a; }, py::is_operator() );
   img.def( py::self == py::self );
   img.def( py::self == dip::dfloat() );
   img.def( py::self != py::self );
   img.def( py::self != dip::dfloat() );
   img.def( py::self > py::self );
   img.def( py::self > dip::dfloat() );
   img.def( py::self >= py::self );
   img.def( py::self >= dip::dfloat() );
   img.def( py::self < py::self );
   img.def( py::self < dip::dfloat() );
   img.def( py::self <= py::self );
   img.def( py::self <= dip::dfloat() );
   img.def( py::self & py::self );
   img.def( py::self & dip::sint() );
   img.def( py::self | py::self );
   img.def( py::self | dip::sint() );
   img.def( py::self ^ py::self );
   img.def( py::self ^ dip::sint() );
   img.def( -py::self );
   img.def( "__invert__", []( dip::Image& a ) { return dip::Not( a ); }, py::is_operator() ); //img.def( ~py::self );
   // Iterators
   // TODO: This does not work! Iterators need to return a writeable reference, which implies making dip::Image::Pixel available from within Pyton
   //img.def( "__iter__", []( dip::Image const& image ) {
   //         return py::make_iterator< py::return_value_policy::reference_internal,
   //                                   dip::GenericImageIterator<>,
   //                                   dip::GenericImageIterator<>,
   //                                   dip::Image::Pixel >( dip::GenericImageIterator<>( image ),
   //                                                        dip::GenericImageIterator<>() );
   //}, py::keep_alive< 0, 1 >() ); // Essential: keep object alive while iterator exists

   m.def( "Create0D", []( dip::Image const& in ) -> dip::Image {
      DIP_THROW_IF( !in.IsForged(), dip::E::IMAGE_NOT_FORGED );
      DIP_THROW_IF( !in.IsScalar(), dip::E::IMAGE_NOT_SCALAR );
      dip::UnsignedArray sz = in.Sizes();
      DIP_THROW_IF( sz.size() > 2, dip::E::DIMENSIONALITY_NOT_SUPPORTED );
      bool swapped = false;
      if( sz.size() == 2 ) {
         std::swap( sz[ 0 ], sz[ 1 ] ); // This way storage will be column-major
         swapped = true;
      } else {
         sz.resize( 2, 1 ); // add dimensions of size 1
      }
      dip::Image out( sz, 1, in.DataType() );
      if( swapped ) {
         out.SwapDimensions( 0, 1 ); // Swap dimensions so they match those of `in`
      }
      out.Copy( in ); // copy pixel data, don't re-use
      out.Flatten();
      out.SpatialToTensor( 0, sz[ 0 ], sz[ 1 ] );
      return out;
   } );

   m.def( "Copy", py::overload_cast< dip::Image const& >( &dip::Copy ), "src"_a );
   m.def( "ExpandTensor", py::overload_cast< dip::Image const& >( &dip::ExpandTensor ), "src"_a );
   m.def( "Convert", py::overload_cast< dip::Image const&, dip::DataType >( &dip::Convert ), "src"_a, "dt"_a );
}
