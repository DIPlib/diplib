/*
 * DIPlib 3.0
 * This file contains definitions for functions in the dip::MeasurementTool class.
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */

#include <iostream>
#include <sstream>
#include <iomanip>

#include "diplib.h"
#include "diplib/measurement.h"
#include "diplib/framework.h"
#include "diplib/regions.h"

// FEATURES:
// Size
#include "feature_size.h"
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
#include "feature_ellipse_variance.h"
#include "feature_p2a.h"
#include "feature_podczeck_shapes.h"
#include "feature_convexity.h"
#include "feature_bending_energy.h"
// Intensity
#include "feature_mass.h"
#include "feature_mean.h"
#include "feature_stdandard_deviation.h"
#include "feature_statistics.h"
#include "feature_max_val.h"
#include "feature_min_val.h"
// Binary moments
#include "feature_center.h"
#include "feature_mu.h"
#include "feature_inertia.h"
#include "feature_major_axes.h"
#include "feature_dimensions_cube.h"
#include "feature_dimensions_ellipsoid.h"
// Grey-value moments
#include "feature_gravity.h"
#include "feature_grey_mu.h"
#include "feature_grey_inertia.h"
#include "feature_grey_major_axes.h"
#include "feature_grey_dimensions_cube.h"
#include "feature_grey_dimensions_ellipsoid.h"


namespace dip {


MeasurementTool::MeasurementTool() {
   // Size
   Register( dip::Feature::Pointer( new Feature::FeatureSize ));
   Register( dip::Feature::Pointer( new Feature::FeatureMinimum ));
   Register( dip::Feature::Pointer( new Feature::FeatureMaximum ));
   Register( dip::Feature::Pointer( new Feature::FeatureCartesianBox ));
   Register( dip::Feature::Pointer( new Feature::FeaturePerimeter ));
   Register( dip::Feature::Pointer( new Feature::FeatureSurfaceArea ));
   Register( dip::Feature::Pointer( new Feature::FeatureFeret ));
   Register( dip::Feature::Pointer( new Feature::FeatureConvexArea ));
   Register( dip::Feature::Pointer( new Feature::FeatureConvexPerimeter ));
   // Shape
   Register( dip::Feature::Pointer( new Feature::FeatureAspectRatioFeret ));
   Register( dip::Feature::Pointer( new Feature::FeatureRadius ));
   Register( dip::Feature::Pointer( new Feature::FeatureEllipseVariance ));
   Register( dip::Feature::Pointer( new Feature::FeatureP2A ));
   Register( dip::Feature::Pointer( new Feature::FeaturePodczeckShapes ));
   Register( dip::Feature::Pointer( new Feature::FeatureConvexity ));
   Register( dip::Feature::Pointer( new Feature::FeatureBendingEnergy ));
   // Intensity
   Register( dip::Feature::Pointer( new Feature::FeatureMass ));
   Register( dip::Feature::Pointer( new Feature::FeatureMean ));
   Register( dip::Feature::Pointer( new Feature::FeatureStandardDeviation ));
   Register( dip::Feature::Pointer( new Feature::FeatureStatistics ));
   Register( dip::Feature::Pointer( new Feature::FeatureMaxVal ));
   Register( dip::Feature::Pointer( new Feature::FeatureMinVal ));
   // Binary moments
   Register( dip::Feature::Pointer( new Feature::FeatureCenter ));
   Register( dip::Feature::Pointer( new Feature::FeatureMu ));
   Register( dip::Feature::Pointer( new Feature::FeatureInertia ));
   Register( dip::Feature::Pointer( new Feature::FeatureMajorAxes ));
   Register( dip::Feature::Pointer( new Feature::FeatureDimensionsCube ));
   Register( dip::Feature::Pointer( new Feature::FeatureDimensionsEllipsoid ));
   // Grey-value moments
   Register( dip::Feature::Pointer( new Feature::FeatureGravity ));
   Register( dip::Feature::Pointer( new Feature::FeatureGreyMu ));
   Register( dip::Feature::Pointer( new Feature::FeatureGreyInertia ));
   Register( dip::Feature::Pointer( new Feature::FeatureGreyMajorAxes ));
   Register( dip::Feature::Pointer( new Feature::FeatureGreyDimensionsCube ));
   Register( dip::Feature::Pointer( new Feature::FeatureGreyDimensionsEllipsoid ));
}

using LineBasedFeatureArray = std::vector< Feature::LineBased* >;
using FeatureArray = std::vector< Feature::Base* >;


// dip::Framework::ScanFilter function, not overloaded because the Feature::LineBased::ScanLine functions
// that we call here are not overloaded.
class dip__Measure : public Framework::ScanLineFilter {
   public:
      virtual void Filter( Framework::ScanLineFilterParameters const& params ) override {
         LineIterator< uint32 > label(
               static_cast< uint32* >( params.inBuffer[ 0 ].buffer ),
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
            feature->ScanLine( label, grey, params.position, params.dimension, objectIndices );
         }
      }
      dip__Measure( LineBasedFeatureArray const& features, ObjectIdToIndexMap const& objectIndices ) :
            features( features ), objectIndices( objectIndices ) {}
   private:
      LineBasedFeatureArray const& features;
      ObjectIdToIndexMap const& objectIndices;
};


Measurement MeasurementTool::Measure(
      Image const& label,
      Image const& grey,
      StringArray features, // copy
      UnsignedArray const& objectIDs,
      dip::uint connectivity
) const {

   // Check input
   DIP_THROW_IF( label.TensorElements() != 1, E::NOT_SCALAR );
   DIP_THROW_IF( !label.DataType().IsUnsigned(), E::DATA_TYPE_NOT_SUPPORTED );
   if( grey.IsForged() ) {
      DIP_THROW_IF( !grey.DataType().IsReal(), E::DATA_TYPE_NOT_SUPPORTED );
      DIP_START_STACK_TRACE
         grey.CompareProperties( label, Option::CmpProps_Sizes );
      DIP_END_STACK_TRACE
   }

   Measurement measurement;

   // Fill out the object IDs
   if( objectIDs.empty() ) {
      measurement.AddObjectIDs( GetObjectLabels( label, Image(), false ));
   } else {
      measurement.AddObjectIDs( objectIDs );
   }
   if( measurement.NumberOfObjects() == 0 ) {
      // There's no objects to be measured. We're done.
      // TODO: throw?
      return measurement;
   }

   // Parse the features array and prepare measurements
   DIP_THROW_IF( features.empty(), "No features given" );
   FeatureArray featureArray;
   featureArray.reserve( features.size() );
   dip::uint ii = 0;
   while( ii < features.size() ) {
      String const& name = features[ ii ];
      if( !measurement.FeatureExists( name ) ) {
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
      ++ii;
   }

   // Allocate memory for all features and objects
   measurement.Forge();

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
      DataTypeArray inBufT{ DT_UINT32 };
      if( grey.IsForged() ) {
         inar.emplace_back( grey );
         inBufT.emplace_back( DT_DFLOAT );
      }
      ImageRefArray outar{};
      DataTypeArray outBufT{};
      DataTypeArray outImT{};
      UnsignedArray nElem{};

      // Do the scan, which calls dip::Feature::LineBased::ScanLine()
      dip__Measure functor{ lineBasedFeatures, measurement.ObjectIndices() };
      Framework::Scan( inar, outar, inBufT, outBufT, outImT, nElem, &functor,
            Framework::Scan_NoMultiThreading + Framework::Scan_NeedCoordinates );

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
      ChainCodeArray chainCodeArray = GetImageChainCodes( label, measurement.Objects(), connectivity );
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
      Measurement::IteratorObject row = measurement.FirstObject(); // these two arrays are ordered the same way
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


std::ostream& operator<<(
      std::ostream& os,
      Measurement const& msr
) {
   // Figure out column widths
   const std::string sep{ " | " };
   constexpr int separatorWidth = 3;
   constexpr int minimumColumnWidth = 10; // we format numbers with at least this many spaces: '-' + 4 digits of precision + '.' + 'e+NN'
   const int firstColumnWidth = int( std::ceil( std::log10( msr.NumberOfObjects() + 1 ) ) );
   // TODO: find the actual largest object ID, rather than the number of objects.
   auto const& values = msr.Values();
   std::vector< int > valueWidths( values.size(), 0 );
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      std::stringstream ss;
      ss << values[ ii ].units;
      std::string units = ss.str();
      valueWidths[ ii ] = std::max( int( values[ ii ].name.size() ), int( units.size() ) + 2 ); // + 2 for the brackets we'll add later
      valueWidths[ ii ] = std::max( valueWidths[ ii ], minimumColumnWidth );
   }
   auto const& features = msr.Features();
   std::vector< int > featureWidths( values.size(), 0 );
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      featureWidths[ ii ] = valueWidths[ features[ ii ].startColumn ];
      for( dip::uint jj = 1; jj < features[ ii ].numberValues; ++jj ) {
         featureWidths[ ii ] += valueWidths[ features[ ii ].startColumn + jj ] + separatorWidth;
      }
      featureWidths[ ii ] = std::max( featureWidths[ ii ], int( features[ ii ].name.size() ));
   }
   // Write out the header: feature names
   std::cout << std::setw( firstColumnWidth ) << " " << " | ";
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      std::cout << std::setw( featureWidths[ ii ] ) << features[ ii ].name << " | ";
   }
   std::cout << std::endl;
   // Write out the header: horizontal line
   std::cout << std::setfill( '-' ) << std::setw( firstColumnWidth ) << "-" << " | ";
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      std::cout << std::setw( featureWidths[ ii ] ) << "-" << " | ";
   }
   std::cout << std::setfill( ' ' ) << std::endl;
   // Write out the header: value names
   std::cout << std::setw( firstColumnWidth ) << " " << " | ";
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      std::cout << std::setw( valueWidths[ ii ] ) << values[ ii ].name << " | ";
   }
   std::cout << std::endl;
   // Write out the header: value units
   std::cout << std::setw( firstColumnWidth ) << " " << " | ";
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      std::stringstream ss;
      ss << values[ ii ].units;
      std::string units = '(' + ss.str() + ')';
      std::cout << std::setw( valueWidths[ ii ] ) << units << " | ";
   }
   std::cout << std::endl;
   // Write out the header: horizontal line
   std::cout << std::setfill( '-' ) << std::setw( firstColumnWidth ) << "-" << " | ";
   for( dip::uint ii = 0; ii < values.size(); ++ii ) {
      std::cout << std::setw( valueWidths[ ii ] ) << "-" << " | ";
   }
   std::cout << std::setfill( ' ' ) << std::endl;
   // Write out the object IDs and associated values
   auto const& objects = msr.Objects();
   Measurement::ValueIterator data = msr.Data();
   for( dip::uint jj = 0; jj < objects.size(); ++jj ) {
      std::cout << std::setw( firstColumnWidth ) << objects[ jj ] << " | ";
      for( dip::uint ii = 0; ii < values.size(); ++ii ) {
         std::cout << std::setw( valueWidths[ ii ] ) << std::setprecision( 4 ) << std::showpoint << *data << " | ";
         ++data;
      }
      std::cout << std::endl;
   }
   return os;
}


} // namespace dip
