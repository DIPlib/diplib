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
#include "diplib/measurement.h"

dip::MeasurementTool measurementTool;

template< typename MeasurementValues >
py::handle MeasurementValuesToList( MeasurementValues values ) {
   py::list list( values.size());
   py::ssize_t index = 0;
   for( auto& value: values ) {
      PyList_SET_ITEM( list.ptr(), index++, py::cast( value ).release().ptr() );
   }
   return list.release();
}

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
   auto feat = py::class_< dip::Measurement::IteratorFeature >( mm, "Feature", "A Measurement table column group representing the results for one\nfeature." );
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
   auto obj = py::class_< dip::Measurement::IteratorObject >( mm, "Object", "A Measurement table row representing the results for one object." );
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
   auto meas = py::class_< dip::Measurement >( mm, "Measurement", "The result of a call to PyDIP.Measure, a table with a column group for\neach feature and a row for each object." );
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
           }, "label"_a, "grey"_a, "features"_a, "objectIDs"_a = dip::StringArray{}, "connectivity"_a = 0 );
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
   m.def( "WriteCSV", &dip::WriteCSV, "measurement"_a, "filename"_a, "options"_a = dip::StringSet{} );
   m.def( "Minimum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Minimum ), "featureValues"_a  );
   m.def( "Maximum", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Maximum ), "featureValues"_a  );
   m.def( "Percentile", py::overload_cast< dip::Measurement::IteratorFeature const&, dip::dfloat >( &dip::Percentile ), "featureValues"_a, "percentile"_a  );
   m.def( "Median", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Median ), "featureValues"_a  );
   m.def( "Mean", py::overload_cast< dip::Measurement::IteratorFeature const& >( &dip::Mean ), "featureValues"_a  );
   m.def( "MaximumAndMinimum", []( dip::Measurement::IteratorFeature const& featureValues ) {
             dip::MinMaxAccumulator acc = dip::MaximumAndMinimum( featureValues );
             return py::make_tuple( acc.Minimum(), acc.Maximum() ).release();
          }, "featureValues"_a );
   m.def( "SampleStatistics", []( dip::Measurement::IteratorFeature const& featureValues ) {
             dip::StatisticsAccumulator acc = dip::SampleStatistics( featureValues );
             return py::make_tuple( acc.Mean(), acc.Variance(), acc.Skewness(), acc.ExcessKurtosis() ).release();
          }, "featureValues"_a );
}
