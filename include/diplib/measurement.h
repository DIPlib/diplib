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

#ifndef DIP_MEASUREMENT_H
#define DIP_MEASUREMENT_H

#include "diplib.h"
#include "diplib/private/robin_map.h"
#include "diplib/accumulators.h"


/// \file
/// \brief Functionality for quantification of objects.
/// See \ref measurement.


namespace dip {

// Forward declaration
template< typename T > class DIP_NO_EXPORT LineIterator;
struct DIP_NO_EXPORT ChainCode;
struct DIP_NO_EXPORT Polygon;
struct DIP_NO_EXPORT ConvexHull;

/// \group measurement Measurement
/// \brief The measurement infrastructure and functionality.
///
/// The \ref dip::MeasurementTool class provides the main interface to the
/// functionality in this module. Quantification results are encapsulated in
/// an object of the \ref dip::Measurement class.
/// \addtogroup

/// \brief Contains classes that implement the measurement features.
namespace Feature {

/// \brief The types of measurement features
enum class DIP_NO_EXPORT Type {
      LINE_BASED,       ///< The feature is derived from \ref dip::Feature::LineBased
      IMAGE_BASED,      ///< The feature is derived from \ref dip::Feature::ImageBased
      CHAINCODE_BASED,  ///< The feature is derived from \ref dip::Feature::ChainCodeBased
      POLYGON_BASED,    ///< The feature is derived from \ref dip::Feature::PolygonBased
      CONVEXHULL_BASED, ///< The feature is derived from \ref dip::Feature::ConvexHullBased
      COMPOSITE         ///< The feature is derived from \ref dip::Feature::Composite
};

/// \brief Information about a measurement feature
struct DIP_NO_EXPORT Information {
   String name;               ///< The name of the feature, used to identify it
   String description;        ///< A description of the feature, to be shown to the user
   bool needsGreyValue;       ///< Does the feature need a grey-value image?
   Information( String name, String description, bool needsGreyValue = false ) :
         name( std::move( name )), description( std::move( description )), needsGreyValue( needsGreyValue ) {}
   Information() : name( "" ), description( "" ), needsGreyValue( false ) {}
};

/// \brief Information about the known measurement features
/// \relates Information
using InformationArray = std::vector< Information >;

/// \brief Information about a measurement value, one of the components of a feature
struct DIP_NO_EXPORT ValueInformation {
   String name; ///< A short string that identifies the value
   Units units; ///< The units for the value
};

/// \brief Information about the values of a measurement feature, or all values of all measurement features
/// in a \ref dip::Measurement object.
/// \relates ValueInformation
using ValueInformationArray = std::vector< ValueInformation >;

} // namespace Feature


//
// Measurement class
//


/// \brief Maps object IDs to object indices
using ObjectIdToIndexMap = tsl::robin_map< dip::uint, dip::uint >;

/// \brief Contains measurement results, as obtained through \ref dip::MeasurementTool::Measure.
///
/// A newly constructed `Measurement` is raw, and will accept calls to \ref AddFeature, \ref SetObjectIDs,
/// \ref AddObjectID and \ref AddObjectIDs. Once the object is set up with all objects and features needed,
/// a call to `Forge` creates the data segment necessary to hold all those measurements. Once
/// forged, it is no longer possible to add features or objects. As with a \ref dip::Image,
/// the method \ref IsForged can be used to test if the object has been forged.
///
/// A forged `Measurement` can be read from in various ways, and a writable pointer to the
/// data can be obtained. As with the \ref dip::Image class, data pointers are always writable,
/// even if the object is const-qualified. This simplifies the code, at the expense of opening
/// the door to undesirable modifications to data. *DIPlib* will never modify the data of a
/// const `Measurement`.
///
/// The columns of the `Measurement` table are the feature values. Since each feature can have multiple
/// values, features represent column groups. The rows of the table are the objects.
///
/// Indexing with a feature name produces a reference to a column group. Indexing with an object ID
/// (an integer) produces a reference to a row. Each of these references can be indexed to
/// produce a reference to a table cell group. A cell group contains the values produced by one feature for one
/// object. The cell group can again be indexed to obtain each of the values. For example, the following two
/// lines are equivalent, and access the second value of the Feret feature (smallest Feret diameter) for
/// object ID 412:
///
/// ```cpp
/// dip::dfloat width = measurement[ "Feret" ][ 412 ][ 1 ];
/// dip::dfloat width = measurement[ 412 ][ "Feret" ][ 1 ];
/// ```
///
/// These three types of references are represented as iterators. Thus, it is also possible to iterate over
/// all columns groups (or all rows), iterate over each of the cell groups within a column group (or within a row),
/// and iterate over the values within a cell group:
///
/// ```cpp
/// auto colIt = measurement[ "Feret" ];
/// auto feretIt = colIt.FirstObject(); // iterator points at Feret value for first object
/// dip::dfloat sum = 0.0;
/// while( feretIt ) {
///    sum += feretIt[ 1 ]; // read width for current object
///    ++feretIt; // iterator points at Feret value for next object
/// }
/// dip::dfloat meanWidth = sum / measurement.NumberOfObjects();
/// ```
///
/// ```cpp
/// auto it = measurement[ "Feret" ][ 412 ];
/// std::cout << "Feret values for object ID = 412:";
/// for( auto f : it ) {
///    std::cout << ' ' << f;
/// }
/// std::cout << '\n';
/// ```
///
/// A `Measurement` with zero object IDs will never be forged, it is possible to call `Forge` on it, but
/// nothing will happen. For such an object, it is possible to index with a feature name, and iterate over
/// the features (column groups). However, each of the columns will be empty, such that \ref FirstObject
/// returns an invalid iterator (evaluates to `false`). This means that, given a `Measurement` obtained
/// from an empty image, one can iterate as usual over features and over objects, without needing to write
/// a special test for the case of an image without objects.
class DIP_NO_EXPORT Measurement {
   public:
      /// The type of the measurement data
      using ValueType = dfloat;
      /// A pointer to measurement data, which we can treat as an iterator
      using ValueIterator = ValueType*;

      /// \brief Structure containing information about the features stored in a \ref dip::Measurement object
      struct DIP_NO_EXPORT FeatureInformation {
         String name;            ///< Name of the feature
         dip::uint startColumn;  ///< Column for first value of feature
         dip::uint numberValues; ///< Number of vales in feature
         FeatureInformation( String name, dip::uint startColumn, dip::uint numberValues )
               : name( std::move( name )), startColumn( startColumn ), numberValues( numberValues ) {}
      };

      /// \brief An iterator to visit all features (column groups) in the \ref dip::Measurement table. Can also
      /// be seen as a view over a specific feature.
      ///
      /// The iterator can be indexed with an object ID to access the table cell that contains the feature's
      /// values for that object. It is also possible to iterate over all objects. See \ref dip::Measurement for
      /// examples of using this class.
      ///
      /// The `Subset` method selects a subset of the values of the current feature. This does not invalidate
      /// the iterator: incrementing it will select the next feature in the same way it would have if `Subset`
      /// hadn't been called. When indexing a subset feature using an object ID, the resulting table cell is
      /// the same subset of the cell, as one would expect. Thus, subsetting can be used to look at only one
      /// value of a feature as if that feature had produced only one value. For example:
      ///
      /// ```cpp
      /// dip::Measurement msr = measureTool.Measure( label, grey, {"Feret"}, {} );
      /// auto featureValues = msr[ "Feret" ];
      /// featureValues.Subset( 1 ); // Select the "FeretMin" column only
      /// ```
      class DIP_NO_EXPORT IteratorFeature {
         public:
            friend class Measurement;

            /// \brief An iterator to visit all objects (rows) within a feature (column group) of the \ref dip::Measurement table.
            ///
            /// An object of this class can be treated (in only the most basic ways) as a `std::array` or `std::vector`.
            class DIP_NO_EXPORT Iterator {
               public:
                  friend class IteratorFeature;

                  /// \brief Index to access a specific value
                  ValueType& operator[]( dip::uint index ) const { return *( begin() + index ); }
                  /// \brief Dereference to access the first value
                  ValueType& operator*() const { return *begin(); }
                  /// \brief Iterator to the first value
                  ValueIterator begin() const {
                     return measurement_->Data() +
                            static_cast< dip::sint >( objectIndex_ ) * measurement_->Stride() +
                            static_cast< dip::sint >( startColumn_ );
                  }
                  /// \brief Iterator one past the last value
                  ValueIterator end() const { return begin() + size(); }
                  /// \brief A pointer to the first value
                  ValueType* data() const { return begin(); }
                  /// \brief Number of values
                  dip::uint size() const { return numberValues_; }
                  /// \brief Pre-increment, to access the next object
                  Iterator& operator++() { ++objectIndex_; return *this; }
                  /// \brief Post-increment, to access the next object
                  Iterator operator++( int ) { Iterator tmp( *this ); operator++(); return tmp; }
                  /// \brief True if done iterating (do not call other methods if this is true!)
                  bool IsAtEnd() const { return objectIndex_ >= measurement_->NumberOfObjects(); }
                  /// \brief True if the iterator is valid and can be used
                  explicit operator bool() const { return !IsAtEnd(); }
                  /// \brief Name of the feature
                  String const& FeatureName() const { return Feature().name; }
                  /// \brief ID of the object
                  dip::uint ObjectID() const { return measurement_->objects_[ objectIndex_ ]; }
                  /// \brief Index of the object (row number)
                  dip::uint ObjectIndex() const { return objectIndex_; }

               private:
                  Iterator( IteratorFeature const& feature, dip::uint objectIndex )
                        : measurement_( feature.measurement_ ), featureIndex_( feature.featureIndex_ ),
                          objectIndex_( objectIndex ),
                          startColumn_( feature.startColumn_ ), numberValues_( feature.numberValues_ ) {}
                  FeatureInformation const& Feature() const { return measurement_->features_[ featureIndex_ ]; }
                  Measurement const* measurement_;
                  dip::uint featureIndex_;
                  dip::uint objectIndex_;
                  dip::uint startColumn_;
                  dip::uint numberValues_;
            };

            /// \brief Iterator to the first object for this feature
            Iterator FirstObject() const { return Iterator( *this, 0 ); }
            /// \brief Iterator to the given object for this feature
            Iterator operator[]( dip::uint objectID ) const { return Iterator( *this, ObjectIndex( objectID )); }
            /// \brief Pre-increment, to access the next feature
            IteratorFeature& operator++() {
               ++featureIndex_;
               if( IsAtEnd() ) {
                  startColumn_ += numberValues_;
                  numberValues_ = 0;
               } else {
                  startColumn_ = measurement_->features_[ featureIndex_ ].startColumn;
                  numberValues_ = measurement_->features_[ featureIndex_ ].numberValues;
               }
               return *this;
            }
            /// \brief Post-increment, to access the next feature
            IteratorFeature operator++( int ) { IteratorFeature tmp( *this ); operator++(); return tmp; }
            /// \brief Selects a subset of values from the current feature. This does not invalidate the iterator.
            IteratorFeature& Subset( dip::uint first, dip::uint number = 1 ) {
               DIP_THROW_IF( first >= Feature().numberValues, E::INDEX_OUT_OF_RANGE );
               DIP_THROW_IF( first+number > Feature().numberValues, E::INDEX_OUT_OF_RANGE );
               startColumn_ = Feature().startColumn + first;
               numberValues_ = number;
               return *this;
            }
            /// \brief True if done iterating (do not call other methods if this is true!)
            bool IsAtEnd() const { return featureIndex_ >= measurement_->NumberOfFeatures(); }
            /// \brief True if the iterator is valid and can be used
            explicit operator bool() const { return !IsAtEnd(); }
            /// \brief Name of the feature
            String const& FeatureName() const { return Feature().name; }
            /// \brief Returns an array with names and units for each of the values for the feature.
            /// (Note: data are copied to output array, this is not a trivial function).
            Feature::ValueInformationArray Values() const {
               Feature::ValueInformationArray values( numberValues_ );
               for( dip::uint ii = 0; ii < numberValues_; ++ii ) {
                  values[ ii ] = measurement_->values_[ ii + startColumn_ ];
               }
               return values;
            }
            /// \brief Number of values
            dip::uint NumberOfValues() const { return numberValues_; }
            /// \brief True if the object ID is available in `this`.
            bool ObjectExists( dip::uint objectID ) const { return measurement_->ObjectExists( objectID ); }
            /// \brief Finds the index for the given object ID
            dip::uint ObjectIndex( dip::uint objectID ) const { return measurement_->ObjectIndex( objectID ); }
            /// \brief Returns the map that links object IDs to row indices.
            ObjectIdToIndexMap const& ObjectIndices() const { return measurement_->ObjectIndices(); }
            /// \brief Returns a list of object IDs
            UnsignedArray const& Objects() const { return measurement_->Objects(); }
            /// \brief Number of objects
            dip::uint NumberOfObjects() const { return measurement_->NumberOfObjects(); }
            /// \brief A raw pointer to the data of the feature. All values for one object are contiguous.
            ValueType* Data() const { return measurement_->Data() + static_cast< dip::sint >( startColumn_ ); }
            /// \brief The stride to use to access the next row of data (next object).
            dip::sint Stride() const { return measurement_->Stride(); }

         private:
            IteratorFeature( Measurement const& measurement, dip::uint index ) : measurement_( &measurement ), featureIndex_( index ) {
               startColumn_ = Feature().startColumn;
               numberValues_ = Feature().numberValues;
            }
            IteratorFeature( Measurement const& measurement, dip::uint startColumn, dip::uint numberValues ) :
                  measurement_( &measurement ), featureIndex_( 0 ),
                  startColumn_( startColumn ), numberValues_( numberValues ) {}
            FeatureInformation const& Feature() const { return measurement_->features_[ featureIndex_ ]; }
            Measurement const* measurement_;
            dip::uint featureIndex_;
            dip::uint startColumn_;  // A local copy of measurement_->features_[ featureIndex_ ].startColumn, so that it can be tweaked.
            dip::uint numberValues_; // A local copy of measurement_->features_[ featureIndex_ ].numberValues, so that it can be tweaked.
      };

      /// \brief An iterator to visit all objects (rows) in the \ref dip::Measurement table. Can also be seen as a
      /// view over a specific object.
      ///
      /// The iterator can be indexed with an feature name to access the table cell group that contains the object's
      /// values for that feature. It is also possible to iterate over all features. See \ref dip::Measurement for
      /// examples of using this class.
      class DIP_NO_EXPORT IteratorObject {
         public:
            friend class Measurement;

            /// \brief An iterator to visit all features (columns) within an object (row) of the \ref dip::Measurement table.
            ///
            /// An object of this class can be treated (in only the most basic ways) as a `std::array` or `std::vector`.
            class DIP_NO_EXPORT Iterator {
               public:
                  friend class IteratorObject;

                  /// \brief Index to access a specific value
                  ValueType& operator[]( dip::uint index ) const { return *( begin() + index ); }
                  /// \brief Dereference to access the first value
                  ValueType& operator*() const { return *begin(); }
                  /// \brief Iterator to the first value
                  ValueIterator begin() const {
                     return measurement_->Data() +
                            static_cast< dip::sint >( objectIndex_ ) * measurement_->Stride() +
                            static_cast< dip::sint >( Feature().startColumn );
                  }
                  /// \brief Iterator one past the last value
                  ValueIterator end() const { return begin() + size(); }
                  /// \brief A pointer to the first value
                  ValueType* data() const { return begin(); }
                  /// \brief Number of values
                  dip::uint size() const { return Feature().numberValues; }
                  /// \brief Pre-increment, to access the next feature
                  Iterator& operator++() { ++featureIndex_; return *this; }
                  /// \brief Post-increment, to access the next feature
                  Iterator operator++( int ) { Iterator tmp( *this ); operator++(); return tmp; }
                  /// \brief True if done iterating (do not call other methods if this is true!)
                  bool IsAtEnd() const { return featureIndex_ >= measurement_->NumberOfFeatures(); }
                  /// \brief True if the iterator is valid and can be used
                  explicit operator bool() const { return !IsAtEnd(); }
                  /// \brief Name of the feature
                  String const& FeatureName() const { return Feature().name; }
                  /// \brief ID of the object
                  dip::uint ObjectID() const { return measurement_->objects_[ objectIndex_ ]; }
                  /// \brief Index of the object (row number)
                  dip::uint ObjectIndex() const { return objectIndex_; }

               private:
                  Iterator( IteratorObject const& object, dip::uint featureIndex )
                        : measurement_( object.measurement_ ), objectIndex_( object.objectIndex_ ),
                          featureIndex_( featureIndex ) {}
                  FeatureInformation const& Feature() const { return measurement_->features_[ featureIndex_ ]; }
                  Measurement const* measurement_;
                  dip::uint objectIndex_;
                  dip::uint featureIndex_;
            };

            /// \brief Iterator to the first feature for this object
            Iterator FirstFeature() const { return Iterator( *this, 0 ); }
            /// \brief Iterator to the given feature for this object
            Iterator operator[]( String const& name ) const { return Iterator( *this, FeatureIndex( name )); }
            /// \brief Pre-increment, to access the next object
            IteratorObject& operator++() { ++objectIndex_; return *this; }
            /// \brief Post-increment, to access the next object
            IteratorObject operator++( int ) { IteratorObject tmp( *this ); operator++(); return tmp; }
            /// \brief True if done iterating (do not call other methods if this is true!)
            bool IsAtEnd() const { return objectIndex_ >= NumberOfObjects(); }
            /// \brief True if the iterator is valid and can be used
            explicit operator bool() const { return !IsAtEnd(); }
            /// \brief ID of the object
            dip::uint ObjectID() const { return measurement_->objects_[ objectIndex_ ]; }
            /// \brief True if the feature is available in `this`.
            bool FeatureExists( String const& name ) const { return measurement_->FeatureExists( name ); }
            /// \brief Returns an array of feature names
            std::vector< FeatureInformation > const& Features() const { return measurement_->Features(); }
            /// \brief Number of features
            dip::uint NumberOfFeatures() const { return measurement_->NumberOfFeatures(); }
            /// \brief Returns the index to the first columns for the feature
            dip::uint ValueIndex( String const& name ) const { return measurement_->ValueIndex( name ); }
            /// \brief Returns an array with names and units for each of the values for the feature.
            /// (Note: data are copied to output array, this is not a trivial function).
            Feature::ValueInformationArray Values( String const& name ) const { return measurement_->Values( name ); }
            /// \brief Returns an array with names and units for each of the values (for all features)
            Feature::ValueInformationArray const& Values() const { return measurement_->Values(); }
            /// \brief Returns the total number of feature values
            dip::uint NumberOfValues() const { return measurement_->NumberOfValues(); }
            /// \brief Returns the number of values for the given feature
            dip::uint NumberOfValues( String const& name ) const { return measurement_->NumberOfValues( name ); }
            /// \brief Index of the object (row number)
            dip::uint ObjectIndex() const { return objectIndex_; }
            /// \brief A raw pointer to the data of the object. All values are contiguous.
            ValueType* Data() const { return measurement_->Data() + static_cast< dip::sint >( objectIndex_ ) * measurement_->Stride(); }

         private:
            IteratorObject( Measurement const& measurement, dip::uint index ) : measurement_( &measurement ), objectIndex_( index ) {}
            dip::uint NumberOfObjects() const { return measurement_->NumberOfObjects(); }
            dip::uint FeatureIndex( String const& name ) const { return measurement_->FeatureIndex( name ); }
            Measurement const* measurement_;
            dip::uint objectIndex_;
      };

      /// \brief Adds a feature to a raw `Measurement` object.
      void AddFeature( String const& name, Feature::ValueInformationArray const& values ) {
         DIP_THROW_IF( IsForged(), E::MEASUREMENT_NOT_RAW );
         DIP_THROW_IF( name.empty(), "No feature name given" );
         DIP_THROW_IF( FeatureExists( name ), "Feature already present: " + name );
         DIP_THROW_IF( values.empty(), "A feature needs at least one value" );
         AddFeature_( name, values.cbegin(), values.cend() );
      }

      /// \brief Adds a feature to a raw `Measurement` object if it is not already there.
      void EnsureFeature( String const& name, Feature::ValueInformationArray const& values ) {
         DIP_THROW_IF( IsForged(), E::MEASUREMENT_NOT_RAW );
         DIP_THROW_IF( name.empty(), "No feature name given" );
         if( !FeatureExists( name )) {
            DIP_THROW_IF( values.empty(), "A feature needs at least one value" );
            AddFeature_( name, values.cbegin(), values.cend() );
         }
      }

      /// \brief Replaces existing objectID array with a new one. The `Measurement` object must be raw.
      void SetObjectIDs( UnsignedArray objectIDs ) {
         DIP_THROW_IF( IsForged(), E::MEASUREMENT_NOT_RAW );
         dip::uint index = 0;
         objectIndices_.clear();
         objectIndices_.reserve( objectIDs.size() );
         for( auto const& objectID : objectIDs ) {
            DIP_THROW_IF( ObjectExists( objectID ), "Object already present: " + std::to_string( objectID ));
            objectIndices_.emplace( objectID, index++ );
         }
         objects_.swap( objectIDs );
      }

      /// \brief Adds an object ID to a raw `Measurement` object.
      /// It is not efficient to use this function in a loop.
      void AddObjectID( dip::uint objectID ) {
         DIP_THROW_IF( IsForged(), E::MEASUREMENT_NOT_RAW );
         DIP_THROW_IF( ObjectExists( objectID ), "Object already present: " + std::to_string( objectID ));
         dip::uint index = objects_.size();
         objectIndices_.emplace( objectID, index );
         objects_.push_back( objectID );
      }

      /// \brief Adds object IDs to a raw `Measurement` object.
      void AddObjectIDs( UnsignedArray const& objectIDs ) {
         DIP_THROW_IF( IsForged(), E::MEASUREMENT_NOT_RAW );
         dip::uint index = objects_.size();
         objectIndices_.reserve( objectIndices_.size() + objectIDs.size() );
         for( auto const& objectID : objectIDs ) {
            DIP_THROW_IF( ObjectExists( objectID ), "Object already present: " + std::to_string( objectID ));
            objectIndices_.emplace( objectID, index++ );
         }
         objects_.append( objectIDs );
      }

      /// \brief Forges the table, allocating space to hold measurement values.
      /// Will fail if there are no features defined.
      void Forge() {
         if( !IsForged() ) {
            dip::uint n = DataSize();
            DIP_THROW_IF( NumberOfFeatures() == 0, "Attempting to forge a table with zero features" );
            data_.resize( n );
         }
      }

      /// \brief Tests if the object is forged (has data segment allocated). A table with zero objects will
      /// always appear raw (non-forged) even if `Forge` was called.
      bool IsForged() const { return !data_.empty(); }

      /// \brief Creates an iterator (view) to the first object
      IteratorObject FirstObject() const {
         return { *this, 0 };
      }

      /// \brief Creates and iterator (view) to the given object. The table must be forged.
      IteratorObject operator[]( dip::uint objectID ) const {
         DIP_THROW_IF( !IsForged(), E::MEASUREMENT_NOT_FORGED );
         return { *this, ObjectIndex( objectID )};
      }

      /// \brief Creates and iterator (view) to the first feature
      IteratorFeature FirstFeature() const {
         return { *this, 0 };
      }

      /// \brief Creates and iterator (view) to the given feature
      IteratorFeature operator[]( String const& name ) const {
         return { *this, FeatureIndex( name )};
      }

      /// \brief Creates and iterator (view) to a subset of feature values
      ///
      /// Example:
      /// ```cpp
      /// dip::Measurement msr = measureTool.Measure( label, grey, {"Feret"}, {} );
      /// auto featureValues = msr.FeatureValuesView( 1, 1 ); // Select the "FeretMin" column only
      /// ```
      IteratorFeature FeatureValuesView( dip::uint startValue, dip::uint numberValues = 1 ) const {
         DIP_THROW_IF( startValue + numberValues > NumberOfValues(), "Subset out of range" );
         return { *this, startValue, numberValues };
      }

      /// \brief A raw pointer to the data of the table. All values for one object are contiguous.
      /// The table must be forged.
      ValueType* Data() const {
         DIP_THROW_IF( !IsForged(), E::MEASUREMENT_NOT_FORGED );
         return data_.data();
      }

      /// \brief The stride to use to access the next row of data in the table (next object).
      dip::sint Stride() const {
         return static_cast< dip::sint >( values_.size() );
      }

      /// \brief The total number of data values in the table, equal to the product of `NumberOfValues`
      /// and `NumberOfObjects`.
      dip::uint DataSize() const {
         return NumberOfValues() * NumberOfObjects();
      }

      /// \brief True if the feature is available in `this`.
      bool FeatureExists( String const& name ) const {
         return featureIndices_.count( name ) != 0;
      }

      /// \brief Finds the index into the \ref Features array for the given feature.
      dip::uint FeatureIndex( String const& name ) const {
         auto it = featureIndices_.find( name );
         DIP_THROW_IF( it == featureIndices_.end(), "Feature not present: " + name );
         return it.value();
      }

      /// \brief Returns an array of feature names
      std::vector< FeatureInformation > const& Features() const {
         return features_;
      }

      /// \brief Returns the number of features
      dip::uint NumberOfFeatures() const {
         return features_.size();
      }

      /// \brief Finds the index into the \ref Values array for the first value of the given feature.
      dip::uint ValueIndex( String const& name ) const {
         return features_[ FeatureIndex( name ) ].startColumn;
      }

      /// \brief Returns an array with names and units for each of the values for the feature.
      /// (Note: data are copied to output array, this is not a trivial function).
      Feature::ValueInformationArray Values( String const& name ) const {
         auto feature = features_[ FeatureIndex( name ) ];
         Feature::ValueInformationArray values( feature.numberValues );
         for( dip::uint ii = 0; ii < feature.numberValues; ++ii ) {
            values[ ii ] = values_[ ii + feature.startColumn ];
         }
         return values;
      }

      /// \brief Returns an array with names and units for each of the values (for all features)
      Feature::ValueInformationArray const& Values() const {
         return values_;
      }

      /// \brief Returns the total number of feature values
      dip::uint NumberOfValues() const {
         return values_.size();
      }

      /// \brief Returns the number of values for the given feature
      dip::uint NumberOfValues( String const& name ) const {
         dip::uint index = FeatureIndex( name );
         return features_[ index ].numberValues;
      }

      /// \brief True if the object ID is available in `this`.
      bool ObjectExists( dip::uint objectID ) const {
         return objectIndices_.count( objectID ) != 0;
      }

      /// \brief Finds the row index for the given object ID.
      dip::uint ObjectIndex( dip::uint objectID ) const {
         auto it = objectIndices_.find( objectID );
         DIP_THROW_IF( it == objectIndices_.end(), "Object not present: " + std::to_string( objectID ));
         return it.value();
      }

      /// \brief Returns the map that links object IDs to row indices.
      ObjectIdToIndexMap const& ObjectIndices() const {
         return objectIndices_;
      }

      /// \brief Returns a list of object IDs
      UnsignedArray const& Objects() const {
          return objects_;
      }

      /// \brief Returns the number of objects
      dip::uint NumberOfObjects() const {
         return objects_.size();
      }

      /// \brief The `+` operator merges two \ref dip::Measurement objects.
      ///
      /// The resulting object has, as feature set, the union of the two input feature sets, and as object IDs,
      /// the union of the two object ID lists. That is, the output might have more columns or more rows
      /// (i.e. more objects) than the two input objects, depending on the overlaps between the two. If both
      /// features and cells differ between the two, then cells with unknown data are filled with NaN. If both
      /// objects contain the same feature for the same measurement, and the value is NaN for one, the other value
      /// is picked. If both have a non-NaN value, the one of the `lhs` argument is picked (values are never
      /// actually added together!). This process insures that it is possible to add multiple sets of measurements
      /// (across different objects and different features) together, without worrying about the order that
      /// they are added together:
      ///
      /// ```cpp
      /// dip::Image label1 = ... // one image with objects 1-10
      /// dip::Image label2 = ... // one image with objects 11-20
      /// auto set1 = measurementTool.Measure( label1, {}, {'Size','Center'} );
      /// auto set2 = measurementTool.Measure( label2, {}, {'Size','Center'} );
      /// auto set3 = measurementTool.Measure( label1, {}, {'Feret','Radius'} );
      /// auto set4 = measurementTool.Measure( label2, {}, {'Feret','Radius'} );
      /// auto sum1 = set1 + set2; // Size and Center features for objects 1-20
      /// auto sum2 = set1 + set3; // Size, Center, Feret and Radius features for objects 1-10
      /// auto sumA = (set1 + set2) + (set3 + set4); // All features for all objects
      /// auto sumB = (set1 + set3) + (set2 + set4); // Idem
      /// auto sumC = set1 + set2 + set3 + set4;     // Idem
      /// auto sumD = set1 + set4 + set3 + set2;     // Idem
      /// ```
      DIP_EXPORT friend Measurement operator+( Measurement const& lhs, Measurement const& rhs );

   private:

      void AddFeature_(
            String const& name,
            Feature::ValueInformationArray::const_iterator valuesBegin,
            Feature::ValueInformationArray::const_iterator valuesEnd
      ) {
         dip::uint startIndex = values_.size();
         dip::uint n = static_cast< dip::uint >( std::distance( valuesBegin, valuesEnd ));
         values_.resize( startIndex + n );
         for( auto out = &values_[ startIndex ]; valuesBegin != valuesEnd; ++valuesBegin, ++out ) {
            *out = *valuesBegin;
         }
         dip::uint index = features_.size();
         features_.emplace_back( name, startIndex, n );
         featureIndices_.emplace( name, index );
      }

      UnsignedArray objects_;                         // the rows of the table (maps row indices to objectIDs)
      ObjectIdToIndexMap objectIndices_;              // maps object IDs to row indices
      std::vector< FeatureInformation > features_;    // the column groups of the table (maps column group indices to feature names and contains other info also)
      Feature::ValueInformationArray values_;         // the columns of the table
      tsl::robin_map< String, dip::uint > featureIndices_;  // maps feature names to column group indices
      std::vector< ValueType > mutable data_;         // this is mutable so that a const object doesn't have const data -- the only reason for this is to avoid making const versions of the iterators, which seems pointless
      // `data` has a row for each objectID, and a column for each feature value. The rows are stored contiguous.
      // `data[ features[ ii ].offset + jj * numberValues ]` gives the first value for feature `ii` for object with
      // index `jj`. `jj = objectIndices_[ id ]`. `ii = features_[ featureIndices_[ name ]].startColumn`.
};


//
// Overloaded operators
//


/// \brief You can output a \ref dip::Measurement to `std::cout` or any other stream to produce a human-readable
/// representation of the tabular data in it.
/// \relates dip::Measurement
DIP_EXPORT std::ostream& operator<<( std::ostream& os, Measurement const& measurement );


//
// Measurement feature framework
//


namespace Feature {

/// \brief The pure virtual base class for all measurement features.
class DIP_CLASS_EXPORT Base {
   public:
      Information const information; ///< Information on the feature
      Type const type; ///< The type of the measurement

      Base( Information information, Type const type ) : information( std::move( information )), type( type ) {}

      /// \brief A feature can have configurable parameters. Such a feature can define a `Configure` method
      /// that the user can access through \ref dip::MeasurementTool::Configure.
      virtual void Configure( String const& parameter, dfloat value ) {
         ( void ) value;
         DIP_THROW_INVALID_FLAG( parameter );
      }

      /// \brief All measurement features define an `Initialize` method that prepares the feature class
      /// to perform measurements on the image. It also gives information on the feature as applied to that image.
      ///
      /// This function should check image properties and throw an exception if the measurement
      /// cannot be made. The \ref dip::MeasurementTool will not catch this exception, please provide a
      /// meaningful error message for the user. `label` will always be a scalar, unsigned integer image, and
      /// `grey` will always be of a real type. But `grey` can be a tensor image, so do check for that. For
      /// chain-code--based and convex-hull--based measurements, the images will always have exactly two
      /// dimensions; for other measurement types, the images will have at least one dimension, check the
      /// image dimensionality if there are other constraints. `grey` will always have the same dimensionality
      /// and sizes as `label` if the measurement requires a grey-value image; it will be a raw image otherwise.
      ///
      /// Information returned includes the number of output values it will generate per object, what
      /// their name and units will be, and how many intermediate values it will need to store (for
      /// line-based functions only).
      ///
      /// Note that this function can store information about the images in private data members of the
      /// class, so that it is available when performing measurements. For example, it can store the
      /// pixel size.
      ///
      /// !!! attention
      ///     This function is not expected to perform any major amount of work.
      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) = 0;

      /// \brief All measurement features define a `Cleanup` method that is called after finishing the measurement
      /// process for one image.
      virtual void Cleanup() {}

      // Ensure the destructor is virtual
      virtual ~Base() = default;

      // Silence IDE warning about there being a destructor but not these other standard functions:
      Base(const Base& other) = delete;
      Base(Base&& other) = delete;
      Base& operator=(const Base& other) = delete;
      Base& operator=(Base&& other) = delete;
};

/// \brief The pure virtual base class for all line-based measurement features.
class DIP_CLASS_EXPORT LineBased : public Base {
   public:
      explicit LineBased( Information const& information ) : Base( information, Type::LINE_BASED ) {}

      /// \brief Called once for each image line, to accumulate information about each object.
      /// This function is not called in parallel, and hence does not need to be thread-safe.
      ///
      /// The two line iterators can always be incremented exactly the same number of times.
      /// `label` is non-zero where there is an object pixel.
      /// Look up the `label` value in `objectIndices` to obtain the index for
      /// the object. Object indices are always between 0 and number of objects - 1. The
      /// \ref dip::Feature::Base::Initialize function should allocate an array with `nObjects`
      /// elements, where measurements are accumulated. The \ref dip::Feature::LineBased::Finish
      /// function is called after the whole image has been scanned, and should provide the
      /// final measurement result for one object given its index (not object ID).
      ///
      /// `coordinates` contains the coordinates of the first pixel on the line, and is passed by copy,
      /// so it can be modified. `dimension` indicates along which dimension to run. Increment
      /// `coordinates[ dimension ]` at the same time as the line iterators if coordinate information
      /// is required by the algorithm.
      virtual void ScanLine(
            LineIterator< LabelType > label,
            LineIterator< dfloat > grey,
            UnsignedArray coordinates,
            dip::uint dimension,
            ObjectIdToIndexMap const& objectIndices
      ) = 0;

      /// \brief Called once for each object, to finalize the measurement.
      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all image-based measurement features.
class DIP_CLASS_EXPORT ImageBased : public Base {
   public:
      explicit ImageBased( Information const& information ) : Base( information, Type::IMAGE_BASED ) {}

      /// \brief Called once to compute measurements for all objects.
      virtual void Measure( Image const& label, Image const& grey, Measurement::IteratorFeature& output ) = 0;
};

/// \brief The pure virtual base class for all chain-code--based measurement features.
class DIP_CLASS_EXPORT ChainCodeBased : public Base {
   public:
      explicit ChainCodeBased( Information const& information ) : Base( information, Type::CHAINCODE_BASED ) {}

      /// \brief Called once for each object.
      virtual void Measure( ChainCode const& chainCode, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all polygon-based measurement features.
class DIP_CLASS_EXPORT PolygonBased : public Base {
   public:
      explicit PolygonBased( Information const& information ) : Base( information, Type::POLYGON_BASED ) {}

      /// \brief Called once for each object.
      virtual void Measure( Polygon const& polygon, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all convex-hull--based measurement features.
class DIP_CLASS_EXPORT ConvexHullBased : public Base {
   public:
      explicit ConvexHullBased( Information const& information ) : Base( information, Type::CONVEXHULL_BASED ) {}

      /// \brief Called once for each object.
      virtual void Measure( ConvexHull const& convexHull, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all composite measurement features.
class DIP_CLASS_EXPORT Composite : public Base {
   public:
      explicit Composite( Information const& information ) : Base( information, Type::COMPOSITE ) {}

      /// \brief Lists the features that the measurement depends on. These features will be computed and made
      /// available to the `Measure` method. This function is always called after \ref dip::Feature::Base::Initialize.
      ///
      /// !!! attention
      ///     Dependency chains are currently not supported. Dependencies listed here should not be
      ///     other `Type::COMPOSITE` features. This would require processing the composite features in the
      ///     right order for all dependencies to be present when needed.
      // TODO: Compute composite features in the right order according to a dependency tree.
      virtual StringArray Dependencies() = 0;

      /// \brief Called once for each object, the input `dependencies` object contains the measurements
      /// for the object from all the features in the \ref Dependencies() list.
      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) = 0;
};

} // namespace Feature


//
// MeasurementTool class
//


/// \brief Performs measurements on images.
///
/// The MeasurementTool class knows about defined measurement features, and can apply them to an
/// image through its \ref dip::MeasurementTool::Measure method.
///
/// ```cpp
/// dip::MeasurementTool tool;
/// dip::Image img = ...
/// dip::Image label = Label( Threshold( img ), 2 );
/// dip::Measurement msr = tool.Measure( label, img, { "Size", "Perimeter" }, {}, 2 );
/// std::cout << "Size of object with label 1 is " << msr[ "Size" ][ 1 ][ 0 ] << '\n';
/// ```
///
/// By default, the features in the following table are defined:
///
/// Measurement name            | Description       | Limitations
/// --------------------------- | ----------------- | -----------
///                             | **Size features**{ .m-text .m-success } |
/// `"Size"`                    | Number of object pixels |
/// `"SolidArea"`               | Area of object with any holes filled | 2D (CC)
/// `"Perimeter"`               | Length of the object perimeter | 2D (CC)
/// `"SurfaceArea"`             | Surface area of object | 3D
/// `"Minimum"`                 | Minimum coordinates of the object |
/// `"Maximum"`                 | Maximum coordinates of the object |
/// `"CartesianBox"`            | Cartesian box size of the object in all dimensions |
/// `"Feret"`                   | Maximum and minimum object diameters | 2D (CC)
/// `"Radius"`                  | Statistics on radius of object | 2D (CC)
/// `"ConvexArea"`              | Area of the convex hull | 2D (CC)
/// `"ConvexPerimeter"`         | Perimeter of the convex hull | 2D (CC)
///                             | **Shape features**{ .m-text .m-success } |
/// `"AspectRatioFeret"`        | Feret-based aspect ratio | 2D (CC)
/// `"P2A"`                     | Perimeter to area ratio of the object | 2D (CC) & 3D
/// `"Roundness"`               | Roundness of the object | 2D (CC)
/// `"Circularity"`             | Circularity of the object | 2D (CC)
/// `"PodczeckShapes"`          | Podczeck shape descriptors | 2D (CC)
/// `"Solidity"`                | Area fraction of convex hull covered by object | 2D (CC)
/// `"Convexity"`               | Ratio of perimeter of convex hull to perimeter of object | 2D (CC)
/// `"EllipseVariance"`         | Distance to best fit ellipse | 2D (CC)
/// `"Eccentricity"`            | Aspect ratio of best fit ellipse | 2D (CC)
/// `"BendingEnergy"`           | Bending energy of object perimeter | 2D (CC)
///                             | **Intensity features**{ .m-text .m-success } |
///  `"Mass"`                   | Mass of object (sum of object intensity) | Tensor grey
///  `"Mean"`                   | Mean object intensity | Tensor grey
///  `"StandardDeviation"`      | Standard deviation of object intensity | Tensor grey
///  `"Statistics"`             | Mean, standard deviation, skewness and excess kurtosis of object intensity | Scalar grey
///  `"DirectionalStatistics"`  | Directional mean and standard deviation of object intensity | Scalar grey
///  `"MaxVal"`                 | Maximum object intensity | Tensor grey
///  `"MinVal"`                 | Minimum object intensity | Tensor grey
///  `"MaxPos"`                 | Position of pixel with maximum intensity | Scalar grey
///  `"MinPos"`                 | Position of pixel with minimum intensity | Scalar grey
///                             | **Moments of binary object**{ .m-text .m-success } |
/// `"Center"`                  | Coordinates of the geometric mean of the object |
/// `"Mu"`                      | Elements of the inertia tensor |
/// `"Inertia"`                 | Moments of inertia of the binary object |
/// `"MajorAxes"`               | Principal axes of the binary object |
/// `"DimensionsCube"`          | Extent along the principal axes of a cube | 2D & 3D
/// `"DimensionsEllipsoid"`     | Extent along the principal axes of an ellipsoid | 2D & 3D
///                             | **Moments of grey-value object**{ .m-text .m-success } |
/// `"Gravity"`                 | Coordinates of the center of mass of the object | Scalar grey
/// `"GreyMu"`                  | Elements of the grey-weighted inertia tensor | Scalar grey
/// `"GreyInertia"`             | Grey-weighted moments of inertia of the object | Scalar grey
/// `"GreyMajorAxes"`           | Grey-weighted principal axes of the object | Scalar grey
/// `"GreyDimensionsCube"`      | Extent along the principal axes of a cube (grey-weighted) | 2D & 3D, scalar grey
/// `"GreyDimensionsEllipsoid"` | Extent along the principal axes of an ellipsoid (grey-weighted) | 2D & 3D, scalar grey
///
/// Note that some features are derived from others, and will cause the features they depend on to be included in the
/// output measurement object.
///
/// Some features are specific for 2D, and include "(CC)" in the limitations column above. "CC" stands for chain code.
/// These features are computed based on the chain code of the object, and only work correctly for connected objects.
/// That is, the object must be a single connected component. In case of the perimeter, only the external perimeter
/// is measured, the boundaries of holes in the object are ignored.
///
/// Features that include "Scalar grey" in the limitations column require a scalar grey-value image to be passed
/// into the \ref dip::MeasurementTool::Measure method together with the label image. "Tensor grey" indicates that
/// this grey-value image can be multi-valued (i.e. a tensor image); each tensor element will be reported as a
/// channel.
///
/// See \ref features for more information on each of these features.
///
/// It is possible for the user to define new measurement features, and register them with the `MeasurementTool` through the
/// \ref dip::MeasurementTool::Register method. The new feature then becomes available in the \ref dip::MeasurementTool::Measure
/// method just like any of the default features.
class DIP_NO_EXPORT MeasurementTool {
      using FeatureBasePointer = std::unique_ptr< Feature::Base >;  // A pointer to a measurement feature of any type

   public:

      /// \brief Constructor.
      DIP_EXPORT MeasurementTool();

      /// \brief Registers a feature with this `MeasurementTool`.
      ///
      /// Create an instance of the feature class on the heap using `new`. The feature class must be
      /// derived from one of the five classes derived from \ref "dip::Feature::Base" (thus not directly from `Base`).
      /// The \ref dip::MeasurementTool object takes ownership of the feature object:
      ///
      /// ```cpp
      /// class MyFeature : public dip::Feature::ChainCodeBased {
      ///    // define constructor and override virtual functions
      /// }
      /// MeasurementTool measurementTool;
      /// measurementTool.Register( new MyFeature );
      /// ```
      ///
      /// See the source files for existing features for examples (and a starting point) on how to write your
      /// own feature.
      void Register(
            Feature::Base* feature
      ) {
         auto smartpointer = FeatureBasePointer( feature );
         String const& name = feature->information.name;
         if( !Exists( name )) {
            dip::uint index = features_.size();
            features_.emplace_back( std::move( smartpointer ));
            featureIndices_.emplace( name, index );
         } else {
            smartpointer = nullptr; // deallocates the feature we received, we don't need it, we already have one with that name
         }
      }

      /// \brief Sets a parameter of a feature registered with this `MeasurementTool`.
      void Configure(
            String const& feature,
            String const& parameter,
            dfloat value
      ) const {
         features_[ Index( feature ) ]->Configure( parameter, value );
      }

      /// \brief Measures one or more features on one or more objects in the labeled image.
      ///
      /// `label` is a labeled image (any unsigned integer type, and scalar), and `grey` is either a raw
      /// image (not forged, without pixel data), or an real-valued image with the same dimensionality and
      /// sizes as `label`. If any selected features require a grey-value image, then it must be provided.
      /// Note that some features can handle multi-valued (tensor) images, and some can not.
      ///
      /// `features` is an array with feature names. See the \ref dip::MeasurementTool::Features method for
      /// information on how to obtain those names. Some features are composite features, they compute
      /// values based on other features. Thus, it is possible that the output \ref dip::Measurement object
      /// contains features not directly requested, but needed to compute another feature.
      ///
      /// `objectIDs` is an array with the IDs of objects to measure, If any of the IDs is not a label
      /// in the `label` image, the resulting measures will be zero or otherwise marked as invalid. If
      /// an empty array is given, all objects in the labeled image are measured. If there are no objects
      /// to be measured, a raw \ref dip::Measurement object is returned.
      ///
      /// `connectivity` should match the value used when creating the labeled image `label`.
      ///
      /// The output \ref dip::Measurement structure contains measurements that take the pixel size of
      /// the `label` image into account. Those of `grey` are ignored. Some measurements require
      /// isotropic pixel sizes, if `label` is not isotropic, the pixel size is ignored and these
      /// measures will return values in pixels instead.
      DIP_EXPORT Measurement Measure(
            Image const& label,
            Image const& grey,
            StringArray features, // we take a copy of this array
            UnsignedArray const& objectIDs = {},
            dip::uint connectivity = 0
      ) const;

      /// \brief Returns a table with known feature names and descriptions, which can directly be shown to the user.
      /// (Note: data is copied to output array, this is not a trivial function).
      Feature::InformationArray Features() const {
         Feature::InformationArray out;
         for( auto const& feature : features_ ) {
            out.push_back( feature->information );
         }
         return out;
      }

   private:

      std::vector< FeatureBasePointer > features_;
      tsl::robin_map< String, dip::uint > featureIndices_;

      bool Exists( String const& name ) const {
         return featureIndices_.count( name ) != 0;
      }

      dip::uint Index( String const& name ) const {
         auto it = featureIndices_.find( name );
         DIP_THROW_IF( it == featureIndices_.end(), "Feature name not known: " + name );
         return it.value();
      }
};


//
// Support functions
//


/// \brief Paints each object with the selected measurement feature values.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object.
/// It is assumed that that measurement object was obtained through measurement of the input `label` image.
/// To obtain such a view, use the measurement's `[]` indexing with a feature name. Alternatively, use the
/// \ref dip::Measurement::FeatureValuesView method to select an arbitrary subset of feature value columns.
/// The \ref dip::Measurement::IteratorFeature::Subset method can be used for the same purpose.
///
/// If the selected feature has more than one value, then `out` will be a vector image with as many tensor elements
/// as values are in the feature.
///
/// `out` will be of type `DT_SFLOAT`. To change the data type, set the data type of `out` and set its protect
/// flag before calling this function:
///
/// ```cpp
/// dip::Image out;
/// out.SetDataType( dip::DT_UINT32 );
/// out.Protect();
/// ObjectToMeasurement( label, out, featureValues );
/// ```
DIP_EXPORT void ObjectToMeasurement(
      Image const& label,
      Image& out,
      Measurement::IteratorFeature const& featureValues
);
DIP_NODISCARD inline Image ObjectToMeasurement(
      Image const& label,
      Measurement::IteratorFeature const& featureValues
) {
   Image out;
   ObjectToMeasurement( label, out, featureValues );
   return out;
}


/// \brief Writes a \ref dip::Measurement structure to a CSV file.
///
/// The CSV (comma separated values) file is a generic container for tabular data, and can be
/// read in just about any graphing and statistics software package.
///
/// The file written contains three header rows, followed by one row per object. The three
/// header rows contain the feature names, the value names, and the value units. The feature
/// names, of which there typically are fewer than columns, are interspersed with empty cells
/// to line them up with the first column for the feature. For example:
///
/// ```text
/// ObjectID, Size,  Center, ,      Feret, ,      ,        ,
/// ,         ,      dim0,   dim1,  Max,   Min,   PerpMin, MaxAng, MinAng
/// ,         um^2,  um,     um,    um,    um,    um,      rad,    rad
/// 1,        397.0, 20.06,  12.98, 34.99, 16.43, 34.83,   2.111,  3.588
/// 2,        171.0, 63.13,  4.123, 20.22, 11.00, 20.00,   2.993,  4.712
/// 3,        628.0, 108.4,  12.47, 32.20, 26.00, 28.00,   2.202,  0.000
/// 4,        412.0, 154.5,  9.561, 26.40, 22.00, 23.00,   2.222,  4.712
/// ```
///
/// !!! attention
///     The file will not have columns aligned with spaces as shown here, each
///     comma is always followed by a single space.
///
/// `options` is one or more of the following values:
///
/// - `"unicode"`: The units will be written using unicode strings. By default, only ASCII
///   characters are used.
/// - `"simple"`: There will only be a single header line, combining the three strings as
///   follows: `"Feature value (units)"`. For example: `"Size (um^2)"`, `"Feret Max (um)"`, etc.
DIP_EXPORT void MeasurementWriteCSV(
      Measurement const& measurement,
      String const& filename,
      StringSet const& options = {}
);


/// \brief Returns the smallest feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
DIP_EXPORT Measurement::ValueType Minimum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the largest feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
DIP_EXPORT Measurement::ValueType Maximum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the `percentile` feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
DIP_EXPORT Measurement::ValueType Percentile( Measurement::IteratorFeature const& featureValues, dfloat percentile );

/// \brief Returns the median feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
inline dfloat Median( Measurement::IteratorFeature const& featureValues ) {
   return Percentile( featureValues, 50.0 );
}

/// \brief Returns the mean feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
DIP_EXPORT dfloat Mean( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the maximum and minimum feature values in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
DIP_EXPORT MinMaxAccumulator MaximumAndMinimum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the first four central moments of the feature values in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a \ref dip::Measurement object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// \ref dip::Measurement::IteratorFeature::Subset method, or pick a column in the `dip::Measurement` object
/// directly using \ref dip::Measurement::FeatureValuesView.
DIP_EXPORT StatisticsAccumulator SampleStatistics( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the object ID with the smallest feature value in the first column of `featureValues`.
///
/// Same as \ref Minimum(Measurement::IteratorFeature const&), but returns the Object ID instead of
/// the feature value.
DIP_EXPORT dip::uint ObjectMinimum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the object ID with the largest feature value in the first column of `featureValues`.
///
/// Same as \ref Maximum(Measurement::IteratorFeature const&), but returns the Object ID instead of
/// the feature value.
DIP_EXPORT dip::uint ObjectMaximum( Measurement::IteratorFeature const& featureValues );

// TODO: ObjectPercentile, ObjectMedian.


/// \endgroup

} // namespace dip

#endif // DIP_MEASUREMENT_H
