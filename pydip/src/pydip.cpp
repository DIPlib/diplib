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

#include <sstream>

#include "pydip.h"
#include "diplib/neighborlist.h"
#include "diplib/multithreading.h"
#include "diplib/random.h"

#if defined(__clang__)
   // Clang gives a bogus diagnostic here for `py::self -= py::self`
   #pragma GCC diagnostic push
   #pragma GCC diagnostic ignored "-Wself-assign-overloaded"
#endif

static_assert( sizeof( bool ) == sizeof( dip::bin ), "bool is not one byte, how can I work with logical Python buffers?" );

namespace {

dip::String TensorRepr( dip::Tensor const& tensor ) {
   std::ostringstream os;
   os << "<Tensor (" << tensor << ")>";
   return os.str();
}

dip::String MetricRepr( dip::Metric const& s ) {
   std::ostringstream os;
   os << '<';
   switch( s.Type() ) {
      case dip::Metric::TypeCode::CHAMFER:
         os << "Chamfer";
         break;
      case dip::Metric::TypeCode::CONNECTED:
         os << "Connected";
         break;
      case dip::Metric::TypeCode::IMAGE:
         os << "Custom";
         break;
      default:
         os << "Unknown";
         break;
   }
   os << " Metric";
   if( s.Type() != dip::Metric::TypeCode::IMAGE ) {
      os << " with parameter " << s.Param();
   }
   os << '>';
   return os.str();
}

} // namespace

dip::Random& RandomNumberGenerator() {
   static dip::Random generator;
   return generator;
}

tsl::robin_map< std::string, py::object > map;

PYBIND11_MODULE( PyDIP_bin, m ) {
   m.doc() = "The portion of the PyDIP module that contains the C++ DIPlib bindings.";

   // diplib.h
   m.attr( "libraryInformation" ) = CreateNamedTuple(
      "LibraryInformation",
      "name description copyright URL version date type isReleaseBuild usingOpenMP stackTracesEnabled assertsEnabled usingUnicode hasICS hasTIFF hasJPEG hasPNG usingFFTW usingFreeType",
      dip::libraryInformation.name,
      dip::libraryInformation.description,
      dip::libraryInformation.copyright,
      dip::libraryInformation.URL,
      dip::libraryInformation.version,
      dip::libraryInformation.date,
      dip::libraryInformation.type,
      dip::libraryInformation.isReleaseBuild,
      dip::libraryInformation.usingOpenMP,
      dip::libraryInformation.stackTracesEnabled,
      dip::libraryInformation.assertsEnabled,
      dip::libraryInformation.usingUnicode,
      dip::libraryInformation.hasICS,
      dip::libraryInformation.hasTIFF,
      dip::libraryInformation.hasJPEG,
      dip::libraryInformation.hasPNG,
      dip::libraryInformation.usingFFTW,
      dip::libraryInformation.usingFreeType
   );
   m.attr( "__version__" ) = dip::libraryInformation.version;

   // diplib/library/error.h
   auto error = py::register_exception< dip::Error >( m, "Error" );
   py::register_exception< dip::AssertionError >( m, "AssertionError", error );
   py::register_exception< dip::ParameterError >( m, "ParameterError", error );
   py::register_exception< dip::RunTimeError >( m, "RunTimeError", error );

   // diplib/library/tensor.h
   auto tensor = py::class_< dip::Tensor >( m, "Tensor", doc_strings::dip·Tensor );
   tensor.def( py::init<>(), doc_strings::dip·Tensor·Tensor );
   tensor.def( py::init< dip::uint >(), "n"_a, doc_strings::dip·Tensor·Tensor·dip·uint· );
   tensor.def( py::init< dip::uint, dip::uint >(), "rows"_a, "cols"_a, doc_strings::dip·Tensor·Tensor·dip·uint··dip·uint· );
   tensor.def( py::init< dip::Tensor::Shape, dip::uint, dip::uint >(), "shape"_a, "rows"_a, "cols"_a, doc_strings::dip·Tensor·Tensor·String·CL·dip·uint··dip·uint· );
   tensor.def( "__repr__", &TensorRepr );
   tensor.def( "IsScalar", &dip::Tensor::IsScalar, doc_strings::dip·Tensor·IsScalar·C );
   tensor.def( "IsVector", &dip::Tensor::IsVector, doc_strings::dip·Tensor·IsVector·C );
   tensor.def( "IsDiagonal", &dip::Tensor::IsDiagonal, doc_strings::dip·Tensor·IsDiagonal·C );
   tensor.def( "IsSymmetric", &dip::Tensor::IsSymmetric, doc_strings::dip·Tensor·IsSymmetric·C );
   tensor.def( "IsTriangular", &dip::Tensor::IsTriangular, doc_strings::dip·Tensor·IsTriangular·C );
   tensor.def( "IsSquare", &dip::Tensor::IsSquare, doc_strings::dip·Tensor·IsSquare·C );
   tensor.def( "TensorShape", &dip::Tensor::TensorShape, doc_strings::dip·Tensor·TensorShape·C );
   tensor.def( "Elements", &dip::Tensor::Elements, doc_strings::dip·Tensor·Elements·C );
   tensor.def( "Rows", &dip::Tensor::Rows, doc_strings::dip·Tensor·Rows·C );
   tensor.def( "Columns", &dip::Tensor::Columns, doc_strings::dip·Tensor·Columns·C );
   tensor.def( "Sizes", &dip::Tensor::Sizes, doc_strings::dip·Tensor·Sizes·C );
   tensor.def( "SetShape", &dip::Tensor::SetShape, "shape"_a, "rows"_a, "cols"_a, doc_strings::dip·Tensor·SetShape·Shape··dip·uint··dip·uint· );
   tensor.def( "SetScalar", &dip::Tensor::SetScalar, doc_strings::dip·Tensor·SetScalar );
   tensor.def( "SetVector", &dip::Tensor::SetVector, "n"_a, doc_strings::dip·Tensor·SetVector·dip·uint· );
   tensor.def( "SetMatrix", &dip::Tensor::SetMatrix, "rows"_a, "cols"_a, doc_strings::dip·Tensor·SetMatrix·dip·uint··dip·uint· );
   tensor.def( "SetSizes", &dip::Tensor::SetSizes, "sizes"_a, doc_strings::dip·Tensor·SetSizes·UnsignedArray·CL );
   tensor.def( "ChangeShape", py::overload_cast<>( &dip::Tensor::ChangeShape), doc_strings::dip·Tensor·ChangeShape );
   tensor.def( "ChangeShape", py::overload_cast< dip::uint >( &dip::Tensor::ChangeShape ), "rows"_a, doc_strings::dip·Tensor·ChangeShape·dip·uint· );
   tensor.def( "ChangeShape", py::overload_cast< dip::Tensor const& >( &dip::Tensor::ChangeShape ), "example"_a, doc_strings::dip·Tensor·ChangeShape·Tensor·CL );
   tensor.def( "Transpose", &dip::Tensor::Transpose, doc_strings::dip·Tensor·Transpose );
   tensor.def( "ExtractDiagonal", &dip::Tensor::ExtractDiagonal, "stride"_a, doc_strings::dip·Tensor·ExtractDiagonal·dip·sint·L );
   tensor.def( "ExtractRow", &dip::Tensor::ExtractRow, "index"_a, "stride"_a, doc_strings::dip·Tensor·ExtractRow·dip·uint··dip·sint·L );
   tensor.def( "ExtractColumn", &dip::Tensor::ExtractColumn, "index"_a, "stride"_a, doc_strings::dip·Tensor·ExtractColumn·dip·uint··dip·sint·L );
   tensor.def( "HasNormalOrder", &dip::Tensor::HasNormalOrder, doc_strings::dip·Tensor·HasNormalOrder·C );
   tensor.def( "Index", &dip::Tensor::Index, "indices"_a, doc_strings::dip·Tensor·Index·UnsignedArray·CL·C );
   tensor.def( "LookUpTable", &dip::Tensor::LookUpTable, doc_strings::dip·Tensor·LookUpTable·C );
   tensor.def( py::self == py::self, doc_strings::dip·Tensor·operatoreqeq·Tensor·CL·C ); // NOLINT(*-redundant-expression)
   tensor.def( py::self != py::self, doc_strings::dip·Tensor·operatornoteq·Tensor·CL·C ); // NOLINT(*-redundant-expression)

   // diplib/library/physical_dimensions.h
   auto units = py::class_< dip::Units >( m, "Units", doc_strings::dip·Units );
   units.def( py::init<>(), doc_strings::dip·Units·Units );
   units.def( py::init< dip::String const& >(), "string"_a, doc_strings::dip·Units·Units·dip·String·CL );
   units.def( "__repr__", &dip::Units::StringUnicode );
   units.def( "__bool__", &dip::Units::operator bool );
   units.def( dip::dfloat() * py::self, doc_strings::dip·operatortimes·dip·dfloat··Units·CL );
   units.def( py::self * dip::dfloat(), doc_strings::dip·operatortimes·Units·CL·dip·dfloat· );
   units.def( "HasSameDimensions", &dip::Units::HasSameDimensions, doc_strings::dip·Units·HasSameDimensions·Units·CL·C );
   units.def( "IsDimensionless", &dip::Units::IsDimensionless, doc_strings::dip·Units·IsDimensionless·C );
   units.def( "IsPhysical", &dip::Units::IsPhysical, doc_strings::dip·Units·IsPhysical·C );
   units.def( "AdjustThousands", &dip::Units::AdjustThousands, doc_strings::dip·Units·AdjustThousands·dip·sint· );
   units.def( "Thousands", &dip::Units::Thousands, doc_strings::dip·Units·Thousands·C );
   py::implicitly_convertible< py::str, dip::Units >();

   auto physQ = py::class_< dip::PhysicalQuantity >( m, "PhysicalQuantity", doc_strings::dip·PhysicalQuantity );
   physQ.def( py::init<>(), doc_strings::dip·PhysicalQuantity·PhysicalQuantity );
   physQ.def( py::init< dip::dfloat, dip::Units >(), "magnitude"_a, "units"_a = dip::Units{}, doc_strings::dip·PhysicalQuantity·PhysicalQuantity·dip·dfloat··Units·CL );
   physQ.def( py::init< dip::Units >(), "units"_a, doc_strings::dip·PhysicalQuantity·PhysicalQuantity·Units·CL );
   physQ.def( "__repr__", []( dip::PhysicalQuantity const& self ) {
      std::ostringstream os;
      os << "<PhysicalQuantity {" << self << "}>";
      return os.str();
   } );
   physQ.def( "__str__", []( dip::PhysicalQuantity const& self ) {
      std::ostringstream os;
      os << self;
      return os.str();
   } );
   physQ.def_readwrite( "magnitude", &dip::PhysicalQuantity::magnitude, doc_strings::dip·PhysicalQuantity·magnitude );
   physQ.def_readwrite( "units", &dip::PhysicalQuantity::units, doc_strings::dip·PhysicalQuantity·units );
   physQ.def( py::self += py::self, doc_strings::dip·PhysicalQuantity·operatorpluseq·PhysicalQuantity·CL );
   physQ.def( py::self + py::self, doc_strings::dip·operatorplus·PhysicalQuantity··PhysicalQuantity·CL );
   physQ.def( py::self -= py::self, doc_strings::dip·PhysicalQuantity·operatorminuseq·PhysicalQuantity· );
   physQ.def( py::self - py::self, doc_strings::dip·operatorminus·PhysicalQuantity··PhysicalQuantity·CL ); // NOLINT(*-redundant-expression)
   physQ.def( py::self *= py::self, doc_strings::dip·PhysicalQuantity·operatortimeseq·PhysicalQuantity·CL );
   physQ.def( py::self *= dip::dfloat(), doc_strings::dip·PhysicalQuantity·operatortimeseq·dip·dfloat· );
   physQ.def( py::self * py::self, doc_strings::dip·operatortimes·PhysicalQuantity··PhysicalQuantity·CL );
   physQ.def( py::self * dip::dfloat(), doc_strings::dip·operatortimes·PhysicalQuantity··dip·dfloat· );
   physQ.def( dip::dfloat() * py::self, doc_strings::dip·operatortimes·dip·dfloat··PhysicalQuantity· );
   physQ.def( py::self /= py::self, doc_strings::dip·PhysicalQuantity·operatordiveq·PhysicalQuantity·CL );
   physQ.def( py::self /= dip::dfloat(), doc_strings::dip·PhysicalQuantity·operatordiveq·dip·dfloat· );
   physQ.def( py::self / py::self, doc_strings::dip·operatordiv·PhysicalQuantity··PhysicalQuantity·CL ); // NOLINT(*-redundant-expression)
   physQ.def( py::self / dip::dfloat(), doc_strings::dip·operatordiv·PhysicalQuantity··dip·dfloat· );
   physQ.def( dip::dfloat() / py::self, doc_strings::dip·operatordiv·dip·dfloat··PhysicalQuantity· );
   physQ.def( "__pow__", []( dip::PhysicalQuantity a, dip::sint8 p ) { return a.Power( p ); }, py::is_operator(), doc_strings::dip·PhysicalQuantity·Power·dip·sint8··C );
   physQ.def( py::self == py::self, doc_strings::dip·PhysicalQuantity·operatoreqeq·PhysicalQuantity·CL·C ); // NOLINT(*-redundant-expression)
   physQ.def( py::self != py::self, doc_strings::dip·PhysicalQuantity·operatornoteq·PhysicalQuantity·CL·C ); // NOLINT(*-redundant-expression)
   physQ.def( -py::self, doc_strings::dip·PhysicalQuantity·operatorminus·C );
   physQ.def( "Invert", &dip::PhysicalQuantity::Invert, doc_strings::dip·PhysicalQuantity·Invert·C );
   physQ.def( "IsDimensionless", &dip::PhysicalQuantity::IsDimensionless, doc_strings::dip·PhysicalQuantity·IsDimensionless·C );
   physQ.def( "IsPhysical", &dip::PhysicalQuantity::IsPhysical, doc_strings::dip·PhysicalQuantity·IsPhysical·C );
   physQ.def( "Normalize", &dip::PhysicalQuantity::Normalize, py::return_value_policy::reference_internal, doc_strings::dip·PhysicalQuantity·Normalize );
   physQ.def( "RemovePrefix", &dip::PhysicalQuantity::RemovePrefix, py::return_value_policy::reference_internal, doc_strings::dip·PhysicalQuantity·RemovePrefix );
   py::implicitly_convertible< py::float_, dip::PhysicalQuantity >();
   py::implicitly_convertible< dip::Units, dip::PhysicalQuantity >();
   py::implicitly_convertible< py::str, dip::PhysicalQuantity >();

   auto pixSz = py::class_< dip::PixelSize >( m, "PixelSize", doc_strings::dip·PixelSize );
   pixSz.def( py::init<>(), doc_strings::dip·PixelSize·PixelSize );
   pixSz.def( py::init< dip::PhysicalQuantityArray const& >(), "physicalQuantities"_a, doc_strings::dip·PixelSize·PixelSize·PhysicalQuantity·CL );
   pixSz.def( py::init( []( dip::dfloat mag, dip::Units const& units ) { return dip::PhysicalQuantity( mag, units ); } ),
              "magnitude"_a, "units"_a = dip::Units{},
              "Overload that accepts the two components of a `dip.PhysicalQuantity`, sets\n"
              "all dimensions to the same value." );
   pixSz.def( py::init( []( dip::FloatArray const& mags, dip::Units const& units ) {
                 dip::PhysicalQuantityArray pq( mags.size() );
                 for( dip::uint ii = 0; ii < mags.size(); ++ii ) {
                    pq[ ii ] = mags[ ii ] * units;
                 }
                 return pq;
              } ),
              "magnitudes"_a, "units"_a = dip::Units{},
              "Overload that accepts the two components of a `dip.PhysicalQuantity`, using\n"
              "a different magnitude for each dimension." );
   pixSz.def( "__repr__", []( dip::PixelSize const& self ) {
      std::ostringstream os;
      os << "<PixelSize " << self << '>';
      return os.str();
   } );
   pixSz.def( "__str__", []( dip::PixelSize const& self ) {
      std::ostringstream os;
      os << self;
      return os.str();
   } );
   pixSz.def( "__len__", []( dip::PixelSize const& self ) { return self.Size(); } );
   pixSz.def( "__getitem__", &dip::PixelSize::Get );
   pixSz.def( "__setitem__", py::overload_cast< dip::uint, dip::PhysicalQuantity >( &dip::PixelSize::Set ));
   pixSz.def( py::self == py::self, doc_strings::dip·PixelSize·operatoreqeq·PixelSize·CL·C ); // NOLINT(*-redundant-expression)
   pixSz.def( py::self != py::self, doc_strings::dip·PixelSize·operatornoteq·PixelSize·CL·C ); // NOLINT(*-redundant-expression)
   pixSz.def( "Scale", py::overload_cast< dip::uint, dip::dfloat >( &dip::PixelSize::Scale ), "d"_a, "s"_a, doc_strings::dip·PixelSize·Scale·dip·uint··dip·dfloat· );
   pixSz.def( "Scale", py::overload_cast< dip::dfloat >( &dip::PixelSize::Scale ), "s"_a, doc_strings::dip·PixelSize·Scale·dip·dfloat· );
   pixSz.def( "Scale", py::overload_cast< dip::FloatArray const& >( &dip::PixelSize::Scale ), "s"_a, doc_strings::dip·PixelSize·Scale·FloatArray·CL );
   pixSz.def( "Invert", py::overload_cast< dip::uint >( &dip::PixelSize::Invert ), "d"_a, doc_strings::dip·PixelSize·Invert·dip·uint· );
   pixSz.def( "Invert", py::overload_cast<>( &dip::PixelSize::Invert ), doc_strings::dip·PixelSize·Invert);
   pixSz.def( "IsIsotropic", &dip::PixelSize::IsIsotropic, doc_strings::dip·PixelSize·IsIsotropic·C );
   pixSz.def( "AspectRatio", &dip::PixelSize::AspectRatio, "d"_a, doc_strings::dip·PixelSize·AspectRatio·dip·uint··C );
   pixSz.def( "IsDefined", &dip::PixelSize::IsDefined, doc_strings::dip·PixelSize·IsDefined·C );
   pixSz.def( "SameUnits", &dip::PixelSize::SameUnits, doc_strings::dip·PixelSize·SameUnits·C );
   pixSz.def( "Product", &dip::PixelSize::Product, "d"_a, doc_strings::dip·PixelSize·Product·dip·uint··C );
   pixSz.def( "UnitLength", &dip::PixelSize::UnitLength, doc_strings::dip·PixelSize·UnitLength·C );
   pixSz.def( "UnitSize", &dip::PixelSize::UnitSize, "d"_a, doc_strings::dip·PixelSize·UnitSize·dip·uint··C );
   pixSz.def( "ForcePhysical", &dip::PixelSize::ForcePhysical, doc_strings::dip·PixelSize·ForcePhysical );
   pixSz.def( "ApproximatelyEquals", &dip::PixelSize::ApproximatelyEquals, "rhs"_a, "nDims"_a, "tolerance"_a = 1e-6, doc_strings::dip·PixelSize·ApproximatelyEquals·PixelSize·CL·dip·uint··double··C );
   pixSz.def( "ToPixels", &dip::PixelSize::ToPixels, "in"_a, doc_strings::dip·PixelSize·ToPixels·PhysicalQuantityArray·CL·C );
   pixSz.def( "ToPhysical", &dip::PixelSize::ToPhysical, "in"_a, doc_strings::dip·PixelSize·ToPhysical·FloatArray·CL·C );
   py::implicitly_convertible< dip::PhysicalQuantity, dip::PixelSize >();
   py::implicitly_convertible< dip::PhysicalQuantityArray, dip::PixelSize >();

   // diplib/neighborlist.h
   auto metric = py::class_< dip::Metric >( m, "Metric", doc_strings::dip·Metric );
   metric.def( py::init<>(), doc_strings::dip·Metric·Metric·TypeCode··dip·uint· );
   metric.def( py::init< dip::Image const& >(), "image"_a , doc_strings::dip·Metric·Metric·dip·Image·CL);
   metric.def( py::init< dip::String const&, dip::uint, dip::PixelSize const& >(), "type"_a, "param"_a = 1, "pixelSize"_a = dip::PixelSize{}, doc_strings::dip·Metric·Metric·String·CL·dip·uint··dip·PixelSize·CL );
   metric.def( "__repr__", &MetricRepr );
   py::implicitly_convertible< py::buffer, dip::Metric >();
   py::implicitly_convertible< py::str, dip::Metric >();

   // diplib/multithreading.h
   m.def( "SetNumberOfThreads", &dip::SetNumberOfThreads, "nThreads"_a, doc_strings::dip·SetNumberOfThreads·dip·uint· );
   m.def( "GetNumberOfThreads", &dip::GetNumberOfThreads, doc_strings::dip·GetNumberOfThreads );

   // Include definitions from all other source files
   init_image( m );
   init_math( m );
   init_statistics( m );
   init_filtering( m );
   init_morphology( m );
   init_analysis( m );
   init_segmentation( m );
   init_measurement( m );
   init_histogram( m );
   init_generation( m );
   init_assorted( m );

}

#if defined(__clang__)
   #pragma GCC diagnostic pop
#endif
