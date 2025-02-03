/*
 * (c)2017-2021, Flagship Biosciences, Inc., written by Cris Luengo.
 * (c)2022-2025, Cris Luengo.
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
#include <tuple>
#include <type_traits>
#include <vector>

#include "pydip.h"
#include "accumulators.h" // IWYU pragma: keep
#include "diplib/measurement.h"
#include "diplib/chain_code.h"
#include "diplib/label_map.h"


namespace {

template< typename T >
py::object VertexTuple( dip::Vertex< T > const& v ) {
   char const* vertexType = std::is_same< T, dip::dfloat >::value ? "VertexFloat" : "VertexInteger";
   return CreateNamedTuple( vertexType, "x y", v.x, v.y );
}

} // namespace

namespace pybind11 {
namespace detail {

#define PyNumber_Check( v ) (PyFloat_Check( v ) || PyLong_Check( v ))

// Cast Python 2-tuple to dip::VertexFloat
template<>
class type_caster< dip::VertexFloat > {
   public:
      using type = dip::VertexFloat;

      bool load( handle src, bool /*convert*/ ) {
         if( !isinstance< sequence >( src )) {
            return false;
         }
         auto const seq = reinterpret_borrow< sequence >( src );
         if( seq.size() != 2 ) {
            return false;
         }
         if( !PyNumber_Check( seq[ 0 ].ptr() ) || !PyNumber_Check( seq[ 1 ].ptr() )) {
            return false;
         }
         value = { seq[ 0 ].cast< dip::dfloat >(), seq[ 1 ].cast< dip::dfloat >() };
         return true;
      }

      static handle cast( dip::VertexFloat const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         return VertexTuple( src ).release();
      }

      PYBIND11_TYPE_CASTER( type, _( "VertexFloat" ));
};

template<>
class type_caster< dip::VertexInteger > {
   public:
      using type = dip::VertexInteger;

      bool load( handle src, bool /*convert*/ ) {
         if( !isinstance< sequence >( src )) {
            return false;
         }
         auto const seq = reinterpret_borrow< sequence >( src );
         if( seq.size() != 2 ) {
            return false;
         }
         if( !PyLong_Check( seq[ 0 ].ptr() ) || !PyLong_Check( seq[ 1 ].ptr() )) {
            return false;
         }
         value = { seq[ 0 ].cast< dip::sint >(), seq[ 1 ].cast< dip::sint >() };
         return true;
      }

      static handle cast( dip::VertexInteger const& src, return_value_policy /*policy*/, handle /*parent*/ ) {
         return VertexTuple( src ).release();
      }

      PYBIND11_TYPE_CASTER( type, _( "VertexInteger" ));
};

DIP_OUTPUT_TYPE_CASTER( Measurement::FeatureInformation, "FeatureInformation", "name startColumn numberValues", src.name, src.startColumn, src.numberValues )
DIP_OUTPUT_TYPE_CASTER( Feature::ValueInformation, "ValueInformation", "name units", src.name, src.units )

DIP_OUTPUT_TYPE_CASTER( BoundingBoxFloat, "BoundingBoxFloat", "topLeft bottomRight", VertexTuple( src.topLeft ), VertexTuple( src.bottomRight ) )
DIP_OUTPUT_TYPE_CASTER( BoundingBoxInteger, "BoundingBoxInteger", "topLeft bottomRight", VertexTuple( src.topLeft ), VertexTuple( src.bottomRight ) )
DIP_OUTPUT_TYPE_CASTER( FeretValues, "FeretValues", "maxDiameter minDiameter maxPerpendicular maxAngle minAngle", src.maxDiameter, src.minDiameter, src.maxPerpendicular, src.maxAngle, src.minAngle )
DIP_OUTPUT_TYPE_CASTER( RadiusValues, "RadiusValues", "mean standardDev maximum minimum circularity", src.Mean(), src.StandardDeviation(), src.Maximum(), src.Minimum(), src.Circularity() )
DIP_OUTPUT_TYPE_CASTER( CircleParameters, "CircleParameters", "center diameter", VertexTuple( src.center ), src.diameter )
DIP_OUTPUT_TYPE_CASTER( EllipseParameters, "EllipseParameters", "center majorAxis minorAxis orientation eccentricity", VertexTuple( src.center ), src.majorAxis, src.minorAxis, src.orientation, src.eccentricity )
DIP_OUTPUT_TYPE_CASTER( CovarianceMatrix::Eigenvalues, "Eigenvalues", "largest smallest, eccentricity", src.largest, src.smallest, src.Eccentricity() )

} // namespace detail
} // namespace pybind11

namespace {

dip::MeasurementTool& measurementTool() {
   static dip::MeasurementTool tool;
   return tool;
}

template< typename MeasurementValues >
py::object MeasurementValuesToList( MeasurementValues values ) {
   py::list list( values.size() );
   py::ssize_t index = 0;
   for( auto& value : values ) {
      PyList_SET_ITEM( list.ptr(), index++, py::cast( value ).release().ptr() );
   }
   return list;
}

py::buffer_info MeasurementFeatureToBuffer( dip::Measurement::IteratorFeature& feature ) {
   dip::sint itemsize = static_cast< dip::sint >( sizeof( dip::dfloat ));
   dip::UnsignedArray sizes = { feature.NumberOfObjects(), feature.NumberOfValues() };
   dip::IntegerArray strides = { feature.Stride() * itemsize, itemsize };
   py::buffer_info info{
      feature.NumberOfObjects() > 0 ? feature.Data() : nullptr,
      itemsize, py::format_descriptor< dip::dfloat >::format(),
      static_cast< py::ssize_t >( sizes.size() ), sizes, strides
   };
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
   py::buffer_info info{
      object.Data(),
      itemsize,
      py::format_descriptor< dip::dfloat >::format(),
      static_cast< py::ssize_t >( sizes.size() ),
      sizes,
      strides
   };
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
   py::buffer_info info{
      msr.NumberOfObjects() > 0 ? msr.Data() : nullptr,
      itemsize,
      py::format_descriptor< dip::dfloat >::format(),
      static_cast< py::ssize_t >( sizes.size() ),
      sizes,
      strides
   };
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
   py::buffer_info info{
      polygon.vertices.data(),
      itemsize,
      py::format_descriptor< dip::dfloat >::format(),
      static_cast< py::ssize_t >( sizes.size() ),
      sizes,
      strides
   };
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

   // dip::MeasurementTool
   auto tool = m.def_submodule( "MeasurementTool",
                                "A tool to quantify objects in an image.\n\n"
                                "This is a submodule that uses a static `dip::Measurement` object. Functions\n"
                                "defined in this module correspond to the object member functions in C++." );
   tool.def( "Configure", []( dip::String const& feature, dip::String const& parameter, dip::dfloat value ) {
      measurementTool().Configure( feature, parameter, value );
   }, "feature"_a, "parameter"_a, "value"_a, doc_strings::dip·MeasurementTool·Configure·String·CL·String·CL·dfloat··C );
   tool.def( "Measure", []( dip::Image const& label, dip::Image const& grey, dip::StringArray const& features, dip::UnsignedArray const& objectIDs, dip::uint connectivity ) {
      return measurementTool().Measure( label, grey, features, objectIDs, connectivity );
   }, "label"_a, "grey"_a = dip::Image{}, "features"_a = dip::StringArray{ "Size" }, "objectIDs"_a = dip::StringArray{}, "connectivity"_a = 0, doc_strings::dip·MeasurementTool·Measure·Image·CL·Image·CL·StringArray··UnsignedArray·CL·dip·uint··C );
   tool.def( "Features", []() {
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

   // dip::Measurement
   auto meas = py::class_< dip::Measurement >( m, "Measurement", py::buffer_protocol(), doc_strings::dip·Measurement );
   meas.def_buffer( []( dip::Measurement& self ) -> py::buffer_info { return MeasurementToBuffer( self ); } );
   meas.def( "__repr__", []( dip::Measurement const& self ) {
      std::ostringstream os;
      os << "<Measurement with " << self.NumberOfFeatures() << " features for " << self.NumberOfObjects() << " objects>";
      return os.str();
   } );
   meas.def( "__str__", []( dip::Measurement const& self ) {
      std::ostringstream os;
      os << self;
      return os.str();
   } );
   meas.def( "__len__", &dip::Measurement::NumberOfObjects, "Same as NumberOfObjects()." ),
   meas.def( "__getitem__", py::overload_cast< dip::uint >( &dip::Measurement::operator[], py::const_ ),
             "objectID"_a, py::return_value_policy::reference_internal, doc_strings::dip·Measurement·operatorsqbra·dip·uint··C );
   meas.def( "__getitem__", py::overload_cast< dip::String const& >( &dip::Measurement::operator[], py::const_ ),
             "name"_a, py::return_value_policy::reference_internal, doc_strings::dip·Measurement·operatorsqbra·String·CL·C );
   meas.def( "__getitem__", py::overload_cast< dip::LabelMap const& >( &dip::Measurement::operator[], py::const_ ),
             "map"_a, doc_strings::dip·Measurement·operatorsqbra·LabelMap·CL·C );
   meas.def( "FeatureValuesView", &dip::Measurement::FeatureValuesView, "startValue"_a, "numberValues"_a = 1,
             py::return_value_policy::reference_internal, doc_strings::dip·Measurement·FeatureValuesView·dip·uint··dip·uint··C );
   meas.def( "FeatureExists", &dip::Measurement::FeatureExists, doc_strings::dip·Measurement·FeatureExists·String·CL·C );
   meas.def( "Features", &dip::Measurement::Features, doc_strings::dip·Measurement·Features·C );
   meas.def( "NumberOfFeatures", &dip::Measurement::NumberOfFeatures, doc_strings::dip·Measurement·NumberOfFeatures·C );
   meas.def( "Values", py::overload_cast< dip::String const& >( &dip::Measurement::Values, py::const_ ), "name"_a, doc_strings::dip·Measurement·Values·String·CL·C );
   meas.def( "Values", py::overload_cast<>( &dip::Measurement::Values, py::const_ ), doc_strings::dip·Measurement·Values·C );
   meas.def( "NumberOfValues", py::overload_cast< dip::String const& >( &dip::Measurement::NumberOfValues, py::const_ ), "name"_a, doc_strings::dip·Measurement·NumberOfValues·String·CL·C );
   meas.def( "NumberOfValues", py::overload_cast<>( &dip::Measurement::NumberOfValues, py::const_ ), doc_strings::dip·Measurement·NumberOfValues·C );
   meas.def( "ObjectExists", &dip::Measurement::ObjectExists, doc_strings::dip·Measurement·ObjectExists·dip·uint··C );
   meas.def( "Objects", &dip::Measurement::Objects, doc_strings::dip·Measurement·Objects·C );
   meas.def( "NumberOfObjects", &dip::Measurement::NumberOfObjects, doc_strings::dip·Measurement·NumberOfObjects·C );
   meas.def( py::self + py::self );

   // dip::Measurement::IteratorFeature
   auto feat = py::class_< dip::Measurement::IteratorFeature >( meas, "IteratorFeature", py::buffer_protocol(), doc_strings::dip·Measurement·IteratorFeature );
   feat.def_buffer( []( dip::Measurement::IteratorFeature& self ) -> py::buffer_info { return MeasurementFeatureToBuffer( self ); } );
   feat.def( "__repr__", []( dip::Measurement::IteratorFeature const& self ) {
      std::ostringstream os;
      os << "<IteratorFeature for feature " << self.FeatureName() << " and " << self.NumberOfObjects() << " objects>";
      return os.str();
   } );
   feat.def( "__getitem__", []( dip::Measurement::IteratorFeature const& self, dip::uint objectID ) {
      return MeasurementValuesToList( self[ objectID ] );
   }, "objectID"_a, doc_strings::dip·Measurement·IteratorFeature·operatorsqbra·dip·uint··C );
   feat.def( "__len__", &dip::Measurement::IteratorFeature::NumberOfObjects, "Same as NumberOfObjects()." ),
   feat.def( "__iter__", []( dip::Measurement::IteratorFeature const& self ) { return self.FirstObject(); } );
   feat.def( "keys", &dip::Measurement::IteratorFeature::Objects, "Returns a list with all the keys (object IDs). Same as Objects()." );
   feat.def( "values", []( dip::Measurement::IteratorFeature const& self  ) {
      py::list list( self.NumberOfObjects() );
      py::ssize_t index = 0;
      auto it = self.FirstObject();
      do {
         PyList_SET_ITEM( list.ptr(), index++, MeasurementValuesToList( it ).release().ptr() );
      } while( ++it );
      return list;
   }, "Returns a list with all the values." );
   feat.def( "items", []( dip::Measurement::IteratorFeature const& self  ) {
      py::list list( self.NumberOfObjects() );
      py::ssize_t index = 0;
      auto it = self.FirstObject();
      do {
         PyList_SET_ITEM( list.ptr(), index++, py::make_tuple( py::cast( it.ObjectID() ).release(), MeasurementValuesToList( it ).release() ).release().ptr() );
      } while( ++it );
      return list;
   }, "Returns a list of tuples with (key, value) pairs." );
   feat.def( "Subset", &dip::Measurement::IteratorFeature::Subset, "first"_a, "number"_a = 1, doc_strings::dip·Measurement·IteratorFeature·Subset·dip·uint··dip·uint· );
   feat.def( "FeatureName", &dip::Measurement::IteratorFeature::FeatureName, doc_strings::dip·Measurement·IteratorFeature·FeatureName·C );
   feat.def( "Values", &dip::Measurement::IteratorFeature::Values, doc_strings::dip·Measurement·IteratorFeature·Values·C );
   feat.def( "NumberOfValues", &dip::Measurement::IteratorFeature::NumberOfValues, doc_strings::dip·Measurement·IteratorFeature·NumberOfValues·C );
   feat.def( "ObjectExists", &dip::Measurement::IteratorFeature::ObjectExists, doc_strings::dip·Measurement·IteratorFeature·ObjectExists·dip·uint··C );
   feat.def( "Objects", &dip::Measurement::IteratorFeature::Objects, doc_strings::dip·Measurement·IteratorFeature·Objects·C );
   feat.def( "NumberOfObjects", &dip::Measurement::IteratorFeature::NumberOfObjects, doc_strings::dip·Measurement·IteratorFeature·NumberOfObjects·C );
   feat.def( "__eq__", []( dip::Measurement::IteratorFeature const& lhs, dip::Measurement::ValueType rhs ) { return lhs == rhs; }, py::is_operator(), doc_strings::dip·operatoreqeq·Measurement·IteratorFeature·CL·Measurement·ValueType· );
   feat.def( "__ne__", []( dip::Measurement::IteratorFeature const& lhs, dip::Measurement::ValueType rhs ) { return lhs != rhs; }, py::is_operator(), doc_strings::dip·operatornoteq·Measurement·IteratorFeature·CL·Measurement·ValueType· );
   feat.def( "__gt__", []( dip::Measurement::IteratorFeature const& lhs, dip::Measurement::ValueType rhs ) { return lhs > rhs; }, py::is_operator(), doc_strings::dip·operatorgt·Measurement·IteratorFeature·CL·Measurement·ValueType· );
   feat.def( "__ge__", []( dip::Measurement::IteratorFeature const& lhs, dip::Measurement::ValueType rhs ) { return lhs >= rhs; }, py::is_operator(), doc_strings::dip·operatorgteq·Measurement·IteratorFeature·CL·Measurement·ValueType· );
   feat.def( "__lt__", []( dip::Measurement::IteratorFeature const& lhs, dip::Measurement::ValueType rhs ) { return lhs < rhs; }, py::is_operator(), doc_strings::dip·operatorlt·Measurement·IteratorFeature·CL·Measurement·ValueType· );
   feat.def( "__le__", []( dip::Measurement::IteratorFeature const& lhs, dip::Measurement::ValueType rhs ) { return lhs <= rhs; }, py::is_operator(), doc_strings::dip·operatorlteq·Measurement·IteratorFeature·CL·Measurement·ValueType· );

   // dip::Measurement::IteratorFeature::Iterator
   auto feat_it = py::class_< dip::Measurement::IteratorFeature::Iterator >( feat, "Iterator", doc_strings::dip·Measurement·IteratorFeature·Iterator );
   feat_it.def( "__iter__", []( dip::Measurement::IteratorFeature::Iterator& self ) { return self; } );
   feat_it.def( "__next__", []( dip::Measurement::IteratorFeature::Iterator& self ) {
      if( self.IsAtEnd() ) { throw py::stop_iteration(); }
      return MeasurementValuesToList( self++ );
   } );

   // dip::Measurement::IteratorObject
   auto obj = py::class_< dip::Measurement::IteratorObject >( meas, "IteratorObject", py::buffer_protocol(), doc_strings::dip·Measurement·IteratorObject );
   obj.def_buffer( []( dip::Measurement::IteratorObject& self ) -> py::buffer_info {
      return MeasurementObjectToBuffer( self );
   } );
   obj.def( "__repr__", []( dip::Measurement::IteratorObject const& self ) {
      std::ostringstream os;
      os << "<IteratorObject with " << self.NumberOfFeatures() << " features for object " << self.ObjectID() << '>';
      return os.str();
   } );
   obj.def( "__getitem__", []( dip::Measurement::IteratorObject const& self, dip::String const& name ) {
      return MeasurementValuesToList( self[ name ] );
   }, "name"_a, doc_strings::dip·Measurement·IteratorObject·operatorsqbra·String·CL·C );
   obj.def( "__len__", py::overload_cast<>( &dip::Measurement::IteratorObject::NumberOfValues, py::const_ ), "Same as NumberOfValues()." ),
   obj.def( "__iter__", []( dip::Measurement::IteratorObject const& self ) { return self.FirstFeature(); } );
   obj.def( "keys", []( dip::Measurement::IteratorObject const& self ) {
      py::list list( self.NumberOfFeatures() );
      py::ssize_t index = 0;
      for( auto const& f : self.Features() ) {
         PyList_SET_ITEM( list.ptr(), index++, py::cast( f.name ).release().ptr() );
      };
      return list;
   }, "Returns a list with all the keys (feature names)." );
   obj.def( "values", []( dip::Measurement::IteratorObject const& self ) {
      py::list list( self.NumberOfFeatures() );
      py::ssize_t index = 0;
      auto it = self.FirstFeature();
      do {
         PyList_SET_ITEM( list.ptr(), index++, MeasurementValuesToList( it ).release().ptr() );
      } while( ++it );
      return list;
   }, "Returns a list with all the values." );
   obj.def( "items", []( dip::Measurement::IteratorObject const& self ) {
      py::list list( self.NumberOfFeatures() );
      py::ssize_t index = 0;
      auto it = self.FirstFeature();
      do {
         PyList_SET_ITEM( list.ptr(), index++, py::make_tuple( py::cast( it.FeatureName() ).release(), MeasurementValuesToList( it ).release() ).release().ptr() );
      } while( ++it );
      return list;
   }, "Returns a list of tuples with (key, value) pairs." );
   obj.def( "ObjectID", &dip::Measurement::IteratorObject::ObjectID, doc_strings::dip·Measurement·IteratorObject·ObjectID·C );
   obj.def( "FeatureExists", &dip::Measurement::IteratorObject::FeatureExists, doc_strings::dip·Measurement·IteratorObject·FeatureExists·String·CL·C );
   obj.def( "Features", &dip::Measurement::IteratorObject::Features, doc_strings::dip·Measurement·IteratorObject·Features·C );
   obj.def( "NumberOfFeatures", &dip::Measurement::IteratorObject::NumberOfFeatures, doc_strings::dip·Measurement·IteratorObject·NumberOfFeatures·C );
   obj.def( "Values", py::overload_cast< dip::String const& >( &dip::Measurement::IteratorObject::Values, py::const_ ), "name"_a, doc_strings::dip·Measurement·IteratorObject·Values·String·CL·C );
   obj.def( "Values", py::overload_cast<>( &dip::Measurement::IteratorObject::Values, py::const_ ), doc_strings::dip·Measurement·IteratorObject·Values·C );
   obj.def( "NumberOfValues", py::overload_cast< dip::String const& >( &dip::Measurement::IteratorObject::NumberOfValues, py::const_ ), "name"_a, doc_strings::dip·Measurement·IteratorObject·NumberOfValues·String·CL·C );
   obj.def( "NumberOfValues", py::overload_cast<>( &dip::Measurement::IteratorObject::NumberOfValues, py::const_ ), doc_strings::dip·Measurement·IteratorObject·NumberOfValues·C );

   // dip::Measurement::IteratorObject::Iterator
   auto obj_it = py::class_< dip::Measurement::IteratorObject::Iterator >( obj, "Iterator", doc_strings::dip·Measurement·IteratorObject·Iterator );
   obj_it.def( "__iter__", []( dip::Measurement::IteratorObject::Iterator& self ) { return self; } );
   obj_it.def( "__next__", []( dip::Measurement::IteratorObject::Iterator& self ) {
      if( self.IsAtEnd() ) { throw py::stop_iteration(); }
      return MeasurementValuesToList( self++ );
   } );

   // Other functions
   m.def( "ObjectToMeasurement", py::overload_cast< dip::Image const&, dip::Measurement::IteratorFeature const& >( &dip::ObjectToMeasurement ), "label"_a, "featureValues"_a, doc_strings::dip·ObjectToMeasurement·Image·CL·Image·L·Measurement·IteratorFeature·CL );
   m.def( "MeasurementWriteCSV", &dip::MeasurementWriteCSV, "measurement"_a, "filename"_a, "options"_a = dip::StringSet{}, doc_strings::dip·MeasurementWriteCSV·Measurement·CL·String·CL·StringSet·CL );
   // (Note that most of the functions below are resolved correctly because we don't include diplib/statistics.h,
   // which defines functions with the same name but different arguments.)
   m.def( "Minimum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Minimum ), "featureValues"_a, doc_strings::dip·Minimum·Measurement·IteratorFeature·CL );
   m.def( "Maximum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Maximum ), "featureValues"_a, doc_strings::dip·Maximum·Measurement·IteratorFeature·CL );
   m.def( "Percentile", py::overload_cast< dip::Measurement::IteratorFeature const&, dip::dfloat >( &dip::Percentile ), "featureValues"_a, "percentile"_a, doc_strings::dip·Percentile·Measurement·IteratorFeature·CL·dfloat· );
   m.def( "Median", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Median ), "featureValues"_a, doc_strings::dip·Median·Measurement·IteratorFeature·CL );
   m.def( "Mean", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Mean ), "featureValues"_a, doc_strings::dip·Mean·Measurement·IteratorFeature·CL );
   m.def( "MaximumAndMinimum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::MaximumAndMinimum ),
          "featureValues"_a, doc_strings::dip·MaximumAndMinimum·Measurement·IteratorFeature·CL );
   m.def( "Quartiles", &dip::Quartiles, "featureValues"_a, doc_strings::dip·Quartiles·Measurement·IteratorFeature·CL );
   m.def( "SampleStatistics", &dip::SampleStatistics, "featureValues"_a, doc_strings::dip·SampleStatistics·Measurement·IteratorFeature·CL );
   m.def( "ObjectMinimum", &dip::ObjectMinimum, "featureValues"_a, doc_strings::dip·ObjectMinimum·Measurement·IteratorFeature·CL );
   m.def( "ObjectMaximum", &dip::ObjectMaximum, "featureValues"_a, doc_strings::dip·ObjectMaximum·Measurement·IteratorFeature·CL );

   // dip::CovarianceMatrix
   auto covmat = py::class_< dip::CovarianceMatrix >( m, "CovarianceMatrix", doc_strings::dip·CovarianceMatrix );
   covmat.def( py::init<>(), doc_strings::dip·CovarianceMatrix·CovarianceMatrix );
   covmat.def( py::init< dip::VertexFloat const& >(), "v"_a, doc_strings::dip·CovarianceMatrix·CovarianceMatrix·VertexFloat· );
   covmat.def( py::init< dip::dfloat, dip::dfloat, dip::dfloat >(), "xx"_a, "yy"_a, "xy"_a, doc_strings::dip·CovarianceMatrix·CovarianceMatrix·dfloat··dfloat··dfloat· );
   covmat.def( "__repr__", []( dip::CovarianceMatrix const& self ) {
         std::ostringstream os;
         os << "<CovarianceMatrix: xx = " << self.xx() << ", xy = " << self.xy() << ", yy = " << self.yy() << ">";
         return os.str();
      } );
   covmat.def_property_readonly( "xx", &dip::CovarianceMatrix::xx, doc_strings::dip·CovarianceMatrix·xx·C );
   covmat.def_property_readonly( "xy", &dip::CovarianceMatrix::xy, doc_strings::dip·CovarianceMatrix·xy·C );
   covmat.def_property_readonly( "yy", &dip::CovarianceMatrix::yy, doc_strings::dip·CovarianceMatrix·yy·C );
   covmat.def( "Det", &dip::CovarianceMatrix::Det, doc_strings::dip·CovarianceMatrix·Det·C );
   covmat.def( "Inv", &dip::CovarianceMatrix::Inv, doc_strings::dip·CovarianceMatrix·Inv·C );
   covmat.def( "__iadd__", &dip::CovarianceMatrix::operator+=, "other"_a, doc_strings::dip·CovarianceMatrix·operatorpluseq·CovarianceMatrix·CL );
   covmat.def( "__imul__", &dip::CovarianceMatrix::operator*=, "d"_a, doc_strings::dip·CovarianceMatrix·operatortimeseq·dfloat· );
   covmat.def( "__itruediv__", &dip::CovarianceMatrix::operator/=, "d"_a, doc_strings::dip·CovarianceMatrix·operatordiveq·dfloat· );
   covmat.def( "Project", &dip::CovarianceMatrix::Project, "v"_a, doc_strings::dip·CovarianceMatrix·Project·VertexFloat·CL·C );
   covmat.def( "Eig", &dip::CovarianceMatrix::Eig, doc_strings::dip·CovarianceMatrix·Eig·C );
   covmat.def( "Ellipse", &dip::CovarianceMatrix::Ellipse, "solid"_a = false, doc_strings::dip·CovarianceMatrix·Ellipse·bool··C );

   // dip::Polygon
   auto poly = py::class_< dip::Polygon >( m, "Polygon", py::buffer_protocol(),
                                           "A polygon with floating-point vertices representing a 2D object.\n"
                                           "Implicitly converts to or from a NumPy array, and can directly be indexed\nand iterated." );
   poly.def( py::init( []( py::double_array_t& buf ) { return BufferToPolygon( buf ); } ));
   py::implicitly_convertible< py::buffer, dip::Polygon >();
   poly.def_buffer( []( dip::Polygon& self ) -> py::buffer_info { return PolygonToBuffer( self ); } );
   poly.def( "__repr__", []( dip::Polygon const& self ) {
      std::ostringstream os;
      os << "<Polygon with " << self.vertices.size() << " vertices>";
      return os.str();
   } );
   poly.def( "__getitem__", []( dip::Polygon const& self, dip::uint index ) {
      return self.vertices[ index ];
   }, "index"_a, "Index to retrieve a single vertex." );
   poly.def( "__len__", []( dip::Polygon const& self ) {
      return self.vertices.size();
   } );
   poly.def( "__iter__", []( dip::Polygon const& self ) {
      return py::make_iterator( self.vertices.begin(), self.vertices.end() );
   }, py::keep_alive< 0, 1 >() );
   poly.def( "BoundingBox", &dip::Polygon::BoundingBox, doc_strings::dip·Polygon·BoundingBox·C );
   poly.def( "IsClockWise", &dip::Polygon::IsClockWise, doc_strings::dip·Polygon·IsClockWise·C );
   poly.def( "Area", &dip::Polygon::Area, doc_strings::dip·Polygon·Area·C );
   poly.def( "Centroid", &dip::Polygon::Centroid, doc_strings::dip·Polygon·Centroid·C );
   poly.def( "CovarianceMatrixVertices", py::overload_cast< dip::VertexFloat const& >( &dip::Polygon::CovarianceMatrixVertices, py::const_ ), "g"_a, doc_strings::dip·Polygon·CovarianceMatrixVertices·VertexFloat·CL·C );
   poly.def( "CovarianceMatrixVertices", py::overload_cast<>( &dip::Polygon::CovarianceMatrixVertices, py::const_ ), doc_strings::dip·Polygon·CovarianceMatrixVertices·C );
   poly.def( "CovarianceMatrixSolid", py::overload_cast< dip::VertexFloat const& >( &dip::Polygon::CovarianceMatrixSolid, py::const_ ), "g"_a, doc_strings::dip·Polygon·CovarianceMatrixSolid·VertexFloat·CL·C );
   poly.def( "CovarianceMatrixSolid", py::overload_cast<>( &dip::Polygon::CovarianceMatrixSolid, py::const_ ), doc_strings::dip·Polygon·CovarianceMatrixSolid·C );
   poly.def( "Length", &dip::Polygon::Length, doc_strings::dip·Polygon·Length·C );
   poly.def( "Perimeter", &dip::Polygon::Perimeter, doc_strings::dip·Polygon·Perimeter·C );
   poly.def( "EllipseParameters", []( dip::Polygon const& self ) { return self.CovarianceMatrixSolid().Ellipse( true ); },
             "Compute parameters of ellipse with same covariance matrix.\n"
             "Corresponds to `dip::Polygon::CovarianceMatrixSolid().Ellipse( true )`." );
   poly.def( "RadiusStatistics", py::overload_cast<>( &dip::Polygon::RadiusStatistics, py::const_ ), doc_strings::dip·Polygon·RadiusStatistics·C );
   poly.def( "RadiusStatistics", py::overload_cast< dip::VertexFloat const& >( &dip::Polygon::RadiusStatistics, py::const_ ), "g"_a, doc_strings::dip·Polygon·RadiusStatistics·VertexFloat·CL·C );
   poly.def( "EllipseVariance", py::overload_cast<>( &dip::Polygon::EllipseVariance, py::const_ ), doc_strings::dip·Polygon·EllipseVariance·C );
   poly.def( "EllipseVariance", py::overload_cast< dip::VertexFloat const&, dip::CovarianceMatrix const& >( &dip::Polygon::EllipseVariance, py::const_ ), doc_strings::dip·Polygon·EllipseVariance·VertexFloat·CL·dip·CovarianceMatrix·CL·C );
   poly.def( "FractalDimension", &dip::Polygon::FractalDimension, "length"_a = 0.0, doc_strings::dip·Polygon·FractalDimension·dfloat··C );
   poly.def( "BendingEnergy", &dip::Polygon::BendingEnergy, doc_strings::dip·Polygon·BendingEnergy·C );
   poly.def( "FitCircle", &dip::Polygon::FitCircle, doc_strings::dip·Polygon·FitCircle·C );
   poly.def( "FitEllipse", &dip::Polygon::FitEllipse, doc_strings::dip·Polygon·FitEllipse·C );
   poly.def( "Simplify", &dip::Polygon::Simplify, "tolerance"_a = 0.5, doc_strings::dip·Polygon·Simplify·dfloat· );
   poly.def( "Augment", &dip::Polygon::Augment, "distance"_a = 1.0, doc_strings::dip·Polygon·Augment·dfloat· );
   poly.def( "Smooth", &dip::Polygon::Smooth, "sigma"_a = 1.0, doc_strings::dip·Polygon·Smooth·dfloat· );
   poly.def( "Reverse", &dip::Polygon::Reverse, doc_strings::dip·Polygon·Reverse );
   poly.def( "Rotate", &dip::Polygon::Rotate, "angle"_a, doc_strings::dip·Polygon·Rotate·dfloat· );
   poly.def( "Scale", py::overload_cast< dip::dfloat >( &dip::Polygon::Scale ), "scale"_a, doc_strings::dip·Polygon·Scale·dfloat· );
   poly.def( "Scale", py::overload_cast< dip::dfloat, dip::dfloat >( &dip::Polygon::Scale ), "scaleX"_a, "scaleY"_a, doc_strings::dip·Polygon·Scale·dfloat··dfloat· );
   poly.def( "Translate", &dip::Polygon::Translate, "shift"_a, doc_strings::dip·Polygon·Translate·VertexFloat· );
   poly.def( "ConvexHull", []( dip::Polygon const& self ) {
                return self.ConvexHull().Polygon();
             },
             "Returns the convex hull of the polygon. The polygon must be simple.\n"
             "Returns a `dip.Polygon` object, not a `dip::ConvexHull` object as the C++\n"
             "function does." );
   poly.def( "Feret", []( dip::Polygon const& self ) { return self.ConvexHull().Feret(); },
             "Returns the Feret diameters of the convex hull.\n"
             "Corresponds to `dip::Polygon::ConvexHull().Feret()`." );

   // dip::ChainCode
   auto chain = py::class_< dip::ChainCode >( m, "ChainCode", doc_strings::dip·ChainCode );
   chain.def( "__repr__", []( dip::ChainCode const& self ) {
      std::ostringstream os;
      os << "<ChainCode for object #" << self.objectID << '>';
      return os.str();
   } );
   chain.def( "__getitem__", []( dip::ChainCode const& self, dip::uint index ) {
      return static_cast< unsigned >( self.codes[ index ] );
   }, "index"_a, "Index into the array of chain codes." );
   chain.def( "__len__", []( dip::ChainCode const& self ) {
      return self.codes.size();
   } );
   chain.def( "__iter__", []( dip::ChainCode const& self ) {
      return py::make_iterator< py::return_value_policy::copy, decltype( self.codes.begin() ), decltype( self.codes.end() ), unsigned >( self.codes.begin(), self.codes.end() );
   }, py::keep_alive< 0, 1 >() );
   chain.def_property_readonly(
         "codes", []( dip::ChainCode const& self ) {
            // We don't make this into a buffer protocol thing, because each code has two values: the code and IsBorder().
            // This latter is encoded in the 4th bit of the number. So exposing that directly to the user would be confusing.
            // We don't do so in C++ either. One alternative is to encapsulate the `Code` object, the other is to copy the
            // chain code value into a Python list. We do the copy, as it is less work. Encapsulating the `Code` object would
            // provide more functionality, but I doubt it will be used by anyone.
            py::list list( self.codes.size() );
            py::ssize_t index = 0;
            for( auto value : self.codes ) {
               PyList_SET_ITEM( list.ptr(), index++, py::cast( static_cast< unsigned >( value )).release().ptr() ); // Casting to unsigned gets the numeric value of the chain code
            }
            return list;
         },
         "cc.codes is the same as list(cc), and copies the chain code values to a list.\n"
         "To access individual code values, it's better to just index cc directly: cc[4],\n"
         "or use an iterator: iter(cc)." );
   chain.def_readonly( "start", &dip::ChainCode::start, doc_strings::dip·ChainCode·start );
   chain.def_readonly( "objectID", &dip::ChainCode::objectID, doc_strings::dip·ChainCode·objectID );
   chain.def_readonly( "is8connected", &dip::ChainCode::is8connected, doc_strings::dip·ChainCode·is8connected );
   chain.def( "ConvertTo8Connected", &dip::ChainCode::ConvertTo8Connected, doc_strings::dip·ChainCode·ConvertTo8Connected·C );
   chain.def( "Empty", &dip::ChainCode::Empty, doc_strings::dip·ChainCode·Empty·C );
   chain.def( "Length", &dip::ChainCode::Length, "boundaryPixels"_a = dip::S::EXCLUDE, doc_strings::dip·ChainCode·Length·String·CL·C );
   chain.def( "Feret", &dip::ChainCode::Feret, "angleStep"_a = 5.0 / 180.0 * dip::pi, doc_strings::dip·ChainCode·Feret·dfloat··C );
   chain.def( "BendingEnergy", &dip::ChainCode::BendingEnergy, doc_strings::dip·ChainCode·BendingEnergy·C );
   chain.def( "BoundingBox", &dip::ChainCode::BoundingBox, doc_strings::dip·ChainCode·BoundingBox·C );
   chain.def( "LongestRun", &dip::ChainCode::LongestRun, doc_strings::dip·ChainCode·LongestRun·C );
   chain.def( "Polygon", &dip::ChainCode::Polygon, "borderCodes"_a = dip::S::KEEP, doc_strings::dip·ChainCode·Polygon·String·CL·C );
   chain.def( "Image", py::overload_cast<>( &dip::ChainCode::Image, py::const_ ), doc_strings::dip·ChainCode·Image·dip·Image·L·C );
   chain.def( "Image", py::overload_cast< dip::Image& >( &dip::ChainCode::Image, py::const_ ), "out"_a, doc_strings::dip·ChainCode·Image·dip·Image·L·C );
   chain.def( "Coordinates", &dip::ChainCode::Coordinates, doc_strings::dip·ChainCode·Coordinates·C );
   chain.def( "Offset", &dip::ChainCode::Offset, doc_strings::dip·ChainCode·Offset·C );

   // Chain code functions
   m.def( "GetImageChainCodes", py::overload_cast< dip::Image const&, std::vector< dip::LabelType > const&, dip::uint >( &dip::GetImageChainCodes ),
          "labels"_a, "objectIDs"_a = dip::UnsignedArray{}, "connectivity"_a = 2, doc_strings::dip·GetImageChainCodes·Image·CL·std·vectorgtLabelTypelt·CL·dip·uint· );
   m.def( "GetSingleChainCode", &dip::GetSingleChainCode, "labels"_a, "startCoord"_a, "connectivity"_a = 2, doc_strings::dip·GetSingleChainCode·Image·CL·UnsignedArray·CL·dip·uint· );

}
