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

#include <diplib/file_io.h>
#include "pydip.h"
#include "diplib/measurement.h"
#include "diplib/chain_code.h"

namespace pybind11 {
namespace detail {

// Cast Python slice to dip::Range
template<>
class type_caster< dip::VertexFloat > {
   public:
      using type = dip::VertexFloat;
      bool load( handle src, bool ) {
         if( !isinstance< sequence >( src )) {
            return false;
         }
         auto const seq = reinterpret_borrow< sequence >( src );
         if( seq.size() != 2 ) {
            return false;
         }
         if( !PyFloat_Check( seq[ 0 ].ptr() ) || !PyFloat_Check( seq[ 1 ].ptr() )) {
            return false;
         }
         value = { seq[ 0 ].cast< dip::dfloat >(), seq[ 1 ].cast< dip::dfloat >() };
         return true;
      }
      static handle cast( dip::VertexFloat const& src, return_value_policy, handle ) {
         return make_tuple( src.x, src.y ).release();
      }
   PYBIND11_TYPE_CASTER( type, _( "VertexFloat" ));
};

} // namespace detail
} // namespace pybind11

namespace {

dip::MeasurementTool& measurementTool() {
   static dip::MeasurementTool tool;
   return tool;
}

template< typename MeasurementValues >
py::handle MeasurementValuesToList( MeasurementValues values ) {
   py::list list( values.size() );
   py::ssize_t index = 0;
   for( auto& value: values ) {
      PyList_SET_ITEM( list.ptr(), index++, py::cast( value ).release().ptr() );
   }
   return list.release();
}

py::buffer_info MeasurementFeatureToBuffer( dip::Measurement::IteratorFeature& feature ) {
   dip::sint itemsize = static_cast< dip::sint >( sizeof( dip::dfloat ));
   dip::UnsignedArray sizes = { feature.NumberOfObjects(), feature.NumberOfValues() };
   dip::IntegerArray strides = { feature.Stride() * itemsize, itemsize };
   py::buffer_info info{ feature.NumberOfObjects() > 0 ? feature.Data() : nullptr,
                         itemsize, py::format_descriptor< dip::dfloat >::format(),
                         static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
   //std::cout << "--Constructed Python buffer for dip::Measurement::IteratorFeature object.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndim = " << info.ndim << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
   return info;
}

py::buffer_info MeasurementObjectToBuffer( dip::Measurement::IteratorObject& object ) {
   dip::sint itemsize = static_cast< dip::sint >( sizeof( dip::dfloat ));
   dip::UnsignedArray sizes = { 1, object.NumberOfValues() };
   dip::IntegerArray strides = { itemsize, itemsize };
   py::buffer_info info{ object.Data(), itemsize, py::format_descriptor< dip::dfloat >::format(),
                         static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
   //std::cout << "--Constructed Python buffer for dip::Measurement::IteratorObject object.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndim = " << info.ndim << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
   return info;
}

py::buffer_info MeasurementToBuffer( dip::Measurement& msr ) {
   dip::sint itemsize = static_cast< dip::sint >( sizeof( dip::dfloat ));
   dip::UnsignedArray sizes = { msr.NumberOfObjects(), msr.NumberOfValues() };
   dip::IntegerArray strides = { msr.Stride() * itemsize, itemsize };
   py::buffer_info info{ msr.NumberOfObjects() > 0 ? msr.Data() : nullptr,
                         itemsize, py::format_descriptor< dip::dfloat >::format(),
                         static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
   //std::cout << "--Constructed Python buffer for dip::Measurement object.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndim = " << info.ndim << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
   return info;
}

dip::Polygon BufferToPolygon( py::buffer& buf ) {
   py::buffer_info info = buf.request();
   //std::cout << "--Constructing dip::Polygon from Python buffer.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndims = " << info.shape.size() << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
   if( info.format[ 0 ] != py::format_descriptor< dip::dfloat >::c ) {
      // TODO: Cast other types to double floats
      std::cout << "Attempted to convert buffer to dip.Polygon object: data must be double-precision floats.\n";
      DIP_THROW( "Buffer data type not compatible with class Polygon" );
   }
   if(( info.shape.size() != 2 ) || ( info.shape[ 1 ] != 2 )) {
      std::cout << "Attempted to convert buffer to dip.Polygon object: data must have two columns.\n";
      DIP_THROW( "Buffer size not compatible with class Polygon" );
   }
   // Copy data into polygon
   dip::Polygon polygon;
   dip::uint nPoints = static_cast< dip::uint >( info.shape[ 0 ] );
   polygon.vertices.resize( nPoints );
   dip::sint stride = info.strides[ 0 ] / static_cast< dip::sint >( info.itemsize );
   DIP_THROW_IF( stride * static_cast< dip::sint >( info.itemsize ) != info.strides[ 0 ],
                 "Stride of buffer is not an integer multiple of the item size" );
   dip::sint dstride = info.strides[ 1 ] / static_cast< dip::sint >( info.itemsize );
   DIP_THROW_IF( dstride * static_cast< dip::sint >( info.itemsize ) != info.strides[ 1 ],
                 "Stride of buffer is not an integer multiple of the item size" );
   double* ptr = static_cast< double* >( info.ptr );
   for( dip::uint ii = 0; ii < nPoints; ++ii, ptr += stride ) {
      polygon.vertices[ ii ] = { ptr[ 0 ], ptr[ dstride ] };
   }
   return polygon;
}

py::buffer_info PolygonToBuffer( dip::Polygon& polygon ) {
   dip::sint itemsize = static_cast< dip::sint >( sizeof( dip::dfloat ));
   dip::IntegerArray strides = { 2 * itemsize, itemsize };
   dip::UnsignedArray sizes = { polygon.vertices.size(), 2 };
   py::buffer_info info{ polygon.vertices.data(), itemsize, py::format_descriptor< dip::dfloat >::format(),
                         static_cast< py::ssize_t >( sizes.size() ), sizes, strides };
   //std::cout << "--Constructed Python buffer for dip::Polygon object.\n";
   //std::cout << "   info.ptr = " << info.ptr << '\n';
   //std::cout << "   info.format = " << info.format << '\n';
   //std::cout << "   info.ndim = " << info.ndim << '\n';
   //std::cout << "   info.size = " << info.size << '\n';
   //std::cout << "   info.itemsize = " << info.itemsize << '\n';
   //std::cout << "   info.shape[0] = " << info.shape[0] << '\n';
   //std::cout << "   info.shape[1] = " << info.shape[1] << '\n';
   //std::cout << "   info.strides[0] = " << info.strides[0] << '\n';
   //std::cout << "   info.strides[1] = " << info.strides[1] << '\n';
   return info;
}

} // namespace

void init_measurement( py::module& m ) {

   auto mm = m.def_submodule( "MeasurementTool",
                              "A tool to quantify objects in an image.\n\n"
                              "This is a submodule that uses a static `dip::Measurement` object. Functions\n"
                              "defined in this module correspond to the object member functions in C++." );

   // dip::Measurement::FeatureInformation
   auto fInfo = py::class_< dip::Measurement::FeatureInformation >( mm, "FeatureInformation", "Information about one measurement feature." );
   fInfo.def( "__repr__", []( dip::Measurement::FeatureInformation const& self ) {
                 std::ostringstream os;
                 os << "<FeatureInformation: name=" << self.name
                    << ", startColumn=" << self.startColumn
                    << ", numberValues=" << self.numberValues << '>';
                 return os.str();
              } );
   fInfo.def_readonly( "name", &dip::Measurement::FeatureInformation::name );
   fInfo.def_readonly( "startColumn", &dip::Measurement::FeatureInformation::startColumn );
   fInfo.def_readonly( "numberValues", &dip::Measurement::FeatureInformation::numberValues );

   // dip::Feature::ValueInformation
   auto vInfo = py::class_< dip::Feature::ValueInformation >( mm, "ValueInformation", "Information about one measurement feature value." );
   vInfo.def( "__repr__", []( dip::Feature::ValueInformation const& self ) {
                 std::ostringstream os;
                 os << "<ValueInformation: name=" << self.name << ", units=" << self.units << '>';
                 return os.str();
              } );
   vInfo.def_readonly( "name", &dip::Feature::ValueInformation::name );
   vInfo.def_readonly( "units", &dip::Feature::ValueInformation::units );

   // dip::Measurement::IteratorFeature
   auto feat = py::class_< dip::Measurement::IteratorFeature >( mm, "MeasurementFeature", py::buffer_protocol(),
         "A Measurement table column group representing the results for one\nfeature." );
   feat.def_buffer( []( dip::Measurement::IteratorFeature& self ) -> py::buffer_info { return MeasurementFeatureToBuffer( self ); } );
   feat.def( "__repr__", []( dip::Measurement::IteratorFeature const& self ) {
                std::ostringstream os;
                os << "<MeasurementFeature for feature " << self.FeatureName() << " and " << self.NumberOfObjects() << " objects>";
                return os.str();
             } );
   feat.def( "__getitem__", []( dip::Measurement::IteratorFeature const& self, dip::uint objectID ) {
                return MeasurementValuesToList( self[ objectID ] );
             }, "objectID"_a );
   feat.def( "FeatureName", &dip::Measurement::IteratorFeature::FeatureName );
   feat.def( "Values", &dip::Measurement::IteratorFeature::Values );
   feat.def( "NumberOfValues", &dip::Measurement::IteratorFeature::NumberOfValues );
   feat.def( "ObjectExists", &dip::Measurement::IteratorFeature::ObjectExists );
   feat.def( "Objects", &dip::Measurement::IteratorFeature::Objects );
   feat.def( "NumberOfObjects", &dip::Measurement::IteratorFeature::NumberOfObjects );

   // dip::Measurement::IteratorObject
   auto obj = py::class_< dip::Measurement::IteratorObject >( mm, "MeasurementObject", py::buffer_protocol(),
         "A Measurement table row representing the results for one object." );
   obj.def_buffer( []( dip::Measurement::IteratorObject& self ) -> py::buffer_info {
                      return MeasurementObjectToBuffer( self );
                   } );
   obj.def( "__repr__", []( dip::Measurement::IteratorObject const& self ) {
               std::ostringstream os;
               os << "<MeasurementObject with " << self.NumberOfFeatures() << " features for object " << self.ObjectID() << '>';
               return os.str();
            } );
   obj.def( "__getitem__", []( dip::Measurement::IteratorObject const& self, dip::String const& name ) {
               return MeasurementValuesToList( self[ name ] );
            }, "name"_a );
   obj.def( "ObjectID", &dip::Measurement::IteratorObject::ObjectID );
   obj.def( "FeatureExists", &dip::Measurement::IteratorObject::FeatureExists );
   obj.def( "Features", &dip::Measurement::IteratorObject::Features );
   obj.def( "NumberOfFeatures", &dip::Measurement::IteratorObject::NumberOfFeatures );
   obj.def( "Values", py::overload_cast< dip::String const& >( &dip::Measurement::IteratorObject::Values, py::const_ ), "name"_a );
   obj.def( "Values", py::overload_cast<>( &dip::Measurement::IteratorObject::Values, py::const_ ));
   obj.def( "NumberOfValues", py::overload_cast< dip::String const& >( &dip::Measurement::IteratorObject::NumberOfValues, py::const_ ), "name"_a );
   obj.def( "NumberOfValues", py::overload_cast<>( &dip::Measurement::IteratorObject::NumberOfValues, py::const_ ));

   // dip::Measurement
   auto meas = py::class_< dip::Measurement >( m, "Measurement", py::buffer_protocol(),
         "The result of a call to dip.MeasurementTool.Measure, a table with a column\n"
         "group foreach feature and a row for each object." );
   meas.def_buffer( []( dip::Measurement& self ) -> py::buffer_info { return MeasurementToBuffer( self ); } );
   meas.def( "__repr__", []( dip::Measurement const& self ) {
                std::ostringstream os;
                os << "<Measurement with " << self.NumberOfFeatures() << " features for " << self.NumberOfObjects() << " objects>";
                return os.str();
             } );
   meas.def( "__str__", []( dip::Measurement const& self ) { std::ostringstream os; os << self; return os.str(); } );
   meas.def( "__getitem__", py::overload_cast< dip::uint >( &dip::Measurement::operator[], py::const_ ),
             "objectID"_a, py::return_value_policy::reference_internal );
   meas.def( "__getitem__", py::overload_cast< dip::String const& >( &dip::Measurement::operator[], py::const_ ),
             "name"_a, py::return_value_policy::reference_internal );
   meas.def( "FeatureExists", &dip::Measurement::FeatureExists );
   meas.def( "Features", &dip::Measurement::Features );
   meas.def( "NumberOfFeatures", &dip::Measurement::NumberOfFeatures );
   meas.def( "Values", py::overload_cast< dip::String const& >( &dip::Measurement::Values, py::const_ ), "name"_a );
   meas.def( "Values", py::overload_cast<>( &dip::Measurement::Values, py::const_ ));
   meas.def( "NumberOfValues", py::overload_cast< dip::String const& >( &dip::Measurement::NumberOfValues, py::const_ ), "name"_a );
   meas.def( "NumberOfValues", py::overload_cast<>( &dip::Measurement::NumberOfValues, py::const_ ));
   meas.def( "ObjectExists", &dip::Measurement::ObjectExists );
   meas.def( "Objects", &dip::Measurement::Objects );
   meas.def( "NumberOfObjects", &dip::Measurement::NumberOfObjects );
   meas.def( py::self + py::self );

   // dip::MeasurementTool
   mm.def( "Configure", []( dip::String const& feature, dip::String const& parameter, dip::dfloat value ) {
              measurementTool().Configure( feature, parameter, value );
           }, "feature"_a, "parameter"_a, "value"_a );
   mm.def( "Measure", []( dip::Image const& label, dip::Image const& grey, dip::StringArray const& features, dip::UnsignedArray const& objectIDs, dip::uint connectivity ) {
              return measurementTool().Measure( label, grey, features, objectIDs, connectivity );
           }, "label"_a, "grey"_a = dip::Image{}, "features"_a = dip::StringArray{ "Size" }, "objectIDs"_a = dip::StringArray{}, "connectivity"_a = 0 );
   mm.def( "Features", []() {
              auto features = measurementTool().Features();
              std::vector< std::tuple< dip::String, dip::String >> out;
              for( auto const& f : features ) {
                 dip::String description = f.description;
                 if( f.needsGreyValue ) {
                    description += "*";
                 }
                 out.emplace_back( f.name, description );
              }
              return out;
           },
           "Returns a list of tuples. Each tuple has two strings: the name of a feature\n"
           "and its description. If the description ends with a '*' character, a gray-value\n"
           "image is required for the feature." );

   // Other functions
   m.def( "ObjectToMeasurement", py::overload_cast< dip::Image const&, dip::Measurement::IteratorFeature const& >( &dip::ObjectToMeasurement ), "label"_a, "featureValues"_a );
   m.def( "MeasurementWriteCSV", &dip::MeasurementWriteCSV, "measurement"_a, "filename"_a, "options"_a = dip::StringSet{} );
   m.def( "Minimum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Minimum ), "featureValues"_a  );
   m.def( "Maximum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Maximum ), "featureValues"_a  );
   m.def( "Percentile", py::overload_cast< dip::Measurement::IteratorFeature const&, dip::dfloat >( &dip::Percentile ), "featureValues"_a, "percentile"_a  );
   m.def( "Median", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Median ), "featureValues"_a  );
   m.def( "Mean", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Mean ), "featureValues"_a  );
   m.def( "MaximumAndMinimum", []( dip::Measurement::IteratorFeature const& featureValues ) {
             dip::MinMaxAccumulator acc = dip::MaximumAndMinimum( featureValues );
             return py::make_tuple( acc.Minimum(), acc.Maximum() ).release();
          }, "featureValues"_a,
          "Instead of returning a `dip::MinMaxAccumulator` object, returns a tuple with\n"
          "the minimum and maximum values.");
   m.def( "SampleStatistics", &dip::SampleStatistics, "featureValues"_a );
   m.def( "ObjectMinimum", &dip::ObjectMinimum, "featureValues"_a );
   m.def( "ObjectMaximum", &dip::ObjectMaximum, "featureValues"_a );

   // dip::Polygon
   auto poly = py::class_< dip::Polygon >( m, "Polygon", py::buffer_protocol(),
         "A polygon representing a 2D object.\n"
         "Implicitly converts to or from a NumPy array, and can directly be indexed and iterated." );
   poly.def( py::init([]( py::double_array_t& buf ) { return BufferToPolygon( buf ); } ));
   py::implicitly_convertible< py::buffer, dip::Polygon >();
   poly.def_buffer( []( dip::Polygon& self ) -> py::buffer_info { return PolygonToBuffer( self ); } );
   poly.def( "__repr__", []( dip::Polygon const& self ) {
                std::ostringstream os;
                os << "<Polygon with " << self.vertices.size() << " vertices>";
                return os.str();
             } );
   poly.def( "__getitem__", []( dip::Polygon const& self, dip::uint index ) {
                return self.vertices[ index ];
             }, "index"_a );
   poly.def( "__len__", []( dip::Polygon const& self ) {
                return self.vertices.size();
             } );
   poly.def( "__iter__", []( dip::Polygon const& self ) {
                return py::make_iterator( self.vertices.begin(), self.vertices.end() );
             }, py::keep_alive< 0, 1 >() );
   poly.def( "BoundingBox", []( dip::Polygon const& self ){
                auto bb = self.BoundingBox();
                auto topLeft = py::make_tuple( bb.topLeft.x, bb.topLeft.y );
                auto bottomRight = py::make_tuple( bb.bottomRight.x, bb.bottomRight.y );
                return py::make_tuple( topLeft, bottomRight ).release();
             },
             "Instead of returning a `dip::BoundingBoxFloat` object, returns a tuple with\n"
             "two tuples. The first tuple is the horizontal range, the second one is the\n"
             "vertical range. Each of these two tuples has two values representing the\n"
             "the lowest and highest value in the range." );
   poly.def( "IsClockWise", &dip::Polygon::IsClockWise );
   poly.def( "Area", &dip::Polygon::Area );
   poly.def( "Centroid", &dip::Polygon::Centroid );
   poly.def( "Length", &dip::Polygon::Length ); // is the perimeter
   poly.def( "Perimeter", &dip::Polygon::Perimeter );
   poly.def( "EllipseParameters", []( dip::Polygon const& self ) { return self.CovarianceMatrixSolid().Ellipse( true ); },
             "Corresponds to `dip::Polygon::CovarianceMatrixSolid().Ellipse( true )`." );
   poly.def( "RadiusStatistics", py::overload_cast<>( &dip::Polygon::RadiusStatistics, py::const_ ));
   poly.def( "EllipseVariance", py::overload_cast<>( &dip::Polygon::EllipseVariance, py::const_ ));
   poly.def( "FractalDimension", &dip::Polygon::FractalDimension, "length"_a = 0.0 );
   poly.def( "BendingEnergy", &dip::Polygon::BendingEnergy );
   poly.def( "Simplify", &dip::Polygon::Simplify, "tolerance"_a = 0.5 );
   poly.def( "Smooth", &dip::Polygon::Smooth, "sigma"_a = 1.0 );
   poly.def( "Reverse", &dip::Polygon::Reverse );
   poly.def( "Rotate", &dip::Polygon::Rotate, "angle"_a );
   poly.def( "Scale", py::overload_cast< dip::dfloat >( &dip::Polygon::Scale ), "scale"_a );
   poly.def( "Scale", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::Polygon::Scale ), "scaleX"_a, "scaleY"_a );
   poly.def( "Translate", &dip::Polygon::Translate, "shift"_a );
   poly.def( "ConvexHull", []( dip::Polygon const& self ) {
                return self.ConvexHull().Polygon();
             },
             "Returns a `dip.Polygon` object, not a `dip::ConvexHull` object as the C++\n"
             "function does." );
   poly.def( "Feret", []( dip::Polygon const& self ) { return self.ConvexHull().Feret(); },
             "Corresponds to `dip::Polygon::ConvexHull().Feret()`." );

   // dip::ChainCode
   auto chain = py::class_< dip::ChainCode >( m, "ChainCode", "" );
   chain.def( "__repr__", []( dip::ChainCode const& self ) {
                 std::ostringstream os;
                 os << "<ChainCode for object #" << self.objectID << '>';
                 return os.str();
              } );
   chain.def( "__getitem__", []( dip::ChainCode const& self, dip::uint index ) {
                return static_cast< unsigned >( self.codes[ index ] );
             }, "index"_a );
   chain.def( "__len__", []( dip::ChainCode const& self ) {
                return self.codes.size();
             } );
   chain.def( "__iter__", []( dip::ChainCode const& self ) {
                return py::make_iterator< py::return_value_policy::copy, decltype( self.codes.begin() ), decltype( self.codes.end() ), unsigned >( self.codes.begin(), self.codes.end() );
             }, py::keep_alive< 0, 1 >() );
   chain.def_property_readonly( "codes", []( dip::ChainCode const& self ) {
                 // We don't make this into a buffer protocol thing, because each code has two values: the code and IsBorder().
                 // This latter is encoded in the 4th bit of the number. So exposing that directly to the user would be confusing.
                 // We don't do so in C++ either. One alternative is to encapsulate the `Code` object, the other is to copy the
                 // chain code value into a Python list. We do the copy, as it is less work. Encapsulating the `Code` object would
                 // provide more functionality, but I doubt it will be used by anyone.
                 py::list list( self.codes.size() );
                 py::ssize_t index = 0;
                 for( auto value: self.codes ) {
                    PyList_SET_ITEM( list.ptr(), index++, py::cast( static_cast< unsigned >( value )).release().ptr() ); // Casting to unsigned gets the numeric value of the chain code
                 }
                 return list.release();
              },
              "cc.codes is the same as list(cc), and copies the chain code values to a list.\n"
              "To access individual code values, it's better to just index cc directly: cc[4],\n"
              "or use an iterator: iter(cc)." );
   chain.def_property_readonly( "start", []( dip::ChainCode const& self ) { return py::make_tuple( self.start.x, self.start.y ).release(); } );
   chain.def_readonly( "objectID", &dip::ChainCode::objectID );
   chain.def_readonly( "is8connected", &dip::ChainCode::is8connected );
   chain.def( "ConvertTo8Connected", &dip::ChainCode::ConvertTo8Connected );
   chain.def( "Empty", &dip::ChainCode::Empty );
   chain.def( "Length", &dip::ChainCode::Length );
   chain.def( "Feret", &dip::ChainCode::Feret, "angleStep"_a = 5.0 / 180.0 * dip::pi );
   chain.def( "BendingEnergy", &dip::ChainCode::BendingEnergy );
   chain.def( "BoundingBox", []( dip::ChainCode const& self ){
                 auto bb = self.BoundingBox();
                 auto topLeft = py::make_tuple( bb.topLeft.x, bb.topLeft.y );
                 auto bottomRight = py::make_tuple( bb.bottomRight.x, bb.bottomRight.y );
                 return py::make_tuple( topLeft, bottomRight ).release();
              },
              "Instead of returning a `dip::BoundingBoxInteger` object, returns a tuple with\n"
              "two tuples. The first tuple is the horizontal range, the second one is the\n"
              "vertical range. Each of these two tuples has two values representing the\n"
              "the lowest and highest value in the range." );
   chain.def( "LongestRun", &dip::ChainCode::LongestRun );
   chain.def( "Polygon", &dip::ChainCode::Polygon );
   chain.def( "Image", py::overload_cast<>( &dip::ChainCode::Image, py::const_ ));
   chain.def( "Offset", &dip::ChainCode::Offset );

   // Chain code functions
   m.def( "GetImageChainCodes", &dip::GetImageChainCodes, "labels"_a, "objectIDs"_a = dip::UnsignedArray{}, "connectivity"_a = 2 );
   m.def( "GetSingleChainCode", &dip::GetSingleChainCode, "labels"_a, "startCoord"_a, "connectivity"_a = 2 );

   // dip::CovarianceMatrix::EllipseParameters
   auto ellipseParams = py::class_< dip::CovarianceMatrix::EllipseParameters >( m, "EllipseParameters", "Parameters of the best fit ellipse." );
   ellipseParams.def( "__repr__", []( dip::CovarianceMatrix::EllipseParameters const& s ) {
                         std::ostringstream os;
                         os << "<EllipseParameters: ";
                         os << "majorAxis=" << s.majorAxis;
                         os << ", minorAxis=" << s.minorAxis;
                         os << ", orientation=" << s.orientation;
                         os << ", eccentricity=" << s.eccentricity;
                         os << '>';
                         return os.str();
                      } );
   ellipseParams.def_readonly( "majorAxis", &dip::CovarianceMatrix::EllipseParameters::majorAxis );
   ellipseParams.def_readonly( "minorAxis", &dip::CovarianceMatrix::EllipseParameters::minorAxis );
   ellipseParams.def_readonly( "orientation", &dip::CovarianceMatrix::EllipseParameters::orientation );
   ellipseParams.def_readonly( "eccentricity", &dip::CovarianceMatrix::EllipseParameters::eccentricity );

   // dip::FeretValues
   auto feretVals = py::class_< dip::FeretValues >( m, "FeretValues", "Values of the various Feret diameters." );
   feretVals.def( "__repr__", []( dip::FeretValues const& s ) {
                         std::ostringstream os;
                         os << "<FeretValues: ";
                         os << "maxDiameter=" << s.maxDiameter;
                         os << ", minDiameter=" << s.minDiameter;
                         os << ", maxPerpendicular=" << s.maxPerpendicular;
                         os << ", maxAngle=" << s.maxAngle;
                         os << ", minAngle=" << s.minAngle;
                         os << '>';
                         return os.str();
                      } );
   feretVals.def_readonly( "maxDiameter", &dip::FeretValues::maxDiameter );
   feretVals.def_readonly( "minDiameter", &dip::FeretValues::minDiameter );
   feretVals.def_readonly( "maxPerpendicular", &dip::FeretValues::maxPerpendicular );
   feretVals.def_readonly( "maxAngle", &dip::FeretValues::maxAngle );
   feretVals.def_readonly( "minAngle", &dip::FeretValues::minAngle );

   // dip::RadiusValues
   auto radiusVals = py::class_< dip::RadiusValues >( m, "RadiusValues", "Statistics on the radii of an object." );
   radiusVals.def( "__repr__", []( dip::RadiusValues const& s ) {
                         std::ostringstream os;
                         os << "<RadiusValues: ";
                         os << "mean=" << s.Mean();
                         os << ", standardDev=" << s.StandardDeviation();
                         os << ", maximum=" << s.Maximum();
                         os << ", minimum=" << s.Minimum();
                         os << ", circularity=" << s.Circularity();
                         os << '>';
                         return os.str();
                      } );
   radiusVals.def_property_readonly( "mean", &dip::RadiusValues::Mean );
   radiusVals.def_property_readonly( "standardDev", &dip::RadiusValues::StandardDeviation );
   radiusVals.def_property_readonly( "maximum", &dip::RadiusValues::Maximum );
   radiusVals.def_property_readonly( "minimum", &dip::RadiusValues::Minimum );
   radiusVals.def_property_readonly( "circularity", &dip::RadiusValues::Circularity );

}
