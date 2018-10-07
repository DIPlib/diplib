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

#include <diplib/file_io.h>
#include "pydip.h"
#include "diplib/measurement.h"
#include "diplib/chain_code.h"

dip::MeasurementTool measurementTool;

namespace {

template< typename MeasurementValues >
py::handle MeasurementValuesToList( MeasurementValues values ) {
   py::list list( values.size() );
   py::ssize_t index = 0;
   for( auto& value: values ) {
      PyList_SET_ITEM( list.ptr(), index++, py::cast( value ).release().ptr() );
   }
   return list.release();
}

dip::Polygon BufferToPolygon( py::buffer& buf ) {
   py::buffer_info info = buf.request();
   //std::cout << "--Constructing dip::Polygon from Python buffer.\n";
   //std::cout << "   info.ptr = " << info.ptr << std::endl;
   //std::cout << "   info.format = " << info.format << std::endl;
   //std::cout << "   info.ndims = " << info.shape.size() << std::endl;
   //std::cout << "   info.size = " << info.size << std::endl;
   //std::cout << "   info.itemsize = " << info.itemsize << std::endl;
   //std::cout << "   info.shape[0] = " << info.shape[0] << std::endl;
   //std::cout << "   info.shape[1] = " << info.shape[1] << std::endl;
   //std::cout << "   info.strides[0] = " << info.strides[0] << std::endl;
   //std::cout << "   info.strides[1] = " << info.strides[1] << std::endl;
   if( info.format[ 0 ] != py::format_descriptor< dip::dfloat >::c ) {
      // TODO: Cast other types to double floats
      std::cout << "Attempted to convert buffer to dip.Polygon object: data must be double-precision floats." << std::endl;
      DIP_THROW( "Buffer data type not compatible with class Polygon" );
   }
   if(( info.shape.size() != 2 ) || ( info.shape[ 1 ] != 2 )) {
      std::cout << "Attempted to convert buffer to dip.Polygon object: data must have two columns." << std::endl;
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
   DIP_THROW_IF( stride * static_cast< dip::sint >( info.itemsize ) != info.strides[ 1 ],
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

} // namespace

void init_measurement( py::module& m ) {
   auto mm = m.def_submodule("MeasurementTool", "A tool to quantify objects in an image.");

   // dip::Measurement::FeatureInformation
   auto fInfo = py::class_< dip::Measurement::FeatureInformation >( mm, "FeatureInformation", "Information about one measurement feature." );
   fInfo.def( "__repr__", []( dip::Measurement::FeatureInformation const& self ) {
                 std::ostringstream os;
                 os << "<FeatureInformation: name = " << self.name << ", numberValues = " << self.numberValues << ">";
                 return os.str();
              } );
   fInfo.def_readonly( "name", &dip::Measurement::FeatureInformation::name );
   fInfo.def_readonly( "startColumn", &dip::Measurement::FeatureInformation::startColumn );
   fInfo.def_readonly( "numberValues", &dip::Measurement::FeatureInformation::numberValues );

   // dip::Feature::ValueInformation
   auto vInfo = py::class_< dip::Feature::ValueInformation >( mm, "ValueInformation", "Information about one measurement feature value." );
   vInfo.def( "__repr__", []( dip::Feature::ValueInformation const& self ) {
                 std::ostringstream os;
                 os << "<ValueInformation: name = " << self.name << ", units = " << self.units << ">";
                 return os.str();
              } );
   vInfo.def_readonly( "name", &dip::Feature::ValueInformation::name );
   vInfo.def_readonly( "units", &dip::Feature::ValueInformation::units );

   // dip::Measurement::IteratorFeature
   auto feat = py::class_< dip::Measurement::IteratorFeature >( mm, "MeasurementFeature", "A Measurement table column group representing the results for one\nfeature." );
   feat.def( "__repr__", []( dip::Measurement::IteratorFeature const& self ) {
                std::ostringstream os;
                os << "<MeasurementFeature for feature " << self.FeatureName() << " and " << self.NumberOfObjects() << " objects>";
                return os.str();
             } );
   feat.def( "__getitem__", []( dip::Measurement::IteratorFeature const& self, dip::uint objectID ) { return MeasurementValuesToList( self[ objectID ] ); }, "objectID"_a );
   feat.def( "FeatureName", &dip::Measurement::IteratorFeature::FeatureName );
   feat.def( "NumberOfValues", &dip::Measurement::IteratorFeature::NumberOfValues );
   feat.def( "NumberOfObjects", &dip::Measurement::IteratorFeature::NumberOfObjects );
   feat.def( "Objects", &dip::Measurement::IteratorFeature::Objects );

   // dip::Measurement::IteratorObject
   auto obj = py::class_< dip::Measurement::IteratorObject >( mm, "MeasurementObject", "A Measurement table row representing the results for one object." );
   obj.def( "__repr__", []( dip::Measurement::IteratorObject const& self ) {
               std::ostringstream os;
               os << "<MeasurementObject with " << self.NumberOfFeatures() << " features for object " << self.ObjectID() << ">";
               return os.str();
            } );
   obj.def( "__getitem__", []( dip::Measurement::IteratorObject const& self, dip::String const& name ) { return MeasurementValuesToList( self[ name ] ); }, "name"_a );
   obj.def( "ObjectID", &dip::Measurement::IteratorObject::ObjectID );
   obj.def( "NumberOfFeatures", &dip::Measurement::IteratorObject::NumberOfFeatures );
   obj.def( "Features", &dip::Measurement::IteratorObject::Features );

   // dip::Measurement
   auto meas = py::class_< dip::Measurement >( mm, "Measurement", "The result of a call to PyDIP.MeasurementTool.Measure, a table with a column group for\neach feature and a row for each object." );
   meas.def( "__repr__", []( dip::Measurement const& self ) {
                std::ostringstream os;
                os << "<Measurement with " << self.NumberOfFeatures() << " features for " << self.NumberOfObjects() << " objects>";
                return os.str();
             } );
   meas.def( "__str__", []( dip::Measurement const& self ) { std::ostringstream os; os << self; return os.str(); } );
   meas.def( "__getitem__", py::overload_cast< dip::uint >( &dip::Measurement::operator[], py::const_ ), "objectID"_a, py::return_value_policy::reference_internal );
   meas.def( "__getitem__", py::overload_cast< dip::String const& >( &dip::Measurement::operator[], py::const_ ), "name"_a, py::return_value_policy::reference_internal );
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
   mm.def( "Measure", []( dip::Image const& label, dip::Image const& grey, dip::StringArray const& features, dip::UnsignedArray const& objectIDs, dip::uint connectivity ) {
              return measurementTool.Measure( label, grey, features, objectIDs, connectivity );
           }, "label"_a, "grey"_a = dip::Image{}, "features"_a = dip::StringArray{ "Size" }, "objectIDs"_a = dip::StringArray{}, "connectivity"_a = 0 );
   mm.def( "Features", []() {
              auto features = measurementTool.Features();
              std::vector< std::tuple< dip::String, dip::String >> out;
              for( auto const& f : features ) {
                 dip::String description = f.description;
                 if( f.needsGreyValue ) {
                    description += "*";
                 }
                 out.emplace_back( f.name, description );
              }
              return out;
           } );

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
          }, "featureValues"_a );
   m.def( "SampleStatistics", &dip::SampleStatistics, "featureValues"_a );

   // dip::Polygon
   auto poly = py::class_< dip::Polygon >( m, "Polygon", py::buffer_protocol(), "A polygon representing a 2D object." );
   poly.def( py::init([]( py::buffer& buf ) { return BufferToPolygon( buf ); } ));
   py::implicitly_convertible< py::buffer, dip::Polygon >();
   poly.def_buffer( []( dip::Polygon& self ) -> py::buffer_info { return PolygonToBuffer( self ); } );
   poly.def( "__repr__", []( dip::Polygon const& self ) {
                std::ostringstream os;
                os << "<Polygon with " << self.vertices.size() << " vertices>";
                return os.str();
             } );
   poly.def( "BoundingBox", []( dip::Polygon const& self ){
                auto bb = self.BoundingBox();
                auto topLeft = py::make_tuple( bb.topLeft.x, bb.topLeft.y );
                auto bottomRight = py::make_tuple( bb.bottomRight.x, bb.bottomRight.y );
                return py::make_tuple( topLeft, bottomRight ).release();
             } );
   poly.def( "IsClockWise", &dip::Polygon::IsClockWise );
   poly.def( "Area", &dip::Polygon::Area );
   poly.def( "Centroid", []( dip::Polygon const& self ) {
                auto centroid = self.Centroid();
                return py::make_tuple( centroid.x, centroid.y ).release();
             } );
   poly.def( "Length", &dip::Polygon::Length ); // is the perimeter
   poly.def( "EllipseParameters", []( dip::Polygon const& self ) { return self.CovarianceMatrix().Ellipse(); } );
   poly.def( "RadiusStatistics", py::overload_cast<>( &dip::Polygon::RadiusStatistics, py::const_ ));
   poly.def( "EllipseVariance", py::overload_cast<>( &dip::Polygon::EllipseVariance, py::const_ ));
   poly.def( "ConvexHull", []( dip::Polygon const& self ) {
                auto out = self.ConvexHull().Polygon(); // Make a copy of the polygon, sadly. Otherwise we'd have to return the ConvexHull object. We can't extract that data from it trivially.
                return out;
             } );
   poly.def( "Feret", []( dip::Polygon const& self ) { return self.ConvexHull().Feret(); } );

   // dip::ChainCode
   auto chain = py::class_< dip::ChainCode >( m, "ChainCode", "" );
   chain.def( "__repr__", []( dip::ChainCode const& self ) {
                 std::ostringstream os;
                 os << "<ChainCode for object #" << self.objectID << ">";
                 return os.str();
              } );
   chain.def_property_readonly( "start", []( dip::ChainCode const& self ) { return py::make_tuple( self.start.x, self.start.y ).release(); } );
   chain.def_readonly( "objectID", &dip::ChainCode::objectID );
   chain.def_readonly( "is8connected", &dip::ChainCode::is8connected );
   chain.def( "ConvertTo8Connected", &dip::ChainCode::ConvertTo8Connected );
   chain.def( "Length", &dip::ChainCode::Length );
   chain.def( "Feret", &dip::ChainCode::Feret, "angleStep"_a = 5.0 / 180.0 * dip::pi );
   chain.def( "BendingEnergy", &dip::ChainCode::BendingEnergy );
   chain.def( "BoundingBox", []( dip::ChainCode const& self ){
      auto bb = self.BoundingBox();
      auto topLeft = py::make_tuple( bb.topLeft.x, bb.topLeft.y );
      auto bottomRight = py::make_tuple( bb.bottomRight.x, bb.bottomRight.y );
      return py::make_tuple( topLeft, bottomRight ).release();
   } );
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
                         os << ">";
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
                         os << ">";
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
                         os << ">";
                         return os.str();
                      } );
   radiusVals.def_property_readonly( "mean", &dip::RadiusValues::Mean );
   radiusVals.def_property_readonly( "standardDev", &dip::RadiusValues::StandardDeviation );
   radiusVals.def_property_readonly( "maximum", &dip::RadiusValues::Maximum );
   radiusVals.def_property_readonly( "minimum", &dip::RadiusValues::Minimum );
   radiusVals.def_property_readonly( "circularity", &dip::RadiusValues::Circularity );
}
