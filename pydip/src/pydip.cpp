/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022, Cris Luengo.
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

dip::String InfoString( dip::LibraryInformation const& info ) {
   std::ostringstream os;
   os << "name: " << info.name << '\n'
      << "description: " << info.description << '\n'
      << "copyright: " << info.copyright << '\n'
      << "URL: " << info.URL << '\n'
      << "version: " << info.version << '\n'
      << "date: " << info.date << '\n'
      << "type: " << info.type << '\n';
   return os.str();
}

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

PYBIND11_MODULE( PyDIP_bin, m ) {
   m.doc() = "The portion of the PyDIP module that contains the C++ DIPlib bindings.";

   // diplib.h
   auto info = py::class_< dip::LibraryInformation >( m, "LibraryInformation", "Holds information about the DIPlib library." );
   info.def_readonly( "name", &dip::LibraryInformation::name, "The library name" );
   info.def_readonly( "description", &dip::LibraryInformation::description, "A short description string" );
   info.def_readonly( "copyright", &dip::LibraryInformation::copyright, "Copyright string for the library" );
   info.def_readonly( "URL", &dip::LibraryInformation::URL, "Library website, with contact information etc." );
   info.def_readonly( "version", &dip::LibraryInformation::version, "The library version number" );
   info.def_readonly( "date", &dip::LibraryInformation::date, "Compilation date" );
   info.def_readonly( "type", &dip::LibraryInformation::type, "Describes options enabled during compilation" );
   info.def( "__repr__", []( dip::LibraryInformation const& ) { return "<LibraryInformation>"; } );
   info.def( "__str__", &InfoString );

   m.attr( "libraryInformation" ) = dip::libraryInformation;
   m.attr( "__version__" ) = dip::libraryInformation.version;

   // diplib/library/error.h
   auto error = py::register_exception< dip::Error >( m, "Error" );
   py::register_exception< dip::AssertionError >( m, "AssertionError", error );
   py::register_exception< dip::ParameterError >( m, "ParameterError", error );
   py::register_exception< dip::RunTimeError >( m, "RunTimeError", error );

   // diplib/library/types.h
   // dip::RegressionParameters defined in histogram.cpp

   auto quartiles = py::class_< dip::QuartilesResult >( m, "QuartilesResult", "Quartiles." );
   quartiles.def( "__repr__", []( dip::QuartilesResult const& s ) {
      std::ostringstream os;
      os << "<QuartilesResult: minimum=" << s.minimum << ", lowerQuartile=" << s.lowerQuartile
         << ", median=" << s.median << ", upperQuartile=" << s.upperQuartile << ", maximum=" << s.maximum << '>';
      return os.str();
   } );
   quartiles.def_readonly( "minimum", &dip::QuartilesResult::minimum );
   quartiles.def_readonly( "lowerQuartile", &dip::QuartilesResult::lowerQuartile );
   quartiles.def_readonly( "median", &dip::QuartilesResult::median );
   quartiles.def_readonly( "upperQuartile", &dip::QuartilesResult::upperQuartile );
   quartiles.def_readonly( "maximum", &dip::QuartilesResult::maximum );


   // diplib/library/tensor.h
   auto tensor = py::class_< dip::Tensor >( m, "Tensor", "Represents the tensor size and shape." );
   tensor.def( py::init<>() );
   tensor.def( py::init< dip::uint >(), "n"_a );
   tensor.def( py::init< dip::uint, dip::uint >(), "rows"_a, "cols"_a );
   tensor.def( py::init< dip::Tensor::Shape, dip::uint, dip::uint >(), "shape"_a, "rows"_a, "cols"_a );
   tensor.def( "__repr__", &TensorRepr );
   tensor.def( "IsScalar", &dip::Tensor::IsScalar );
   tensor.def( "IsVector", &dip::Tensor::IsVector );
   tensor.def( "IsDiagonal", &dip::Tensor::IsDiagonal );
   tensor.def( "IsSymmetric", &dip::Tensor::IsSymmetric );
   tensor.def( "IsTriangular", &dip::Tensor::IsTriangular );
   tensor.def( "IsSquare", &dip::Tensor::IsSquare );
   tensor.def( "TensorShape", &dip::Tensor::TensorShape );
   tensor.def( "Elements", &dip::Tensor::Elements );
   tensor.def( "Rows", &dip::Tensor::Rows );
   tensor.def( "Columns", &dip::Tensor::Columns );
   tensor.def( "Sizes", &dip::Tensor::Sizes );
   tensor.def( "SetShape", &dip::Tensor::SetShape, "shape"_a, "rows"_a, "cols"_a );
   tensor.def( "SetScalar", &dip::Tensor::SetScalar );
   tensor.def( "SetVector", &dip::Tensor::SetVector, "n"_a );
   tensor.def( "SetMatrix", &dip::Tensor::SetMatrix, "rows"_a, "cols"_a );
   tensor.def( "SetSizes", &dip::Tensor::SetSizes, "sizes"_a );
   tensor.def( "ChangeShape", py::overload_cast<>( &dip::Tensor::ChangeShape ));
   tensor.def( "ChangeShape", py::overload_cast< dip::uint >( &dip::Tensor::ChangeShape ), "rows"_a );
   tensor.def( "ChangeShape", py::overload_cast< dip::Tensor const& >( &dip::Tensor::ChangeShape ), "example"_a );
   tensor.def( "Transpose", &dip::Tensor::Transpose );
   tensor.def( "ExtractDiagonal", &dip::Tensor::ExtractDiagonal, "stride"_a );
   tensor.def( "ExtractRow", &dip::Tensor::ExtractRow, "index"_a, "stride"_a );
   tensor.def( "ExtractColumn", &dip::Tensor::ExtractColumn, "index"_a, "stride"_a );
   tensor.def( "HasNormalOrder", &dip::Tensor::HasNormalOrder );
   tensor.def( "Index", &dip::Tensor::Index, "indices"_a );
   tensor.def( "LookUpTable", &dip::Tensor::LookUpTable );
   tensor.def( py::self == py::self ); // NOLINT(*-redundant-expression)
   tensor.def( py::self != py::self ); // NOLINT(*-redundant-expression)

   // diplib/library/physical_dimensions.h
   auto units = py::class_< dip::Units >( m, "Units", "Represents physical units." );
   units.def( py::init<>() );
   units.def( py::init< dip::String const& >(), "string"_a );
   units.def( "__repr__", &dip::Units::StringUnicode );
   units.def( "__bool__", &dip::Units::operator bool );
   units.def( dip::dfloat() * py::self ); // dip.PhysicalQuantity generated by float * dip.Units
   units.def( py::self * dip::dfloat() ); // dip.PhysicalQuantity generated by dip.Units * float
   units.def( "HasSameDimensions", &dip::Units::HasSameDimensions );
   units.def( "IsDimensionless", &dip::Units::IsDimensionless );
   units.def( "IsPhysical", &dip::Units::IsPhysical );
   units.def( "AdjustThousands", &dip::Units::AdjustThousands );
   units.def( "Thousands", &dip::Units::Thousands );
   py::implicitly_convertible< py::str, dip::Units >();

   auto physQ = py::class_< dip::PhysicalQuantity >( m, "PhysicalQuantity", "Represents a physical quantity." );
   physQ.def( py::init<>() );
   physQ.def( py::init< dip::dfloat, dip::Units >(), "magnitude"_a, "units"_a = dip::Units{} );
   physQ.def( py::init< dip::Units >(), "units"_a );
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
   physQ.def_readwrite( "magnitude", &dip::PhysicalQuantity::magnitude );
   physQ.def_readwrite( "units", &dip::PhysicalQuantity::units );
   physQ.def( py::self += py::self );
   physQ.def( py::self + py::self );
   physQ.def( py::self -= py::self );
   physQ.def( py::self - py::self ); // NOLINT(*-redundant-expression)
   physQ.def( py::self *= py::self );
   physQ.def( py::self *= dip::dfloat() );
   physQ.def( py::self * py::self );
   physQ.def( py::self * dip::dfloat() );
   physQ.def( dip::dfloat() * py::self );
   physQ.def( py::self /= py::self );
   physQ.def( py::self /= dip::dfloat() );
   physQ.def( py::self / py::self ); // NOLINT(*-redundant-expression)
   physQ.def( py::self / dip::dfloat() );
   physQ.def( dip::dfloat() / py::self );
   physQ.def( "__pow__", []( dip::PhysicalQuantity a, dip::sint8 p ) { return a.Power( p ); }, py::is_operator() );
   physQ.def( py::self == py::self ); // NOLINT(*-redundant-expression)
   physQ.def( py::self != py::self ); // NOLINT(*-redundant-expression)
   physQ.def( -py::self );
   physQ.def( "Invert", &dip::PhysicalQuantity::Invert );
   physQ.def( "IsDimensionless", &dip::PhysicalQuantity::IsDimensionless );
   physQ.def( "IsPhysical", &dip::PhysicalQuantity::IsPhysical );
   physQ.def( "Normalize", &dip::PhysicalQuantity::Normalize, py::return_value_policy::reference_internal );
   physQ.def( "RemovePrefix", &dip::PhysicalQuantity::RemovePrefix, py::return_value_policy::reference_internal );
   py::implicitly_convertible< py::float_, dip::PhysicalQuantity >();
   py::implicitly_convertible< dip::Units, dip::PhysicalQuantity >();
   py::implicitly_convertible< py::str, dip::PhysicalQuantity >();

   auto pixSz = py::class_< dip::PixelSize >( m, "PixelSize", "Represents the physical size of a pixel." );
   pixSz.def( py::init<>() );
   pixSz.def( py::init< dip::PhysicalQuantityArray const& >(), "physicalQuantities"_a );
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
   pixSz.def( py::self == py::self ); // NOLINT(*-redundant-expression)
   pixSz.def( py::self != py::self ); // NOLINT(*-redundant-expression)
   pixSz.def( "Scale", py::overload_cast< dip::uint, dip::dfloat >( &dip::PixelSize::Scale ), "d"_a, "s"_a );
   pixSz.def( "Scale", py::overload_cast< dip::dfloat >( &dip::PixelSize::Scale ), "s"_a );
   pixSz.def( "Scale", py::overload_cast< dip::FloatArray const& >( &dip::PixelSize::Scale ), "s"_a );
   pixSz.def( "Invert", py::overload_cast< dip::uint >( &dip::PixelSize::Invert ), "d"_a );
   pixSz.def( "Invert", py::overload_cast<>( &dip::PixelSize::Invert ));
   pixSz.def( "IsIsotropic", &dip::PixelSize::IsIsotropic );
   pixSz.def( "AspectRatio", &dip::PixelSize::AspectRatio, "d"_a );
   pixSz.def( "IsDefined", &dip::PixelSize::IsDefined );
   pixSz.def( "SameUnits", &dip::PixelSize::SameUnits );
   pixSz.def( "Product", &dip::PixelSize::Product, "d"_a );
   pixSz.def( "UnitLength", &dip::PixelSize::UnitLength );
   pixSz.def( "UnitSize", &dip::PixelSize::UnitSize, "d"_a );
   pixSz.def( "ForcePhysical", &dip::PixelSize::ForcePhysical );
   pixSz.def( "ApproximatelyEquals", &dip::PixelSize::ApproximatelyEquals, "rhs"_a, "nDims"_a, "tolerance"_a = 1e-6 );
   pixSz.def( "ToPixels", &dip::PixelSize::ToPixels, "in"_a );
   pixSz.def( "ToPhysical", &dip::PixelSize::ToPhysical, "in"_a );
   py::implicitly_convertible< dip::PhysicalQuantity, dip::PixelSize >();
   py::implicitly_convertible< dip::PhysicalQuantityArray, dip::PixelSize >();

   // diplib/neighborlist.h
   auto metric = py::class_< dip::Metric >( m, "Metric", "Represents the metric to use in some neighbor-based operations." );
   metric.def( py::init<>() );
   metric.def( py::init< dip::Image const& >(), "image"_a );
   metric.def( py::init< dip::String const&, dip::uint, dip::PixelSize const& >(), "type"_a, "param"_a = 1, "pixelSize"_a = dip::PixelSize{} );
   metric.def( "__repr__", &MetricRepr );
   py::implicitly_convertible< py::buffer, dip::Metric >();
   py::implicitly_convertible< py::str, dip::Metric >();

   // diplib/multithreading.h
   m.def( "SetNumberOfThreads", &dip::SetNumberOfThreads, "nThreads"_a );
   m.def( "GetNumberOfThreads", &dip::GetNumberOfThreads );

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
