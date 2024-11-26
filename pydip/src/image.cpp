/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022-2024, Cris Luengo.
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

#include <iostream>
#include <sstream>
#include <utility>

#include "pydip.h"
#include "diplib/file_io.h"

// We set `reverseDimensions` to `true` by default, meaning that dimensions are reversed when
// reading or writing from a Python buffer protocol object. Calling `ReverseDimensions()`
// from Python sets the `reverseDimensions` value to `false`.
bool reverseDimensions = true;

void OptionallyReverseDimensions( dip::Image& img ) {
   if( !reverseDimensions ) {
      img.ReverseDimensions();
   }
}

void OptionallyReverseDimensions( dip::FileInformation& fi ) {
   if( !reverseDimensions ) {
      ReverseDimensions( fi );
   }
}

// This is the number of elements along the first or last dimension of an array that will
// be interpreted as the tensor dimension when converting the array to a dip.Image.
// Use dip.SetTensorConversionThreshold() in Python to change this value.
dip::uint tensorConversionThreshold = 4;

namespace {

dip::Image BufferToImage( py::buffer& buf, bool auto_tensor = true ) {
   py::buffer_info info = buf.request();
   //std::cout << "--Constructing dip::Image from Python buffer.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndims = " << info.shape.size() << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
   // Data type
   dip::DataType datatype;
   switch( info.format[ 0 ] ) {
      case '?':
         datatype = dip::DT_BIN;
         break;
      case 'B':
         datatype = dip::DT_UINT8;
         break;
      case 'H':
         datatype = dip::DT_UINT16;
         break;
      case 'I':
         datatype = dip::DT_UINT32;
         break;
      case 'L':
         datatype = dip::DataType( static_cast< unsigned long >( 0 )); // This size varies for 32-bit and 64-bit Linux, and Windows.
         break;
      case 'K': // The documentation mentions this one
      case 'Q': // The pybind11 sources use this one
         datatype = dip::DT_UINT64;
         break;
      case 'b':
         datatype = dip::DT_SINT8;
         break;
      case 'h':
         datatype = dip::DT_SINT16;
         break;
      case 'i':
         datatype = dip::DT_SINT32;
         break;
      case 'l':
         datatype = dip::DataType( long{} ); // This size varies for 32-bit and 64-bit Linux, and Windows.
         break;
      case 'k': // The documentation mentions this one
      case 'q': // The pybind11 sources use this one
         datatype = dip::DT_SINT64;
         break;
      case 'f':
         datatype = dip::DT_SFLOAT;
         break;
      case 'd':
         datatype = dip::DT_DFLOAT;
         break;
      case 'Z':
         switch( info.format[ 1 ] ) {
            case 'f':
               datatype = dip::DT_SCOMPLEX;
               break;
            case 'd':
               datatype = dip::DT_DCOMPLEX;
               break;
            default:
               std::cout << "Attempted to convert buffer to dip.Image object: data type not compatible.\n";
               DIP_THROW( "Buffer data type not compatible with class Image" );
         }
         break;
      default:
         std::cout << "Attempted to convert buffer to dip.Image object: data type not compatible.\n";
         DIP_THROW( "Buffer data type not compatible with class Image" );
   }
   // An empty array leads to a raw image
   dip::uint ndim = static_cast< dip::uint >( info.ndim );
   DIP_ASSERT( ndim == info.shape.size() );
   bool isEmpty = false;
   for( dip::uint ii = 0; ii < ndim; ++ii ) {
      isEmpty |= info.shape[ ii ] == 0;
   }
   //std::cout << "isEmpty = " << isEmpty << '\n';
   if( isEmpty ) {
      dip::Image out;
      out.SetDataType( datatype );
      return out;
   }
   // Sizes
   dip::UnsignedArray sizes( ndim, 1 );
   for( dip::uint ii = 0; ii < ndim; ++ii ) {
      sizes[ ii ] = static_cast< dip::uint >( info.shape[ ii ] );
   }
   // Strides
   dip::IntegerArray strides( ndim, 1 );
   for( dip::uint ii = 0; ii < ndim; ++ii ) {
      dip::sint s = info.strides[ ii ] / static_cast< dip::sint >( info.itemsize );
      DIP_THROW_IF( s * static_cast< dip::sint >( info.itemsize ) != info.strides[ ii ],
                    "Cannot create image out of an array where strides are not in whole pixels" );
      strides[ ii ] = s;
   }
   // Optionally reverse dimensions
   if( reverseDimensions ) {
      sizes.reverse();
      strides.reverse();
   }
   // The containing Python object. We increase its reference count, and create a unique_ptr that decreases
   // its reference count again.
   PyObject* pyObject = buf.ptr();
   //std::cout << "   *** Incrementing ref count for pyObject " << pyObject << '\n';
   Py_XINCREF( pyObject );
   dip::DataSegment dataSegment{
      pyObject, []( void* obj ) {
         //std::cout << "   *** Decrementing ref count for pyObject " << obj << '\n';
         PyGILState_STATE gstate = PyGILState_Ensure();
         Py_XDECREF( static_cast< PyObject* >( obj ));
         PyGILState_Release( gstate );
      }
   };
   // Create an image with all of this.
   dip::Image out( dataSegment, info.ptr, datatype, sizes, strides, {}, 1 );

   if( auto_tensor ) {
      // If it's a 3D image and the first or last dimension has few pixels, let's assume it's a tensor dimension:
      if( sizes.size() > 2 ) {
         dip::uint dim = ndim - 1;
         if( sizes[ 0 ] < sizes[ dim ] ) {
            dim = 0;
         }
         if( sizes[ dim ] <= tensorConversionThreshold ) {
            out.SpatialToTensor( dim );
         }
      }
   }
   out.Protect();
   return out;
}

dip::Image BufferToImage( py::buffer& buf, dip::sint tensor_axis ) {
   auto img = BufferToImage( buf, false );
   tensor_axis = -tensor_axis - 1;
   if( tensor_axis < 0 ) {
      tensor_axis += static_cast< dip::sint >( img.Dimensionality() );
   }
   img.SpatialToTensor( static_cast< dip::uint >( tensor_axis ));
   return img;
}

py::buffer_info ImageToBuffer( dip::Image const& image ) {
   // Get data type and size
   dip::String format;
   switch( image.DataType() ) {
      case dip::DT_BIN:
         format = "?";
         break;
      case dip::DT_UINT8:
         format = "B";
         break;
      case dip::DT_UINT16:
         format = "H";
         break;
      case dip::DT_UINT32:
         format = "I";
         break;
      case dip::DT_UINT64:
         format = "Q";
         break;
      case dip::DT_SINT8:
         format = "b";
         break;
      case dip::DT_SINT16:
         format = "h";
         break;
      case dip::DT_SINT32:
         format = "i";
         break;
      case dip::DT_SINT64:
         format = "q";
         break;
      case dip::DT_SFLOAT:
         format = "f";
         break;
      case dip::DT_DFLOAT:
         format = "d";
         break;
      case dip::DT_SCOMPLEX:
         format = "Zf";
         break;
      case dip::DT_DCOMPLEX:
         format = "Zd";
         break;
      default:
         DIP_THROW( "Image of unknown type" ); // should never happen
   }
   dip::sint itemsize = static_cast< dip::sint >( image.DataType().SizeOf() );
   // Short cut for non-forged images
   if( !image.IsForged() ) {
      //std::cout << "Getting buffer of non-forged image\n";
      return py::buffer_info{ nullptr, itemsize, format, 1, { 0 }, { 1 } };
   }
   // Get sizes and strides
   dip::UnsignedArray sizes = image.Sizes();
   dip::IntegerArray strides = image.Strides();
   for( dip::sint& s : strides ) {
      s *= itemsize;
   }
   // Optionally reverse sizes and strides arrays
   if( reverseDimensions ) {
      sizes.reverse();
      strides.reverse();
   }
   // Add tensor dimension as the last array dimension
   if( !image.IsScalar() ) {
      sizes.push_back( image.TensorElements() );
      strides.push_back( image.TensorStride() * itemsize );
   }
   py::buffer_info info{ image.Origin(), itemsize, format, static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
   //std::cout << "--Constructed Python buffer for dip::Image object.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndims = " << info.ndims << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
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

constexpr char const* TensorElementDeprecationMessage = "dip.Image.TensorElement() is deprecated and will be removed in a future version of PyDIP. Use () indexing instead.";

} // namespace

void init_image( py::module& m ) {

   auto img = py::class_< dip::Image >( m, "Image", py::buffer_protocol(), doc_strings::dip·Image );
   // Constructor for raw (unforged) image, to be used e.g. when no mask input argument is needed:
   //   None implicitly converts to an image
   img.def( py::init<>(), doc_strings::dip·Image·Image );
   img.def( py::init( []( py::none const& ) { return dip::Image{}; } ), "none"_a, doc_strings::dip·Image·Image );
   py::implicitly_convertible< py::none, dip::Image >();
   // Constructor that takes a Sample: scalar implicitly converts to an image
   img.def( py::init< dip::Image::Sample const& >(), "sample"_a, doc_strings::dip·Image·Image·Sample·CL );
   img.def( py::init< dip::Image::Sample const&, dip::DataType >(), "sample"_a, "dt"_a, doc_strings::dip·Image·Image·Sample·CL·dip·DataType· );
   py::implicitly_convertible< dip::Image::Sample, dip::Image >();
   // Constructor that takes a Python raw buffer
   img.def( py::init( []( py::buffer& buf ) { return BufferToImage( buf ); } ), "array"_a, "Creates an image around the data of a NumPy array, automatically finding the tensor dimension." );
   img.def( py::init( []( py::buffer& buf, py::none const& ) { return BufferToImage( buf, false ); } ), "array"_a, "none"_a, "Creates a scalar image around the data of a NumPy array." );
   img.def( py::init( []( py::buffer& buf, dip::sint tensor_axis ) { return BufferToImage( buf, tensor_axis ); } ), "array"_a, "tensor_axis"_a, "Creates an image around the data of a NumPy array, using the given axis as the tensor dimension." );
   py::implicitly_convertible< py::buffer, dip::Image >();
   // Export a Python raw buffer
   img.def_buffer( []( dip::Image& self ) -> py::buffer_info { return ImageToBuffer( self ); } );
   // Constructor, generic
   img.def( py::init< dip::UnsignedArray const&, dip::uint, dip::DataType >(), "sizes"_a, "tensorElems"_a = 1, "dt"_a = dip::DT_SFLOAT, doc_strings::dip·Image·Image·UnsignedArray··dip·uint··dip·DataType· );
   // Create new similar image
   img.def( "Similar", py::overload_cast<>( &dip::Image::Similar, py::const_ ), doc_strings::dip·Image·Similar·C);
   img.def( "Similar", py::overload_cast< dip::DataType >( &dip::Image::Similar, py::const_ ), "dt"_a, doc_strings::dip·Image·Similar·dip·DataType··C );

   // Basic properties
   img.def( "__repr__", &ImageRepr );
   img.def( "__str__", []( dip::Image const& self ) {
      std::ostringstream os;
      os << self;
      return os.str();
   } );
   img.def( "__len__", []( dip::Image const& self ) { return self.IsForged() ? self.NumberOfPixels() : 0; } );
   img.def( "IsForged", &dip::Image::IsForged, "See also `IsEmpty()`.", doc_strings::dip·Image·IsForged·C );
   img.def( "IsEmpty", []( dip::Image const& self ) { return !self.IsForged(); },
            "Returns `True` if the image is raw. Reverse of `IsForged()`." ); // Honestly, I don't remember why I created this...
   img.def( "Dimensionality", &dip::Image::Dimensionality, doc_strings::dip·Image·Dimensionality·C );
   img.def( "Sizes", &dip::Image::Sizes, doc_strings::dip·Image·Sizes·C );
   img.def( "Size", &dip::Image::Size, "dim"_a, doc_strings::dip·Image·Size·dip·uint··C );
   img.def( "NumberOfPixels", &dip::Image::NumberOfPixels, doc_strings::dip·Image·NumberOfPixels·C );
   img.def( "NumberOfSamples", &dip::Image::NumberOfSamples, doc_strings::dip·Image·NumberOfSamples·C );
   img.def( "Strides", &dip::Image::Strides, doc_strings::dip·Image·Strides·C );
   img.def( "Stride", &dip::Image::Stride, "dim"_a, doc_strings::dip·Image·Stride·dip·uint··C );
   img.def( "TensorStride", &dip::Image::TensorStride, doc_strings::dip·Image·TensorStride·C );
   img.def( "HasContiguousData", &dip::Image::HasContiguousData, doc_strings::dip·Image·HasContiguousData·C );
   img.def( "HasNormalStrides", &dip::Image::HasNormalStrides, doc_strings::dip·Image·HasNormalStrides·C );
   img.def( "IsSingletonExpanded", &dip::Image::IsSingletonExpanded, doc_strings::dip·Image·IsSingletonExpanded·C );
   img.def( "HasSimpleStride", &dip::Image::HasSimpleStride, doc_strings::dip·Image·HasSimpleStride·C );
   img.def( "HasSameDimensionOrder", &dip::Image::HasSameDimensionOrder, "other"_a, doc_strings::dip·Image·HasSameDimensionOrder·Image·CL·C );
   img.def( "TensorSizes", &dip::Image::TensorSizes, doc_strings::dip·Image·TensorSizes·C );
   img.def( "TensorElements", &dip::Image::TensorElements, doc_strings::dip·Image·TensorElements·C );
   img.def( "TensorColumns", &dip::Image::TensorColumns, doc_strings::dip·Image·TensorColumns·C );
   img.def( "TensorRows", &dip::Image::TensorRows, doc_strings::dip·Image·TensorRows·C );
   img.def( "TensorShape", &dip::Image::TensorShape, doc_strings::dip·Image·TensorShape·C );
   img.def( "Tensor", &dip::Image::Tensor, doc_strings::dip·Image·Tensor·C );
   img.def( "IsScalar", &dip::Image::IsScalar, doc_strings::dip·Image·IsScalar·C );
   img.def( "IsVector", &dip::Image::IsVector, doc_strings::dip·Image·IsVector·C );
   img.def( "IsSquare", &dip::Image::IsSquare, doc_strings::dip·Image·IsSquare·C );
   img.def( "DataType", &dip::Image::DataType, doc_strings::dip·Image·DataType·C );
   img.def( "SetDataType", &dip::Image::SetDataType, doc_strings::dip·Image·SetDataType·dip·DataType· );
   img.def( "ColorSpace", &dip::Image::ColorSpace, doc_strings::dip·Image·ColorSpace·C );
   img.def( "IsColor", &dip::Image::IsColor, doc_strings::dip·Image·IsColor·C );
   img.def( "SetColorSpace", &dip::Image::SetColorSpace, "colorSpace"_a, doc_strings::dip·Image·SetColorSpace·String· );
   img.def( "ResetColorSpace", &dip::Image::ResetColorSpace, doc_strings::dip·Image·ResetColorSpace );
   img.def( "PixelSize", py::overload_cast<>( &dip::Image::PixelSize ), doc_strings::dip·Image·PixelSize );
   img.def( "PixelSize", py::overload_cast<>( &dip::Image::PixelSize, py::const_ ), doc_strings::dip·Image·PixelSize·C );
   img.def( "PixelSize", py::overload_cast< dip::uint >( &dip::Image::PixelSize, py::const_ ), "dim"_a, doc_strings::dip·Image·PixelSize·dip·uint··C );
   img.def( "SetPixelSize", py::overload_cast< dip::PixelSize >( &dip::Image::SetPixelSize ), "pixelSize"_a, doc_strings::dip·Image·SetPixelSize·dip·PixelSize· );
   img.def( "SetPixelSize", []( dip::Image& self, dip::dfloat mag, dip::Units const& units ) {
               self.SetPixelSize( dip::PhysicalQuantity( mag, units ));
            }, "magnitude"_a, "units"_a = dip::Units{},
            "Overload that accepts the two components of a `dip.PhysicalQuantity`, sets\n"
            "all dimensions to the same value." );
   img.def( "SetPixelSize", []( dip::Image& self, dip::FloatArray mags, dip::Units const& units ) {
               dip::PhysicalQuantityArray pq( mags.size() );
               for( dip::uint ii = 0; ii < mags.size(); ++ii ) {
                  pq[ ii ] = mags[ ii ] * units;
               }
               self.SetPixelSize( pq );
            }, "magnitudes"_a, "units"_a = dip::Units{},
            "Overload that accepts the two components of a `dip.PhysicalQuantity`, using\n"
            "a different magnitude for each dimension." );
   img.def( "SetPixelSize", py::overload_cast< dip::uint, dip::PhysicalQuantity >( &dip::Image::SetPixelSize ), "dim"_a, "sz"_a, doc_strings::dip·Image·SetPixelSize·dip·uint··PhysicalQuantity· );
   img.def( "SetPixelSize", []( dip::Image& self, dip::uint dim, dip::dfloat mag, dip::Units const& units ) {
               self.SetPixelSize( dim, dip::PhysicalQuantity( mag, units ));
            }, "dim"_a, "magnitude"_a, "units"_a = dip::Units{},
            "Overload that accepts the two components of a `dip.PhysicalQuantity`, sets\n"
            "dimension `dim` only." );
   img.def( "ResetPixelSize", &dip::Image::ResetPixelSize, doc_strings::dip·Image·ResetPixelSize );
   img.def( "HasPixelSize", &dip::Image::HasPixelSize, doc_strings::dip·Image·HasPixelSize·C );
   img.def( "IsIsotropic", &dip::Image::IsIsotropic, doc_strings::dip·Image·IsIsotropic·C );
   img.def( "PixelsToPhysical", &dip::Image::PixelsToPhysical, "array"_a, doc_strings::dip·Image·PixelsToPhysical·FloatArray·CL·C );
   img.def( "PhysicalToPixels", &dip::Image::PhysicalToPixels, "array"_a, doc_strings::dip·Image·PhysicalToPixels·PhysicalQuantityArray·CL·C );

   // About the data segment
   img.def( "IsShared", &dip::Image::IsShared, doc_strings::dip·Image·IsShared·C );
   img.def( "ShareCount", &dip::Image::ShareCount, doc_strings::dip·Image·ShareCount·C );
   img.def( "SharesData", &dip::Image::SharesData, "other"_a, doc_strings::dip·Image·SharesData·Image·CL·C );
   img.def( "Aliases", &dip::Image::Aliases, "other"_a, doc_strings::dip·Image·Aliases·Image·CL·C );
   img.def( "IsIdenticalView", &dip::Image::IsIdenticalView, "other"_a, doc_strings::dip·Image·IsIdenticalView·Image·CL·C );
   img.def( "IsOverlappingView", py::overload_cast< dip::Image const& >( &dip::Image::IsOverlappingView, py::const_ ), "other"_a, doc_strings::dip·Image·IsOverlappingView·Image·CL·C );
   img.def( "Protect", &dip::Image::Protect, "set"_a = true, doc_strings::dip·Image·Protect·bool· );
   img.def( "IsProtected", &dip::Image::IsProtected, doc_strings::dip·Image·IsProtected·C );

   // Modify image without copying pixel data
   img.def( "PermuteDimensions", &dip::Image::PermuteDimensions, "order"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·PermuteDimensions·UnsignedArray·CL );
   img.def( "SwapDimensions", &dip::Image::SwapDimensions, "dim1"_a, "dim2"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·SwapDimensions·dip·uint··dip·uint· );
   img.def( "ReverseDimensions", &dip::Image::ReverseDimensions, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReverseDimensions );
   img.def( "Flatten", &dip::Image::Flatten, py::return_value_policy::reference_internal, doc_strings::dip·Image·Flatten );
   img.def( "FlattenAsMuchAsPossible", &dip::Image::FlattenAsMuchAsPossible, py::return_value_policy::reference_internal, doc_strings::dip·Image·FlattenAsMuchAsPossible );
   img.def( "SplitDimension", &dip::Image::SplitDimension, "dim"_a, "size"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·SplitDimension·dip·uint··dip·uint· );
   img.def( "Squeeze", py::overload_cast<>( &dip::Image::Squeeze ), py::return_value_policy::reference_internal, doc_strings::dip·Image·Squeeze·UnsignedArray·L );
   img.def( "Squeeze", py::overload_cast< dip::uint >( &dip::Image::Squeeze ), "dims"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Squeeze );
   img.def( "Squeeze", py::overload_cast< dip::UnsignedArray& >( &dip::Image::Squeeze ), "dim"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Squeeze·dip·uint· );
   img.def( "AddSingleton", py::overload_cast< dip::uint >( &dip::Image::AddSingleton ), "dim"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·AddSingleton·dip·uint· );
   img.def( "AddSingleton", py::overload_cast< dip::UnsignedArray const& >( &dip::Image::AddSingleton ), "dims"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·AddSingleton·UnsignedArray·CL );
   img.def( "ExpandDimensionality", &dip::Image::ExpandDimensionality, "dim"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·ExpandDimensionality·dip·uint· );
   img.def( "ExpandSingletonDimension", &dip::Image::ExpandSingletonDimension, "dim"_a, "newSize"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·ExpandSingletonDimension·dip·uint··dip·uint· );
   img.def( "ExpandSingletonDimensions", &dip::Image::ExpandSingletonDimensions, "newSizes"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·ExpandSingletonDimensions·UnsignedArray·CL );
   img.def( "UnexpandSingletonDimensions", &dip::Image::UnexpandSingletonDimensions, py::return_value_policy::reference_internal, doc_strings::dip·Image·UnexpandSingletonDimensions );
   img.def( "IsSingletonExpansionPossible", &dip::Image::IsSingletonExpansionPossible, "newSizes"_a, doc_strings::dip·Image·IsSingletonExpansionPossible·UnsignedArray·CL·C );
   img.def( "ExpandSingletonTensor", &dip::Image::ExpandSingletonTensor, "size"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·ExpandSingletonTensor·dip·uint· );
   img.def( "Mirror", py::overload_cast< dip::uint >( &dip::Image::Mirror ), "dimension"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Mirror·dip·uint· );
   img.def( "Mirror", py::overload_cast< dip::BooleanArray >( &dip::Image::Mirror ), "process"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Mirror·BooleanArray· );
   img.def( "Rotation90", py::overload_cast< dip::sint, dip::uint, dip::uint >( &dip::Image::Rotation90 ), "n"_a, "dimension1"_a, "dimension2"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Rotation90·dip·sint··dip·uint··dip·uint· );
   img.def( "Rotation90", py::overload_cast< dip::sint, dip::uint >( &dip::Image::Rotation90 ), "n"_a, "axis"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Rotation90·dip·sint··dip·uint· );
   img.def( "Rotation90", py::overload_cast< dip::sint >( &dip::Image::Rotation90 ), "n"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·Rotation90·dip·sint· );
   img.def( "StandardizeStrides", py::overload_cast<>( &dip::Image::StandardizeStrides ), py::return_value_policy::reference_internal, doc_strings::dip·Image·StandardizeStrides );
   img.def( "ReshapeTensor", py::overload_cast< dip::uint, dip::uint >( &dip::Image::ReshapeTensor ), "rows"_a, "cols"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReshapeTensor·dip·uint··dip·uint· );
   img.def( "ReshapeTensor", py::overload_cast< dip::Tensor const& >( &dip::Image::ReshapeTensor ), "example"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReshapeTensor·dip·Tensor·CL );
   img.def( "ReshapeTensorAsVector", &dip::Image::ReshapeTensorAsVector, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReshapeTensorAsVector );
   img.def( "ReshapeTensorAsDiagonal", &dip::Image::ReshapeTensorAsDiagonal, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReshapeTensorAsDiagonal );
   img.def( "Transpose", &dip::Image::Transpose, py::return_value_policy::reference_internal, doc_strings::dip·Image·Transpose );
   img.def( "TensorToSpatial", py::overload_cast<>( &dip::Image::TensorToSpatial ), py::return_value_policy::reference_internal );
   img.def( "TensorToSpatial", py::overload_cast< dip::uint >( &dip::Image::TensorToSpatial ), "dim"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·TensorToSpatial·dip·uint· );
   img.def( "SpatialToTensor", py::overload_cast<>( &dip::Image::SpatialToTensor ), py::return_value_policy::reference_internal );
   img.def( "SpatialToTensor", py::overload_cast< dip::uint >( &dip::Image::SpatialToTensor ), "dim"_a, py::return_value_policy::reference_internal );
   img.def( "SpatialToTensor", py::overload_cast< dip::uint, dip::uint >( &dip::Image::SpatialToTensor ), "rows"_a, "cols"_a, py::return_value_policy::reference_internal );
   img.def( "SpatialToTensor", py::overload_cast< dip::uint, dip::uint, dip::uint >( &dip::Image::SpatialToTensor ), "dim"_a, "rows"_a, "cols"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·SpatialToTensor·dip·uint··dip·uint··dip·uint· );
   img.def( "SplitComplex", py::overload_cast<>( &dip::Image::SplitComplex ), py::return_value_policy::reference_internal );
   img.def( "SplitComplex", py::overload_cast< dip::uint >( &dip::Image::SplitComplex ), "dim"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·SplitComplex·dip·uint· );
   img.def( "MergeComplex", py::overload_cast<>( &dip::Image::MergeComplex ), py::return_value_policy::reference_internal );
   img.def( "MergeComplex", py::overload_cast< dip::uint >( &dip::Image::MergeComplex ), "dim"_a, py::return_value_policy::reference_internal, doc_strings::dip·Image·MergeComplex·dip·uint· );
   img.def( "SplitComplexToTensor", &dip::Image::SplitComplexToTensor, py::return_value_policy::reference_internal, doc_strings::dip·Image·SplitComplexToTensor );
   img.def( "MergeTensorToComplex", &dip::Image::MergeTensorToComplex, py::return_value_policy::reference_internal, doc_strings::dip·Image·MergeTensorToComplex );
   img.def( "ReinterpretCast", &dip::Image::ReinterpretCast, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReinterpretCast·dip·DataType· );
   img.def( "ReinterpretCastToSignedInteger", &dip::Image::ReinterpretCastToSignedInteger, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReinterpretCastToSignedInteger );
   img.def( "ReinterpretCastToUnsignedInteger", &dip::Image::ReinterpretCastToUnsignedInteger, py::return_value_policy::reference_internal, doc_strings::dip·Image·ReinterpretCastToUnsignedInteger );
   img.def( "Crop", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::Image::Crop ), "sizes"_a, "cropLocation"_a = "center", py::return_value_policy::reference_internal, doc_strings::dip·Image·Crop·UnsignedArray·CL·String·CL );

   // Create a view of another image.
   img.def( "Diagonal", []( dip::Image const& self ) -> dip::Image { return self.Diagonal(); }, doc_strings::dip·Image·Diagonal·C );
   img.def( "TensorRow", []( dip::Image const& self, dip::uint index ) -> dip::Image { return self.TensorRow( index ); }, "index"_a, doc_strings::dip·Image·TensorRow·dip·uint··C );
   img.def( "TensorColumn", []( dip::Image const& self, dip::uint index ) -> dip::Image { return self.TensorColumn( index ); }, "index"_a, doc_strings::dip·Image·TensorColumn·dip·uint··C );
   img.def( "At", []( dip::Image const& self, dip::uint index ) -> dip::Image::Pixel { return self.At( index ); }, "index"_a, doc_strings::dip·Image·At·dip·uint··C );
   img.def( "At", []( dip::Image const& self, dip::uint x_index, dip::uint y_index ) -> dip::Image::Pixel { return self.At( x_index, y_index ); }, "x_index"_a, "y_index"_a, doc_strings::dip·Image·At·dip·uint··dip·uint··C );
   img.def( "At", []( dip::Image const& self, dip::uint x_index, dip::uint y_index, dip::uint z_index ) -> dip::Image::Pixel { return self.At( x_index, y_index, z_index ); }, "x_index"_a, "y_index"_a, "z_index"_a, doc_strings::dip·Image·At·dip·uint··dip·uint··dip·uint··C );
   img.def( "At", []( dip::Image const& self, dip::UnsignedArray const& coords ) -> dip::Image::Pixel { return self.At( coords ); }, "coords"_a, doc_strings::dip·Image·At·UnsignedArray·CL·C );
   img.def( "At", []( dip::Image const& self, dip::Range x_range ) -> dip::Image { return self.At( x_range ); }, "x_range"_a, doc_strings::dip·Image·At·Range·CL·C );
   img.def( "At", []( dip::Image const& self, dip::Range x_range, dip::Range y_range ) -> dip::Image { return self.At( x_range, y_range ); }, "x_range"_a, "y_range"_a, doc_strings::dip·Image·At·Range·CL·Range·CL·C );
   img.def( "At", []( dip::Image const& self, dip::Range x_range, dip::Range y_range, dip::Range z_range ) -> dip::Image { return self.At( x_range, y_range, z_range ); }, "x_range"_a, "y_range"_a, "z_range"_a, doc_strings::dip·Image·At·Range·CL·Range·CL·Range·CL·C );
   img.def( "At", []( dip::Image const& self, dip::RangeArray ranges ) -> dip::Image { return self.At( std::move( ranges )); }, "ranges"_a, doc_strings::dip·Image·At·RangeArray··C );
   img.def( "At", []( dip::Image const& self, dip::Image mask ) -> dip::Image { return self.At( std::move( mask )); }, "mask"_a, doc_strings::dip·Image·At·Image··C );
   img.def( "At", []( dip::Image const& self, dip::CoordinateArray const& coordinates ) -> dip::Image { return self.At( coordinates ); }, "coordinates"_a, doc_strings::dip·Image·At·CoordinateArray·CL·C );
   img.def( "Cropped", []( dip::Image const& self, dip::UnsignedArray const& sizes, dip::String const& cropLocation ) -> dip::Image {
      return self.Cropped( sizes, cropLocation );
   }, "sizes"_a, "cropLocation"_a = "center", doc_strings::dip·Image·Cropped·UnsignedArray·CL·String·CL·C );
   img.def( "Real", []( dip::Image const& self ) -> dip::Image { return self.Real(); }, doc_strings::dip·Image·Real·C );
   img.def( "Imaginary", []( dip::Image const& self ) -> dip::Image { return self.Imaginary(); }, doc_strings::dip·Image·Imaginary·C );
   img.def( "QuickCopy", &dip::Image::QuickCopy, doc_strings::dip·Image·QuickCopy·C );
   // These don't exist, but we need to have a function for operator[] too
   img.def( "__call__", []( dip::Image const& self, dip::sint index ) -> dip::Image { return self[ index ]; }, "index"_a, doc_strings::dip·Image·operatorsqbra·T·T··C );
   img.def( "__call__", []( dip::Image const& self, dip::uint i, dip::uint j ) -> dip::Image { return self[ dip::UnsignedArray{ i, j } ]; }, "i"_a, "j"_a, doc_strings::dip·Image·operatorsqbra·UnsignedArray·CL·C );
   img.def( "__call__", []( dip::Image const& self, dip::Range const& range ) -> dip::Image { return self[ range ]; }, "range"_a, doc_strings::dip·Image·operatorsqbra·Range··C );
   // Deprecated functions Nov 29, 2022 (after release 3.3.0) TODO: remove eventually.
   img.def( "TensorElement", []( dip::Image const& self, dip::sint index ) -> dip::Image {
      PyErr_WarnEx( PyExc_DeprecationWarning, TensorElementDeprecationMessage, 1 );
      return self[ index ];
   }, "index"_a, "Deprecated." );
   img.def( "TensorElement", []( dip::Image const& self, dip::uint i, dip::uint j ) -> dip::Image {
      PyErr_WarnEx( PyExc_DeprecationWarning, TensorElementDeprecationMessage, 1 );
      return self[ dip::UnsignedArray{ i, j } ];
   }, "i"_a, "j"_a, "Deprecated." );
   img.def( "TensorElement", []( dip::Image const& self, dip::Range const& range ) -> dip::Image {
      PyErr_WarnEx( PyExc_DeprecationWarning, TensorElementDeprecationMessage, 1 );
      return self[ range ];
   }, "range"_a, "Deprecated." );

   // Copy or write data
   img.def( "Pad", py::overload_cast< dip::UnsignedArray const&, dip::String const& >( &dip::Image::Pad, py::const_ ), "sizes"_a, "cropLocation"_a = "center", doc_strings::dip·Image·Pad·UnsignedArray·CL·String·CL·C );
   img.def( "Pad", py::overload_cast< dip::UnsignedArray const&, dip::Image::Pixel const&, dip::String const& >( &dip::Image::Pad, py::const_ ), "sizes"_a, "value"_a, "cropLocation"_a = "center", doc_strings::dip·Image·Pad·UnsignedArray·CL·Pixel·CL·String·CL·C );
   img.def( "Copy", py::overload_cast<>( &dip::Image::Copy, py::const_ ), doc_strings::dip·Image·Copy·C );
   img.def( "Copy", py::overload_cast< dip::Image const& >( &dip::Image::Copy ), "src"_a, doc_strings::dip·Image·Copy·Image·CL );
   img.def( "Convert", &dip::Image::Convert, "dataType"_a, doc_strings::dip·Image·Convert·dip·DataType· );
   img.def( "SwapBytesInSample", &dip::Image::SwapBytesInSample, doc_strings::dip·Image·SwapBytesInSample );
   img.def( "ExpandTensor", &dip::Image::ExpandTensor, doc_strings::dip·Image·ExpandTensor );
   img.def( "Fill", py::overload_cast< dip::Image::Pixel const& >( &dip::Image::Fill ), "pixel"_a, doc_strings::dip·Image·Fill·Pixel·CL );
   img.def( "Mask", &dip::Image::Mask, "mask"_a, doc_strings::dip·Image·Mask·dip·Image·CL );

   // Indexing into single pixel using coordinates
   img.def( "__getitem__", []( dip::Image const& self, dip::uint index ) -> dip::Image::Pixel { return self.At( index ); }, doc_strings::dip·Image·At·dip·uint··C );
   img.def( "__getitem__", []( dip::Image const& self, dip::UnsignedArray const& coords ) -> dip::Image::Pixel { return self.At( coords ); }, doc_strings::dip·Image·At·UnsignedArray·CL·C );
   // Indexing into slice for 1D image
   img.def( "__getitem__", []( dip::Image const& self, dip::Range const& range ) -> dip::Image { return self.At( range ); }, doc_strings::dip·Image·At·Range·CL·C );
   // Indexing into slice for nD image
   img.def( "__getitem__", []( dip::Image const& self, dip::RangeArray rangeArray ) -> dip::Image { return self.At( std::move( rangeArray )); }, doc_strings::dip·Image·At·RangeArray··C );
   // Indexing using a mask image
   img.def( "__getitem__", []( dip::Image const& self, dip::Image mask ) -> dip::Image { return self.At( std::move( mask )); }, doc_strings::dip·Image·At·Image··C );
   // Indexing using a list of coordinates
   img.def( "__getitem__", []( dip::Image const& self, dip::CoordinateArray const& coordinates ) -> dip::Image { return self.At( coordinates ); }, doc_strings::dip·Image·At·CoordinateArray·CL·C );

   // Assignment into single pixel using linear index
   img.def( "__setitem__", []( dip::Image& self, dip::uint index, dip::Image::Sample const& v ) { self.At( index ) = v; } );
   img.def( "__setitem__", []( dip::Image& self, dip::uint index, dip::Image::Pixel const& v ) { self.At( index ) = v; } );
   // Assignment into single pixel using coordinates
   img.def( "__setitem__", []( dip::Image& self, dip::UnsignedArray const& coords, dip::Image::Sample const& v ) { self.At( coords ) = v; } );
   img.def( "__setitem__", []( dip::Image& self, dip::UnsignedArray const& coords, dip::Image::Pixel const& v ) { self.At( coords ) = v; } );
   // Assignment into slice for 1D image
   img.def( "__setitem__", []( dip::Image& self, dip::Range const& range, dip::Image::Pixel const& pixel ) { self.At( range ).Fill( pixel ); } );
   img.def( "__setitem__", []( dip::Image& self, dip::Range const& range, dip::Image const& source ) { self.At( range ).Copy( source ); } );
   // Assignment into slice for nD image
   img.def( "__setitem__", []( dip::Image& self, dip::RangeArray rangeArray, dip::Image::Pixel const& value ) { self.At( std::move( rangeArray )).Fill( value ); } );
   img.def( "__setitem__", []( dip::Image& self, dip::RangeArray rangeArray, dip::Image const& source ) { self.At( std::move( rangeArray )).Copy( source ); } );
   // Assignment using a mask image
   img.def( "__setitem__", []( dip::Image& self, dip::Image mask, dip::Image::Pixel const& pixel ) { self.At( std::move( mask )).Fill( pixel ); } );
   img.def( "__setitem__", []( dip::Image& self, dip::Image mask, dip::Image const& source ) { self.At( std::move( mask )).Copy( source ); } );
   // Assignment using a list of coordinates
   img.def( "__setitem__", []( dip::Image& self, dip::CoordinateArray const& coordinates, dip::Image::Pixel const& pixel ) { self.At( coordinates ).Fill( pixel ); } );
   img.def( "__setitem__", []( dip::Image& self, dip::CoordinateArray const& coordinates, dip::Image const& source ) { self.At( coordinates ).Copy( source ); } );

   // Operators
   // +, +=
   img.def( "__iadd__", []( dip::Image& self, py::object const& rhs ) {
      dip::Add( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·Add·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__add__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Add( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Add·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__radd__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Add( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Add·Image·CL·Image·CL·Image·L·DataType· );
   // -, -=
   img.def( "__isub__", []( dip::Image& self, py::object const& rhs ) {
      dip::Subtract( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·Subtract·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__sub__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Subtract( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Subtract·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__rsub__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Subtract( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Subtract·Image·CL·Image·CL·Image·L·DataType· );
   // *, *=
   img.def( "__imul__", []( dip::Image& self, py::object const& rhs ) {
      dip::MultiplySampleWise( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·MultiplySampleWise·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__mul__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::MultiplySampleWise( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·MultiplySampleWise·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__rmul__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::MultiplySampleWise( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·MultiplySampleWise·Image·CL·Image·CL·Image·L·DataType· );
   // @, @=
   img.def( "__imatmul__", []( dip::Image& self, py::object const& rhs ) {
      dip::Multiply( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·Multiply·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__matmul__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Multiply( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Multiply·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__rmatmul__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Multiply( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Multiply·Image·CL·Image·CL·Image·L·DataType· );
   // /, /=
   img.def( "__itruediv__", []( dip::Image& self, py::object const& rhs ) {
      dip::Divide( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·Divide·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__truediv__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Divide( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Divide·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__rtruediv__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Divide( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Divide·Image·CL·Image·CL·Image·L·DataType· );
   // %, %=
   img.def( "__imod__", []( dip::Image& self, py::object const& rhs ) {
      dip::Modulo( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·Modulo·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__mod__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Modulo( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Modulo·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__rmod__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Modulo( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Modulo·Image·CL·Image·CL·Image·L·DataType· );
   // **, **=
   img.def( "__ipow__", []( dip::Image& self, py::object const& rhs ) {
      dip::Power( self, ImageOrPixel( rhs ), self, self.DataType() );
      return self;
   }, py::is_operator(), doc_strings::dip·Power·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__pow__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Power( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Power·Image·CL·Image·CL·Image·L·DataType· );
   img.def( "__rpow__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Power( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Power·Image·CL·Image·CL·Image·L·DataType· );
   // &, &=
   img.def( "__iand__", []( dip::Image& self, py::object const& rhs ) {
      dip::And( self, ImageOrPixel( rhs ), self );
      return self;
   }, py::is_operator(), doc_strings::dip·And·Image·CL·Image·CL·Image·L );
   img.def( "__and__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::And( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·And·Image·CL·Image·CL·Image·L );
   img.def( "__rand__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::And( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·And·Image·CL·Image·CL·Image·L );
   // |, |=
   img.def( "__ior__", []( dip::Image& self, py::object const& rhs ) {
      dip::Or( self, ImageOrPixel( rhs ), self );
      return self;
   }, py::is_operator(), doc_strings::dip·Or·Image·CL·Image·CL·Image·L );
   img.def( "__or__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Or( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Or·Image·CL·Image·CL·Image·L );
   img.def( "__ror__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Or( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Or·Image·CL·Image·CL·Image·L );
   // ^, ^=
   img.def( "__ixor__", []( dip::Image& self, py::object const& rhs ) {
      dip::Xor( self, ImageOrPixel( rhs ), self );
      return self;
   }, py::is_operator(), doc_strings::dip·Xor·Image·CL·Image·CL·Image·L );
   img.def( "__xor__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Xor( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Xor·Image·CL·Image·CL·Image·L );
   img.def( "__rxor__", []( dip::Image const& rhs, py::object const& lhs ) { return dip::Xor( ImageOrPixel( lhs ), rhs ); }, py::is_operator(), doc_strings::dip·Xor·Image·CL·Image·CL·Image·L );
   // +img
   img.def( "__pos__", []( dip::Image const& self ) { return dip::operator+( self ); }, py::is_operator(), doc_strings::dip·operatorplus·Image·CL );
   // -img
   img.def( "__neg__", []( dip::Image const& self ) { return dip::Invert( self ); }, py::is_operator(), doc_strings::dip·Invert·Image·CL·Image·L );
   // ~img
   img.def( "__invert__", []( dip::Image const& self ) { return dip::Not( self ); }, py::is_operator(), doc_strings::dip·Not·Image·CL·Image·L );
   // ==, !=, >, >=, <, <=
   img.def( "__eq__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Equal( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Equal·Image·CL·Image·CL·Image·L );
   img.def( "__ne__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::NotEqual( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·NotEqual·Image·CL·Image·CL·Image·L );
   img.def( "__gt__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Greater( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Greater·Image·CL·Image·CL·Image·L );
   img.def( "__ge__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::NotLesser( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·NotLesser·Image·CL·Image·CL·Image·L );
   img.def( "__lt__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::Lesser( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·Lesser·Image·CL·Image·CL·Image·L );
   img.def( "__le__", []( dip::Image const& lhs, py::object const& rhs ) { return dip::NotGreater( lhs, ImageOrPixel( rhs )); }, py::is_operator(), doc_strings::dip·NotGreater·Image·CL·Image·CL·Image·L );

   // Some new functions useful in Python
   m.def( "Create0D", []( dip::Image::Pixel const& in ) -> dip::Image {
             return dip::Image( in );
          },
          "Function that creates a 0D image from a scalar or tensor value." );
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
          },
          "Overload that takes a scalar image, the pixel values are the values for each\n"
          "tensor element of the output 0D image." );

   // Free functions in diplib/library/image.h
   m.def( "Copy", py::overload_cast< dip::Image const& >( &dip::Copy ), "src"_a, doc_strings::dip·Copy·Image·CL·Image·L );
   m.def( "Copy", py::overload_cast< dip::Image const&, dip::Image& >( &dip::Copy ), "src"_a, py::kw_only(), "dest"_a, doc_strings::dip·Copy·Image·CL·Image·L );
   m.def( "ExpandTensor", py::overload_cast< dip::Image const& >( &dip::ExpandTensor ), "src"_a, doc_strings::dip·ExpandTensor·Image·CL·Image·L );
   m.def( "ExpandTensor", py::overload_cast< dip::Image const&, dip::Image& >( &dip::ExpandTensor ), "src"_a, py::kw_only(), "dest"_a, doc_strings::dip·ExpandTensor·Image·CL·Image·L );
   m.def( "Convert", py::overload_cast< dip::Image const&, dip::DataType >( &dip::Convert ), "src"_a, "dt"_a, doc_strings::dip·Convert·Image·CL·Image·L·dip·DataType· );
   m.def( "Convert", py::overload_cast< dip::Image const&, dip::Image&, dip::DataType >( &dip::Convert ), "src"_a, py::kw_only(), "dest"_a, "dt"_a, doc_strings::dip·Convert·Image·CL·Image·L·dip·DataType· );

   m.def( "ReverseDimensions", []() { reverseDimensions = false; },
          "By default, DIPlib uses the (x,y,z) index order. This order is reversed from\n"
          "how NumPy (and, by extension, packages such as scikit-image, matplotlib and\n"
          "imageio) index. Calling `ReverseDimensions()` causes DIPlib, for the remainder\n"
          "of the session, to also follow the (z,y,x) index order, giving dimensions the\n"
          "same order as Python users might be used to. Use this function at the top of\n"
          "your program, right after importing the `diplib` package. There is no way to\n"
          "revert to the default order. Please don't try to mix dimension ordering within\n"
          "your program.\n\nSee dip.AreDimensionsReversed()." );
   m.def( "AreDimensionsReversed", []() { return reverseDimensions; },
          "Shows the status of the dimension order flag, see dip.ReverseDimensions()." );

   m.def( "SetTensorConversionThreshold", []( dip::uint n ) { tensorConversionThreshold = n; },
          "n"_a = 4,
          "Sets the threshold for the size of array dimension to be converted to the image\n"
          "tensor dimension, used when converting a NumPy array to a DIPlib image." );

}
