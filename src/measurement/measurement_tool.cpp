/*
 * (c)2016-2022, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
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

#include "diplib/measurement.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "diplib.h"
#include "diplib/chain_code.h"
#include "diplib/framework.h"
#include "diplib/iterators.h"
#include "diplib/polygon.h"
#include "diplib/regions.h"

// FEATURES:
#include "feature_common_stuff.h" // IWYU pragma: keep
// Size
#include "feature_size.h"
#include "feature_solid_area.h"
#include "feature_minimum.h"
#include "feature_maximum.h"
#include "feature_cartesian_box.h"
#include "feature_perimeter.h"
#include "feature_surface_area.h"
#include "feature_feret.h"
#include "feature_convex_area.h"
#include "feature_convex_perimeter.h"
// Shape
#include "feature_aspect_ratio_feret.h"
#include "feature_radius.h"
#include "feature_p2a.h"
#include "feature_roundness.h"
#include "feature_circularity.h"
#include "feature_podczeck_shapes.h"
#include "feature_solidity.h"
#include "feature_convexity.h"
#include "feature_ellipse_variance.h"
#include "feature_eccentricity.h"
#include "feature_bending_energy.h"
// Intensity
#include "feature_mass.h"
#include "feature_mean.h"
#include "feature_stdandard_deviation.h"
#include "feature_statistics.h"
#include "feature_directional_statistics.h"
#include "feature_max_val.h"
#include "feature_min_val.h"
#include "feature_max_pos.h"
#include "feature_min_pos.h"
// Binary moments
#include "feature_center.h"
#include "feature_mu.h"
#include "feature_inertia.h"
#include "feature_major_axes.h"
#include "feature_dimensions_cube.h"
#include "feature_dimensions_ellipsoid.h"
// Grey-value moments
#include "feature_grey_size.h"
#include "feature_gravity.h"
#include "feature_grey_mu.h"
#include "feature_grey_inertia.h"
#include "feature_grey_major_axes.h"
#include "feature_grey_dimensions_cube.h"
#include "feature_grey_dimensions_ellipsoid.h"

namespace dip {


MeasurementTool::MeasurementTool() {
   // Size
   Register( new Feature::FeatureSize );
   Register( new Feature::FeatureSolidArea );
   Register( new Feature::FeaturePerimeter );
   Register( new Feature::FeatureSurfaceArea );
   Register( new Feature::FeatureMinimum );
   Register( new Feature::FeatureMaximum );
   Register( new Feature::FeatureCartesianBox );
   Register( new Feature::FeatureFeret );
   Register( new Feature::FeatureRadius );
   Register( new Feature::FeatureConvexArea );
   Register( new Feature::FeatureConvexPerimeter );
   // Shape
   Register( new Feature::FeatureAspectRatioFeret );
   Register( new Feature::FeatureP2A );
   Register( new Feature::FeatureRoundness );
   Register( new Feature::FeatureCircularity );
   Register( new Feature::FeaturePodczeckShapes );
   Register( new Feature::FeatureSolidity );
   Register( new Feature::FeatureConvexity );
   Register( new Feature::FeatureEllipseVariance );
   Register( new Feature::FeatureEccentricity );
   Register( new Feature::FeatureBendingEnergy );
   // Intensity
   Register( new Feature::FeatureMass );
   Register( new Feature::FeatureMean );
   Register( new Feature::FeatureStandardDeviation );
   Register( new Feature::FeatureStatistics );
   Register( new Feature::FeatureDirectionalStatistics );
   Register( new Feature::FeatureMaxVal );
   Register( new Feature::FeatureMinVal );
   Register( new Feature::FeatureMaxPos );
   Register( new Feature::FeatureMinPos );
   // Binary moments
   Register( new Feature::FeatureCenter );
   Register( new Feature::FeatureMu );
   Register( new Feature::FeatureInertia );
   Register( new Feature::FeatureMajorAxes );
   Register( new Feature::FeatureDimensionsCube );
   Register( new Feature::FeatureDimensionsEllipsoid );
   // Grey-value moments
   Register( new Feature::FeatureGreySize );
   Register( new Feature::FeatureGravity );
   Register( new Feature::FeatureGreyMu );
   Register( new Feature::FeatureGreyInertia );
   Register( new Feature::FeatureGreyMajorAxes );
   Register( new Feature::FeatureGreyDimensionsCube );
   Register( new Feature::FeatureGreyDimensionsEllipsoid );
}

using LineBasedFeatureArray = std::vector< Feature::LineBased* >;
using FeatureArray = std::vector< Feature::Base* >;


namespace {

// dip::Framework::ScanFilter function, not overloaded because the Feature::LineBased::ScanLine functions
// that we call here are not overloaded.
class MeasureLineFilter : public Framework::ScanLineFilter {
   public:
      // not defining GetNumberOfOperations(), always called in a single thread
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         LineIterator< LabelType > label(
               static_cast< LabelType* >( params.inBuffer[ 0 ].buffer ),
               0, params.bufferLength, params.inBuffer[ 0 ].stride,
               params.inBuffer[ 0 ].tensorLength, params.inBuffer[ 0 ].tensorStride
         );
         LineIterator< dfloat > grey;
         if( params.inBuffer.size() > 1 ) {
            grey = LineIterator< dfloat >(
                  static_cast< dfloat* >( params.inBuffer[ 1 ].buffer ),
                  0, params.bufferLength, params.inBuffer[ 1 ].stride,
                  params.inBuffer[ 1 ].tensorLength, params.inBuffer[ 1 ].tensorStride
            );
         }

         for( auto const& feature : features ) {
            // NOTE! params.dimension here works as long as params.tensorToSpatial is false.
            // As is now, MeasurementTool::Measure only works with scalar images, so we don't need to test here.
            feature->ScanLine( label, grey, params.position, params.dimension, objectIndices );
         }
      }
      MeasureLineFilter( LineBasedFeatureArray const& features, ObjectIdToIndexMap const& objectIndices ) :
            features( features ), objectIndices( objectIndices ) {}
   private:
      LineBasedFeatureArray const& features;
      ObjectIdToIndexMap const& objectIndices;
};

} // namespace

Measurement MeasurementTool::Measure(
      Image const& label,
      Image const& grey,
      StringArray features, // copy
      UnsignedArray const& objectIDs,
      dip::uint connectivity
) const {

   // Check input
   DIP_THROW_IF( !label.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   if( grey.IsForged() ) {
      DIP_THROW_IF( !grey.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_STACK_TRACE_THIS( grey.CompareProperties( label, Option::CmpProp::Sizes ));
   }

   Measurement measurement;

   // Fill out the object IDs
   std::vector< LabelType > labelList;
   if( objectIDs.empty() ) {
      labelList = ListObjectLabels( label, Image{}, S::EXCLUDE );
      UnsignedArray ids( labelList.size() );
      std::copy( labelList.begin(), labelList.end(), ids.begin() );
      measurement.SetObjectIDs( ids );
   } else {
      measurement.SetObjectIDs( objectIDs );
   }

   // Parse the features array and prepare measurements
   DIP_THROW_IF( features.empty(), "No features given" );
   FeatureArray featureArray;
   featureArray.reserve( features.size() );
   for( dip::uint ii = 0; ii < features.size(); ++ii ) { // NOTE! `features` can expand every iteration
      String const& name = features[ ii ];
      if( !measurement.FeatureExists( name )) {
         Feature::Base* feature = features_[ Index( name ) ].get();
         if( feature->information.needsGreyValue ) {
            DIP_THROW_IF( !grey.IsForged(), "Measurement feature requires grey-value image" );
         }
         featureArray.push_back( feature );
         DIP_START_STACK_TRACE
            Feature::ValueInformationArray values = feature->Initialize( label, grey, measurement.NumberOfObjects() );
            measurement.AddFeature( name, values );
         DIP_END_STACK_TRACE
         if( feature->type == Feature::Type::COMPOSITE ) {
            StringArray names = dynamic_cast< Feature::Composite* >( feature )->Dependencies();
            for( auto const& n : names ) {
               features.push_back( n ); // Add features needed for composite measure to the list of features to process in this loop
            }
         }
      }
   }

   // Allocate memory for all features and objects
   measurement.Forge();
   if( measurement.NumberOfObjects() == 0 ) {
      // There's no objects to be measured. We're done.
      return measurement;
   }

   // Figure out which types of measurements we want to do
   bool doLineBased = false;
   LineBasedFeatureArray lineBasedFeatures;
   bool doImageBased = false;
   bool doChaincodeBased = false;
   bool doPolygonBased = false;
   bool doConvHullBased = false;
   bool doComposite = false;
   for( auto const& feature : featureArray ) {
      switch( feature->type ) {
         case Feature::Type::LINE_BASED:
            doLineBased = true;
            lineBasedFeatures.emplace_back( dynamic_cast< Feature::LineBased* >( feature ));
            break;
         case Feature::Type::IMAGE_BASED:
            doImageBased = true;
            break;
         case Feature::Type::CHAINCODE_BASED:
            doChaincodeBased = true;
            break;
         case Feature::Type::POLYGON_BASED:
            doPolygonBased = true;
            break;
         case Feature::Type::CONVEXHULL_BASED:
            doConvHullBased = true;
            break;
         case Feature::Type::COMPOSITE:
            doComposite = true;
            break;
      }
   }

   // Let the line based functions do their work
   if( doLineBased ) {

      // Create arrays for Scan framework
      ImageConstRefArray inar{ label };
      DataTypeArray inBufT{ DT_LABEL };
      if( grey.IsForged() ) {
         inar.emplace_back( grey );
         inBufT.push_back( DT_DFLOAT );
      }
      ImageRefArray outar{};

      // Do the scan, which calls dip::Feature::LineBased::ScanLine()
      MeasureLineFilter functor{ lineBasedFeatures, measurement.ObjectIndices() };
      Framework::Scan( inar, outar, inBufT, {}, {}, {}, functor,
            Framework::ScanOption::NoMultiThreading + Framework::ScanOption::NeedCoordinates );

      // Call dip::Feature::LineBased::Finish()
      for( auto const& feature : lineBasedFeatures ) {
         Measurement::IteratorFeature column = measurement[ feature->information.name ];
         Measurement::IteratorFeature::Iterator it = column.FirstObject();
         do {
            feature->Finish( it.ObjectIndex(), it.data() );
         } while( ++it );
      }
   }

   // Let the image based functions do their work
   if( doImageBased ) {
      for( auto const& feature : featureArray ) {
         if( feature->type == Feature::Type::IMAGE_BASED ) {
            Measurement::IteratorFeature column = measurement[ feature->information.name ];
            dynamic_cast< Feature::ImageBased* >( feature )->Measure( label, grey, column );
         }
      }
   }

   // Let the chaincode based functions do their work
   if( doChaincodeBased || doPolygonBased || doConvHullBased ) {
      if( labelList.empty() ) {
         auto const& ids = measurement.Objects();
         labelList.resize( ids.size() );
         std::transform( ids.begin(), ids.end(), labelList.begin(), []( dip::uint v ){ return CastLabelType( v ); } );
      }
      ChainCodeArray chainCodeArray = GetImageChainCodes( label, labelList, connectivity );
      auto itCC = chainCodeArray.begin();
      auto itObj = measurement.FirstObject(); // these two arrays are ordered the same way
      do {
         Polygon polygon;
         ConvexHull convexHull;
         if( doPolygonBased || doConvHullBased ) {
            polygon = itCC->Polygon();
         }
         if( doConvHullBased ) {
            convexHull = polygon.ConvexHull();
         }
         for( auto const& feature : featureArray ) {
            if( feature->type == Feature::Type::CHAINCODE_BASED ) {
               auto cell = itObj[ feature->information.name ];
               dynamic_cast< Feature::ChainCodeBased* >( feature )->Measure( *itCC, cell.data() );
            } else if( feature->type == Feature::Type::POLYGON_BASED ) {
               auto cell = itObj[ feature->information.name ];
               dynamic_cast< Feature::PolygonBased* >( feature )->Measure( polygon, cell.data() );
            } else if( feature->type == Feature::Type::CONVEXHULL_BASED ) {
               auto cell = itObj[ feature->information.name ];
               dynamic_cast< Feature::ConvexHullBased* >( feature )->Measure( convexHull, cell.data() );
            }
         }
      } while( ++itCC, ++itObj );
   }

   // Let the composite functions do their work
   if( doComposite ) {
      Measurement::IteratorObject row = measurement.FirstObject();
      do {
         for( auto const& feature : featureArray ) {
            if( feature->type == Feature::Type::COMPOSITE ) {
               auto cell = row[ feature->information.name ];
               dynamic_cast< Feature::Composite* >( feature )->Compose( row, cell.data() );
            }
         }
      } while( ++row );
   }

   // Clean up
   for( auto const& feature : featureArray ) {
      feature->Cleanup();
   }

   return measurement;
}

} // namespace dip


#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/generation.h"

DOCTEST_TEST_CASE( "[DIPlib] testing dip::MeasurementTool::Measure" ) {
   // A test image with a single circle
   dip::dfloat r = 22;
   dip::Image img = dip::CreateRadiusCoordinate( { 50, 50 } ) < r;
   img.Convert( dip::DT_UINT8 ); // copyless conversion from binary to unsigned integer -- like calling dip::Label()!

   // Measure everything (except SurfaceArea, which requires a 3D image)
   dip::MeasurementTool measurementTool;
   auto msr = measurementTool.Measure( img, img, {
         "Size",
         "Minimum",
         "Maximum",
         "CartesianBox",
         "Perimeter",
         "Feret",
         "SolidArea",
         "ConvexArea",
         "ConvexPerimeter",
         "AspectRatioFeret",
         "Radius",
         "P2A",
         "Roundness",
         "Circularity",
         "PodczeckShapes",
         "Solidity",
         "Convexity",
         "EllipseVariance",
         "Eccentricity",
         "BendingEnergy",
         "Mass",
         "Mean",
         "StandardDeviation",
         "Statistics",
         "DirectionalStatistics",
         "MaxVal",
         "MinVal",
         "MaxPos",
         "MinPos",
         "Center",
         "Mu",
         "Inertia",
         "MajorAxes",
         "DimensionsCube",
         "DimensionsEllipsoid",
         "GreySize",
         "Gravity",
         "GreyMu",
         "GreyInertia",
         "GreyMajorAxes",
         "GreyDimensionsCube",
         "GreyDimensionsEllipsoid",
   } );

   // Verify all measurements
   DOCTEST_REQUIRE( msr.IsForged() );
   DOCTEST_REQUIRE( msr.ObjectExists( 1 ));
   auto msr_obj = msr[ 1 ];
   DOCTEST_CHECK( std::abs( msr_obj[ "Size" ][ 0 ] - dip::pi * r * r ) < 8 );
   DOCTEST_CHECK( msr_obj[ "Minimum" ][ 0 ] == 4 );
   DOCTEST_CHECK( msr_obj[ "Minimum" ][ 1 ] == 4 );
   DOCTEST_CHECK( msr_obj[ "Maximum" ][ 0 ] == 46 );
   DOCTEST_CHECK( msr_obj[ "Maximum" ][ 1 ] == 46 );
   DOCTEST_CHECK( msr_obj[ "CartesianBox" ][ 0 ] == 2 * r - 1 );
   DOCTEST_CHECK( msr_obj[ "CartesianBox" ][ 1 ] == 2 * r - 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Perimeter" ][ 0 ] - 2 * dip::pi * r ) < 0.08 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 0 ] - 2 * r ) < 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 1 ] - 2 * r ) < 1.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 2 ] - 2 * r ) < 1.1 );
   // DOCTEST_CHECK( msr_obj[ "Feret" ][ 3 ] == 0); // arbitrary angle, ignore
   // DOCTEST_CHECK( msr_obj[ "Feret" ][ 4 ] == 0); // arbitrary angle, ignore
   DOCTEST_CHECK( std::abs( msr_obj[ "SolidArea" ][ 0 ] - dip::pi * r * r ) < 8 );
   DOCTEST_CHECK( std::abs( msr_obj[ "ConvexArea" ][ 0 ] - dip::pi * r * r ) < 17 );
   DOCTEST_CHECK( std::abs( msr_obj[ "ConvexPerimeter" ][ 0 ] - 2 * dip::pi * r ) < 1.5 );
   DOCTEST_CHECK( std::abs( msr_obj[ "AspectRatioFeret" ][ 0 ] - 1 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 0 ] - r ) < 0.4 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 1 ] - r ) < 0.04 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 2 ] - r ) < 0.51 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 3 ] - 0 ) < 0.3 );
   DOCTEST_CHECK( std::abs( msr_obj[ "P2A" ][ 0 ] - 1 ) < 0.007 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Roundness" ][ 0 ] - 1 ) < 0.007 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Circularity" ][ 0 ] - 0 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 0 ] - dip::pi / 4 ) < 0.04 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 1 ] - 1 ) < 0.05 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 2 ] - dip::pi / 2 ) < 0.07 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 3 ] - 1 ) < 0.05 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 4 ] - dip::pi ) < 0.06 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Solidity" ][ 0 ] - 1 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Convexity" ][ 0 ] - 1 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "EllipseVariance" ][ 0 ] - 0 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Eccentricity" ][ 0 ] - 0 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "BendingEnergy" ][ 0 ] - 2 * dip::pi / r ) < 0.03 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mass" ][ 0 ] - dip::pi * r * r ) < 8 );
   DOCTEST_CHECK( msr_obj[ "Mean" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "StandardDeviation" ][ 0 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 3 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "DirectionalStatistics" ][ 0 ] == doctest::Approx( 1.0 ));
   DOCTEST_CHECK( msr_obj[ "DirectionalStatistics" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MaxVal" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "MinVal" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "MaxPos" ][ 0 ] == 19 );
   DOCTEST_CHECK( msr_obj[ "MaxPos" ][ 1 ] == 4 );
   DOCTEST_CHECK( msr_obj[ "MinPos" ][ 0 ] == 19 );
   DOCTEST_CHECK( msr_obj[ "MinPos" ][ 1 ] == 4 );
   DOCTEST_CHECK( msr_obj[ "Center" ][ 0 ] == 25 );
   DOCTEST_CHECK( msr_obj[ "Center" ][ 1 ] == 25 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mu" ][ 0 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mu" ][ 1 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( msr_obj[ "Mu" ][ 2 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Inertia" ][ 0 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Inertia" ][ 1 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 3 ] == 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsCube" ][ 0 ] - 2 * r * std::sqrt( 12.0 / 16.0 )) < 0.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsCube" ][ 1 ] - 2 * r * std::sqrt( 12.0 / 16.0 )) < 0.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsEllipsoid" ][ 0 ] - 2 * r ) < 0.2 );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsEllipsoid" ][ 1 ] - 2 * r ) < 0.2 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreySize" ][ 0 ] - dip::pi * r * r ) < 8 );
   DOCTEST_CHECK( msr_obj[ "Gravity" ][ 0 ] == 25 );
   DOCTEST_CHECK( msr_obj[ "Gravity" ][ 1 ] == 25 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyMu" ][ 0 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyMu" ][ 1 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( msr_obj[ "GreyMu" ][ 2 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyInertia" ][ 0 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyInertia" ][ 1 ] - r * r / 4 ) < 0.6 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 3 ] == 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsCube" ][ 0 ] - 2 * r * std::sqrt( 12.0 / 16.0 )) < 0.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsCube" ][ 1 ] - 2 * r * std::sqrt( 12.0 / 16.0 )) < 0.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsEllipsoid" ][ 0 ] - 2 * r ) < 0.2 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsEllipsoid" ][ 1 ] - 2 * r ) < 0.2 );

   // Repeat the above, but with a pixel size and scaling the gray values
   img *= 2; // the object ID is now also 2!
   dip::dfloat ps = 0.21;
   img.SetPixelSize( ps * dip::Units::Micrometer() );
   msr = measurementTool.Measure( img, img, {
         "Size",
         "Minimum",
         "Maximum",
         "CartesianBox",
         "Perimeter",
         "Feret",
         "SolidArea",
         "ConvexArea",
         "ConvexPerimeter",
         "AspectRatioFeret",
         "Radius",
         "P2A",
         "Roundness",
         "Circularity",
         "PodczeckShapes",
         "Solidity",
         "Convexity",
         "EllipseVariance",
         "Eccentricity",
         "BendingEnergy",
         "Mass",
         "Mean",
         "StandardDeviation",
         "Statistics",
         "DirectionalStatistics",
         "MaxVal",
         "MinVal",
         "MaxPos",
         "MinPos",
         "Center",
         "Mu",
         "Inertia",
         "MajorAxes",
         "DimensionsCube",
         "DimensionsEllipsoid",
         "GreySize",
         "Gravity",
         "GreyMu",
         "GreyInertia",
         "GreyMajorAxes",
         "GreyDimensionsCube",
         "GreyDimensionsEllipsoid",
   } );
   DOCTEST_REQUIRE( msr.IsForged() );
   DOCTEST_REQUIRE( msr.ObjectExists( 2 ));
   msr_obj = msr[ 2 ];
   DOCTEST_CHECK( std::abs( msr_obj[ "Size" ][ 0 ] - dip::pi * r * r * ps * ps ) < 8 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "Minimum" ][ 0 ] == 4 * ps );
   DOCTEST_CHECK( msr_obj[ "Minimum" ][ 1 ] == 4 * ps );
   DOCTEST_CHECK( msr_obj[ "Maximum" ][ 0 ] == 46 * ps );
   DOCTEST_CHECK( msr_obj[ "Maximum" ][ 1 ] == 46 * ps );
   DOCTEST_CHECK( msr_obj[ "CartesianBox" ][ 0 ] == ( 2 * r - 1 ) * ps );
   DOCTEST_CHECK( msr_obj[ "CartesianBox" ][ 1 ] == ( 2 * r - 1 ) * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Perimeter" ][ 0 ] - 2 * dip::pi * r * ps ) < 0.08 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 0 ] - 2 * r * ps ) < 1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 1 ] - 2 * r * ps ) < 1.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 2 ] - 2 * r * ps ) < 1.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "SolidArea" ][ 0 ] - dip::pi * r * r * ps * ps ) < 8 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "ConvexArea" ][ 0 ] - dip::pi * r * r * ps * ps ) < 17 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "ConvexPerimeter" ][ 0 ] - 2 * dip::pi * r * ps ) < 1.5 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "AspectRatioFeret" ][ 0 ] - 1 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 0 ] - r * ps ) < 0.4 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 1 ] - r * ps ) < 0.04 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 2 ] - r * ps ) < 0.51 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 3 ] - 0 ) < 0.3 );
   DOCTEST_CHECK( std::abs( msr_obj[ "P2A" ][ 0 ] - 1 ) < 0.007 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Roundness" ][ 0 ] - 1 ) < 0.007 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Circularity" ][ 0 ] - 0 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 0 ] - dip::pi / 4 ) < 0.04 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 1 ] - 1 ) < 0.05 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 2 ] - dip::pi / 2 ) < 0.07 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 3 ] - 1 ) < 0.05 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 4 ] - dip::pi ) < 0.06 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Solidity" ][ 0 ] - 1 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Convexity" ][ 0 ] - 1 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "EllipseVariance" ][ 0 ] - 0 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Eccentricity" ][ 0 ] - 0 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "BendingEnergy" ][ 0 ] - 2 * dip::pi / r / ps ) < 0.03 / ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mass" ][ 0 ] - 2 * dip::pi * r * r ) < 2 * 16 );
   DOCTEST_CHECK( msr_obj[ "Mean" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "StandardDeviation" ][ 0 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 3 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "DirectionalStatistics" ][ 0 ] == doctest::Approx( 2.0 ));
   DOCTEST_CHECK( msr_obj[ "DirectionalStatistics" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MaxVal" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "MinVal" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "MaxPos" ][ 0 ] == 19 * ps );
   DOCTEST_CHECK( msr_obj[ "MaxPos" ][ 1 ] == 4 * ps );
   DOCTEST_CHECK( msr_obj[ "MinPos" ][ 0 ] == 19 * ps );
   DOCTEST_CHECK( msr_obj[ "MinPos" ][ 1 ] == 4 * ps );
   DOCTEST_CHECK( msr_obj[ "Center" ][ 0 ] == 25 * ps );
   DOCTEST_CHECK( msr_obj[ "Center" ][ 1 ] == 25 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mu" ][ 0 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mu" ][ 1 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "Mu" ][ 2 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Inertia" ][ 0 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Inertia" ][ 1 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 3 ] == 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsCube" ][ 0 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps ) < 0.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsCube" ][ 1 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps ) < 0.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsEllipsoid" ][ 0 ] - 2 * r * ps ) < 0.2 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsEllipsoid" ][ 1 ] - 2 * r * ps ) < 0.2 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreySize" ][ 0 ] - 2 * dip::pi * r * r * ps * ps ) < 2 * 8 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "Gravity" ][ 0 ] == 25 * ps );
   DOCTEST_CHECK( msr_obj[ "Gravity" ][ 1 ] == 25 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyMu" ][ 0 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyMu" ][ 1 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "GreyMu" ][ 2 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyInertia" ][ 0 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyInertia" ][ 1 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 0 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 3 ] == 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsCube" ][ 0 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps ) < 0.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsCube" ][ 1 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps ) < 0.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsEllipsoid" ][ 0 ] - 2 * r * ps ) < 0.2 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsEllipsoid" ][ 1 ] - 2 * r * ps ) < 0.2 * ps );

   // Repeat the above, but with an anisotropic pixel size
   dip::dfloat yscale = 1.3;
   img.SetPixelSize( 1, yscale * ps * dip::Units::Micrometer() );
   msr = measurementTool.Measure( img, img, {
         "Size",
         "Minimum",
         "Maximum",
         "CartesianBox",
         "Perimeter",
         "Feret",
         "SolidArea",
         "ConvexArea",
         "ConvexPerimeter",
         "AspectRatioFeret",
         "Radius",
         "P2A",
         "Roundness",
         "Circularity",
         "PodczeckShapes",
         "Solidity",
         "Convexity",
         "EllipseVariance",
         "Eccentricity",
         "BendingEnergy",
         "Mass",
         "Mean",
         "StandardDeviation",
         "Statistics",
         "DirectionalStatistics",
         "MaxVal",
         "MinVal",
         "MaxPos",
         "MinPos",
         "Center",
         "Mu",
         "Inertia",
         "MajorAxes",
         "DimensionsCube",
         "DimensionsEllipsoid",
         "GreySize",
         "Gravity",
         "GreyMu",
         "GreyInertia",
         "GreyMajorAxes",
         "GreyDimensionsCube",
         "GreyDimensionsEllipsoid",
   } );
   DOCTEST_REQUIRE( msr.IsForged() );
   DOCTEST_REQUIRE( msr.ObjectExists( 2 ));
   msr_obj = msr[ 2 ];
   DOCTEST_CHECK( std::abs( msr_obj[ "Size" ][ 0 ] - dip::pi * r * r * ps * ps * yscale ) < 8 * ps * ps * yscale );
   DOCTEST_CHECK( msr_obj[ "Minimum" ][ 0 ] == 4 * ps );
   DOCTEST_CHECK( msr_obj[ "Minimum" ][ 1 ] == doctest::Approx( 4 * ps * yscale ));
   DOCTEST_CHECK( msr_obj[ "Maximum" ][ 0 ] == 46 * ps );
   DOCTEST_CHECK( msr_obj[ "Maximum" ][ 1 ] == doctest::Approx( 46 * ps * yscale ));
   DOCTEST_CHECK( msr_obj[ "CartesianBox" ][ 0 ] == ( 2 * r - 1 ) * ps );
   DOCTEST_CHECK( msr_obj[ "CartesianBox" ][ 1 ] == doctest::Approx(( 2 * r - 1 ) * ps * yscale ));
   DOCTEST_CHECK( std::abs( msr_obj[ "Perimeter" ][ 0 ] - 2 * dip::pi * r ) < 0.08 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 0 ] - 2 * r ) < 1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 1 ] - 2 * r ) < 1.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Feret" ][ 2 ] - 2 * r ) < 1.1 );
   DOCTEST_CHECK( std::abs( msr_obj[ "SolidArea" ][ 0 ] - dip::pi * r * r * ps * ps * yscale  ) < 8 * ps * ps * yscale  );
   DOCTEST_CHECK( std::abs( msr_obj[ "ConvexArea" ][ 0 ] - dip::pi * r * r * ps * ps * yscale  ) < 17 * ps * yscale  );
   DOCTEST_CHECK( std::abs( msr_obj[ "ConvexPerimeter" ][ 0 ] - 2 * dip::pi * r ) < 1.5 );
   DOCTEST_CHECK( std::abs( msr_obj[ "AspectRatioFeret" ][ 0 ] - 1 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 0 ] - r ) < 0.4 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 1 ] - r ) < 0.04 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 2 ] - r ) < 0.51 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Radius" ][ 3 ] - 0 ) < 0.3 );
   DOCTEST_CHECK( std::abs( msr_obj[ "P2A" ][ 0 ] - 1 ) < 0.007 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Roundness" ][ 0 ] - 1 ) < 0.007 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Circularity" ][ 0 ] - 0 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 0 ] - dip::pi / 4 ) < 0.04 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 1 ] - 1 ) < 0.05 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 2 ] - dip::pi / 2 ) < 0.07 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 3 ] - 1 ) < 0.05 );
   DOCTEST_CHECK( std::abs( msr_obj[ "PodczeckShapes" ][ 4 ] - dip::pi ) < 0.06 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Solidity" ][ 0 ] - 1 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Convexity" ][ 0 ] - 1 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "EllipseVariance" ][ 0 ] - 0 ) < 0.02 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Eccentricity" ][ 0 ] - 0 ) < 1e-6 );
   DOCTEST_CHECK( std::abs( msr_obj[ "BendingEnergy" ][ 0 ] - 2 * dip::pi / r ) < 0.03 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mass" ][ 0 ] - 2 * dip::pi * r * r ) < 2 * 16 );
   DOCTEST_CHECK( msr_obj[ "Mean" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "StandardDeviation" ][ 0 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 2 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "Statistics" ][ 3 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "DirectionalStatistics" ][ 0 ] == doctest::Approx( 2.0 ));
   DOCTEST_CHECK( msr_obj[ "DirectionalStatistics" ][ 1 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MaxVal" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "MinVal" ][ 0 ] == 2 );
   DOCTEST_CHECK( msr_obj[ "MaxPos" ][ 0 ] == 19 * ps );
   DOCTEST_CHECK( msr_obj[ "MaxPos" ][ 1 ] == doctest::Approx( 4 * ps * yscale  ));
   DOCTEST_CHECK( msr_obj[ "MinPos" ][ 0 ] == 19 * ps );
   DOCTEST_CHECK( msr_obj[ "MinPos" ][ 1 ] == doctest::Approx( 4 * ps * yscale  ));
   DOCTEST_CHECK( msr_obj[ "Center" ][ 0 ] == 25 * ps );
   DOCTEST_CHECK( msr_obj[ "Center" ][ 1 ] == doctest::Approx( 25 * ps * yscale  ));
   DOCTEST_CHECK( std::abs( msr_obj[ "Mu" ][ 0 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "Mu" ][ 1 ] - r * r / 4 * ps * ps * yscale * yscale ) < 0.6 * ps * ps * yscale * yscale  );
   DOCTEST_CHECK( msr_obj[ "Mu" ][ 2 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "Inertia" ][ 0 ] - r * r / 4 * ps * ps * yscale * yscale ) < 0.6 * ps * ps * yscale * yscale );
   DOCTEST_CHECK( std::abs( msr_obj[ "Inertia" ][ 1 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 0 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 1 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 2 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "MajorAxes" ][ 3 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsCube" ][ 0 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps * yscale ) < 0.1 * ps * yscale );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsCube" ][ 1 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps ) < 0.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsEllipsoid" ][ 0 ] - 2 * r * ps * yscale ) < 0.2 * ps * yscale );
   DOCTEST_CHECK( std::abs( msr_obj[ "DimensionsEllipsoid" ][ 1 ] - 2 * r * ps ) < 0.2 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreySize" ][ 0 ] - 2 * dip::pi * r * r * ps * ps * yscale ) < 2 * 8 * ps * ps * yscale );
   DOCTEST_CHECK( msr_obj[ "Gravity" ][ 0 ] == 25 * ps );
   DOCTEST_CHECK( msr_obj[ "Gravity" ][ 1 ] == doctest::Approx( 25 * ps * yscale ));
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyMu" ][ 0 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyMu" ][ 1 ] - r * r / 4 * ps * ps * yscale * yscale ) < 0.6 * ps * ps * yscale * yscale );
   DOCTEST_CHECK( msr_obj[ "GreyMu" ][ 2 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyInertia" ][ 0 ] - r * r / 4 * ps * ps * yscale * yscale ) < 0.6 * ps * ps * yscale * yscale );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyInertia" ][ 1 ] - r * r / 4 * ps * ps ) < 0.6 * ps * ps );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 0 ] == 0 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 1 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 2 ] == 1 );
   DOCTEST_CHECK( msr_obj[ "GreyMajorAxes" ][ 3 ] == 0 );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsCube" ][ 0 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps * yscale ) < 0.1 * ps * yscale );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsCube" ][ 1 ] - 2 * r * std::sqrt( 12.0 / 16.0 ) * ps ) < 0.1 * ps );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsEllipsoid" ][ 0 ] - 2 * r * ps * yscale ) < 0.2 * ps * yscale );
   DOCTEST_CHECK( std::abs( msr_obj[ "GreyDimensionsEllipsoid" ][ 1 ] - 2 * r * ps ) < 0.2 * ps );
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
