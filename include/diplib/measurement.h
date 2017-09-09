/*
 * DIPlib 3.0
 * This file contains declarations for measurement-related classes
 *
 * (c)2016-2017, Cris Luengo.
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

#include <map>

#include "diplib.h"


/// \file
/// \brief Declares the `dip::Measurement` and `dip::MeasurementTool` classes, and the `dip::Feature` namespace.
/// \see measurement


namespace dip {

// Forward declaration
template< typename T > class DIP_NO_EXPORT LineIterator;
struct DIP_NO_EXPORT ChainCode;
struct DIP_NO_EXPORT Polygon;
class DIP_NO_EXPORT ConvexHull;

/// \defgroup measurement Measurement
/// \brief The measurement infrastructure and functionality.
/// \{


/// \brief Contains classes that implement the measurement features.
/// \ingroup measurement
namespace Feature {

/// \brief The types of measurement features
enum class DIP_NO_EXPORT Type {
      LINE_BASED, ///< The feature is derived from `dip::Feature::LineBased`
      IMAGE_BASED, ///< The feature is derived from `dip::Feature::ImageBased`
      CHAINCODE_BASED, ///< The feature is derived from `dip::Feature::ChainCodeBased`
      POLYGON_BASED, ///< The feature is derived from `dip::Feature::PolygonBased`
      CONVEXHULL_BASED, ///< The feature is derived from `dip::Feature::ConvexHullBased`
      COMPOSITE ///< The feature is derived from `dip::Feature::Composite`
};

/// \brief %Information about a measurement feature
struct DIP_NO_EXPORT Information {
   String name;               ///< The name of the feature, used to identify it
   String description;        ///< A description of the feature, to be shown to the user
   bool needsGreyValue;       ///< Does the feature need a grey-value image?
   Information( String const& name, String const& description, bool needsGreyValue = false ) :
         name( name ), description( description ), needsGreyValue( needsGreyValue ) {}
   Information() : name( "" ), description( "" ), needsGreyValue( false ) {}
};

/// \brief %Information about the known measurement features
using InformationArray = std::vector< Information >;

/// \brief %Information about a measurement value, one of the components of a feature
struct DIP_NO_EXPORT ValueInformation {
   String name; ///< A short string that identifies the value
   Units units; ///< The units for the value
};

/// \brief %Information about the values of a measurement feature, or all values of all measurement features
/// in a `dip::Measurement` object.
using ValueInformationArray = std::vector< ValueInformation >;

} // namespace Feature


/// \brief Maps object IDs to object indices
using ObjectIdToIndexMap = std::map< dip::uint, dip::uint >;

/// \brief Contains measurement results, as obtained through `dip::MeasurementTool::Measure`.
///
/// \ingroup measurement
///
/// A newly constructed `%Measurement` will accept calls to `AddFeature`, and
/// `AddObjectIDs`. Once the object is set up with all objects and features needed, a call
/// to `Forge` creates the data segment necessary to hold all those measurements. Once
/// forged, it is no longer possible to add features or objects. As with a `dip::Image`,
/// the method `IsForged` can be used to test if the object has been forged.
///
/// A forged `%Measurement` can be read from in various ways, and a writable pointer to the
/// data can be obtained. As with the `dip::Image` class, data pointers are always writable,
/// even if the object is const-qualified. This simplifies the code, at the expense of opening
/// the door to undesirable modifications to data. *DIPlib* will never modify the data of a
/// const `%Measurement`.
///
/// The columns of the `%Measurement` table are the feature values. Since each feature can have multiple
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
///     dip::dfloat width = measurement[ "Feret" ][ 412 ][ 1 ];
///     dip::dfloat width = measurement[ 412 ][ "Feret" ][ 1 ];
/// ```
///
/// These three types of references are represented as iterators. Thus, it is also possible to iterate over
/// all columns groups (or all rows), iterate over each of the cell groups within a column group (or within a row),
/// and iterate over the values within a cell group:
///
/// ```cpp
///     auto colIt = measurement[ "Feret" ];
///     auto feretIt = colIt.FirstObject(); // iterator points at Feret value for first object
///     dip::dfloat sum = 0.0;
///     while( feretIt ) {
///        sum += feretIt[ 1 ]; // read width for current object
///        ++feretIt; // iterator points at Feret value for next object
///     }
///     dip::dfloat meanWidth = sum / measurement.NumberOfObjects();
/// ```
///
/// ```cpp
///     auto it = measurement[ "Feret" ][ 412 ];
///     std::cout << "Feret values for object ID = 412:";
///     for( auto f : it ) {
///        std::cout << " " << f;
///     }
///     std::cout << std::endl;
/// ```
class DIP_NO_EXPORT Measurement {
   public:
      using ValueType = dfloat;           ///< The type of the measurement data
      using ValueIterator = ValueType*;   ///< A pointer to measurement data, which we can treat as an iterator

      /// \brief Structure containing information about the features stored in a `dip::Measurement` object
      struct DIP_NO_EXPORT FeatureInformation {
         String name;            ///< Name of the feature
         dip::uint startColumn;  ///< Column for first value of feature
         dip::uint numberValues; ///< Number of vales in feature
         FeatureInformation( String const& name, dip::uint startColumn, dip::uint numberValues )
               : name( name ), startColumn( startColumn ), numberValues( numberValues ) {}
      };

      /// \brief An iterator to visit all features (column groups) in the `dip::Measurement` table. Can also
      /// be seen as a view over a specific feature.
      ///
      /// The iterator can be indexed with an object ID to access the table cell that contains the feature's
      /// values for that object. It is also possible to iterate over all objects. See `dip::Measurement` for
      /// examples of using this class.
      ///
      /// The `Subset` method selects a subset of the values of the current feature. This does not invalidate
      /// the iterator: incrementing it will select the next feature in the same way it would have if `Subset`
      /// hadn't been called. When indexing a subset feature using an object ID, the resulting table cell is
      /// the same subset of the cell, as one would expect. Thus, subsetting can be used to look at only one
      /// value of a feature as if that feature had produced only one value. For example:
      ///
      /// ```cpp
      ///     dip::Measurement msr = measureTool.Measure( label, grey, {"Feret"}, {} );
      ///     auto featureValues = msr[ "Feret" ];
      ///     featureValues.Subset( 1 ); // Select the "FeretMin" column only
      /// ```
      class DIP_NO_EXPORT IteratorFeature {
         public:
            friend class Measurement;
            /// \brief An iterator to visit all objects (rows) within a feature (column group) of the `dip::Measurement` table.
            ///
            /// An object of this class can be treated (in only the most basic ways) as a `std::array` or `std::vector`.
            class Iterator {
               public:
                  friend class IteratorFeature;
                  /// \brief Index to access a specific value
                  ValueType& operator[]( dip::uint index ) const { return *( begin() + index ); }
                  /// \brief Dereference to access the first value
                  ValueType& operator*() const { return *begin(); }
                  /// \brief Iterator to the first value
                  ValueIterator begin() const {
                     return feature_.measurement_.Data() +
                            static_cast< dip::sint >( index_ ) * feature_.measurement_.Stride() +
                            static_cast< dip::sint >( feature_.startColumn_ );
                  }
                  /// \brief Iterator one past the last value
                  ValueIterator end() const { return begin() + size(); }
                  /// \brief A pointer to the first value
                  ValueType* data() const { return begin(); }
                  /// \brief Number of values
                  dip::uint size() const { return feature_.numberValues_; }
                  /// \brief Increment, to access the next object
                  Iterator& operator++() { ++index_; return *this; }
                  /// \brief Increment, to access the next object
                  Iterator operator++( int ) { Iterator tmp( *this ); operator++(); return tmp; }
                  /// \brief True if done iterating (do not call other methods if this is true!)
                  bool IsAtEnd() const { return index_ >= feature_.NumberOfObjects(); }
                  /// \brief True if the iterator is valid and can be used
                  explicit operator bool() const { return !IsAtEnd(); }
                  /// \brief Name of the feature
                  String const& FeatureName() const { return feature_.FeatureName(); }
                  /// \brief ID of the object
                  dip::uint ObjectID() const { return feature_.measurement_.objects_[ index_ ]; }
                  /// \brief Index of the object (row number)
                  dip::uint ObjectIndex() const { return index_; }
               private:
                  Iterator( IteratorFeature const& feature, dip::uint index ) : feature_( feature ), index_( index ) {}
                  IteratorFeature const& feature_;
                  dip::uint index_;
            };
            /// \brief Iterator to the first object for this feature
            Iterator FirstObject() const { return Iterator( *this, 0 ); }
            /// \brief Iterator to the given object for this feature
            Iterator operator[]( dip::uint objectID ) const { return Iterator( *this, ObjectIndex( objectID )); }
            /// \brief Increment, to access the next feature
            IteratorFeature& operator++() {
               ++index_;
               startColumn_ = measurement_.features_[ index_ ].startColumn;
               numberValues_ = measurement_.features_[ index_ ].numberValues;
               return *this;
            }
            /// \brief Increment, to access the next feature
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
            bool IsAtEnd() const { return index_ >= NumberOfFeatures(); }
            /// \brief True if the iterator is valid and can be used
            explicit operator bool() const { return !IsAtEnd(); }
            /// \brief Name of the feature
            String const& FeatureName() const { return Feature().name; }
            /// \brief Number of values
            dip::uint NumberOfValues() const { return numberValues_; }
            /// \brief Number of objects
            dip::uint NumberOfObjects() const { return measurement_.NumberOfObjects(); }
            /// \brief Returns a list of object IDs
            UnsignedArray const& Objects() const { return measurement_.Objects(); }
            /// \brief Finds the index for the given object ID
            dip::uint ObjectIndex( dip::uint objectID ) const { return measurement_.ObjectIndex( objectID ); }
         private:
            IteratorFeature( Measurement const& measurement, dip::uint index ) : measurement_( measurement ), index_( index ) {
               startColumn_ = Feature().startColumn;
               numberValues_ = Feature().numberValues;
            }
            IteratorFeature( Measurement const& measurement, dip::uint startColumn, dip::uint numberValues ) :
                  measurement_( measurement ), index_( 0 ),
                  startColumn_( startColumn ), numberValues_( numberValues ) {}
            dip::uint NumberOfFeatures() const { return measurement_.NumberOfFeatures(); }
            FeatureInformation const& Feature() const { return measurement_.features_[ index_ ]; }
            Measurement const& measurement_;
            dip::uint index_;
            dip::uint startColumn_;  // A local copy of measurement_.features_[ index_ ].startColumn, so that it can be tweaked.
            dip::uint numberValues_; // A local copy of measurement_.features_[ index_ ].numberValues, so that it can be tweaked.
      };

      /// \brief An iterator to visit all objects (rows) in the `dip::Measurement` table. Can also be seen as a
      /// view over a specific object.
      ///
      /// The iterator can be indexed with an feature name to access the table cell group that contains the object's
      /// values for that feature. It is also possible to iterate over all features. See `dip::Measurement` for
      /// examples of using this class.
      class DIP_NO_EXPORT IteratorObject {
         public:
            friend class Measurement;
            /// \brief An iterator to visit all features (columns) within an object (row) of the `dip::Measurement` table.
            ///
            /// An object of this class can be treated (in only the most basic ways) as a `std::array` or `std::vector`.
            class Iterator {
               public:
                  friend class IteratorObject;
                  /// \brief Index to access a specific value
                  ValueType& operator[]( dip::uint index ) const { return *( begin() + index ); }
                  /// \brief Dereference to access the first value
                  ValueType& operator*() const { return *begin(); }
                  /// \brief Iterator to the first value
                  ValueIterator begin() const {
                     return object_.measurement_.Data() +
                            static_cast< dip::sint >( object_.index_ ) * object_.measurement_.Stride() +
                            static_cast< dip::sint >( Feature().startColumn );
                  }
                  /// \brief Iterator one past the last value
                  ValueIterator end() const { return begin() + size(); }
                  /// \brief A pointer to the first value
                  ValueType* data() const { return begin(); }
                  /// \brief Number of values
                  dip::uint size() const { return Feature().numberValues; }
                  /// \brief Increment, to access the next feature
                  Iterator& operator++() { ++index_; return *this; }
                  /// \brief Increment, to access the next feature
                  Iterator operator++( int ) { Iterator tmp( *this ); operator++(); return tmp; }
                  /// \brief True if done iterating (do not call other methods if this is true!)
                  bool IsAtEnd() const { return index_ >= object_.NumberOfFeatures(); }
                  /// \brief True if the iterator is valid and can be used
                  explicit operator bool() const { return !IsAtEnd(); }
                  /// \brief Name of the feature
                  String const& FeatureName() const { return Feature().name; }
                  /// \brief ID of the object
                  dip::uint ObjectID() const { return object_.ObjectID(); }
                  /// \brief Index of the object (row number)
                  dip::uint ObjectIndex() const { return object_.ObjectIndex(); }
               private:
                  Iterator( IteratorObject const& object, dip::uint index ) : object_( object ), index_( index ) {}
                  FeatureInformation const& Feature() const { return object_.measurement_.features_[ index_ ]; }
                  IteratorObject const& object_;
                  dip::uint index_;
            };
            /// \brief Iterator to the first feature for this object
            Iterator FirstFeature() const { return Iterator( *this, 0 ); }
            /// \brief Iterator to the given feature for this object
            Iterator operator[]( String const& name ) const { return Iterator( *this, FeatureIndex( name )); }
            /// \brief Increment, to access the next object
            IteratorObject& operator++() { ++index_; return *this; }
            /// \brief Increment, to access the next object
            IteratorObject operator++( int ) { IteratorObject tmp( *this ); operator++(); return tmp; }
            /// \brief True if done iterating (do not call other methods if this is true!)
            bool IsAtEnd() const { return index_ >= NumberOfObjects(); }
            /// \brief True if the iterator is valid and can be used
            explicit operator bool() const { return !IsAtEnd(); }
            /// \brief ID of the object
            dip::uint ObjectID() const { return measurement_.objects_[ index_ ]; }
            /// \brief Index of the object (row number)
            dip::uint ObjectIndex() const { return index_; }
            /// \brief Number of features
            dip::uint NumberOfFeatures() const { return measurement_.NumberOfFeatures(); }
            /// \brief Returns an array of feature names
            std::vector< FeatureInformation > const& Features() const { return measurement_.Features(); }
            /// \brief Returns the index to the first columns for the feature
            dip::uint ValueIndex( String const& name ) const { return measurement_.ValueIndex( name ); }
         private:
            IteratorObject( Measurement const& measurement, dip::uint index ) : measurement_( measurement ), index_( index ) {}
            dip::uint NumberOfObjects() const { return measurement_.NumberOfObjects(); }
            dip::uint FeatureIndex( String const& name ) const { return measurement_.FeatureIndex( name ); }
            Measurement const& measurement_;
            dip::uint index_;
      };

      /// \brief Adds a feature to a non-forged `Measurement` object.
      void AddFeature( String const& name, Feature::ValueInformationArray const& values ) {
         DIP_THROW_IF( IsForged(), "Measurement object is forged" );
         DIP_THROW_IF( name.empty(), "No feature name given" );
         DIP_THROW_IF( FeatureExists( name ), String( "Feature already present: " ) + name );
         DIP_THROW_IF( values.empty(), "A feature needs at least one value" );
         AddFeature_( name, values );
      }

      /// \brief Adds a feature to a non-forged `Measurement` object if it is not already there.
      void EnsureFeature( String const& name, Feature::ValueInformationArray const& values ) {
         DIP_THROW_IF( IsForged(), "Measurement object is forged" );
         DIP_THROW_IF( name.empty(), "No feature name given" );
         if( FeatureExists( name )) {
            return;
         }
         DIP_THROW_IF( values.empty(), "A feature needs at least one value" );
         AddFeature_( name, values );
      }

      /// \brief Adds object IDs to a non-forged `Measurement` object.
      void AddObjectIDs( UnsignedArray const& objectIDs ) {
         DIP_THROW_IF( IsForged(), "Measurement object is forged" );
         for( auto const& objectID : objectIDs ) {
            DIP_THROW_IF( ObjectExists( objectID ), String( "Object already present: " ) + std::to_string( objectID ));
            dip::uint index = objects_.size();
            objects_.push_back( objectID );
            objectIndices_.emplace( objectID, index );
         }
      }

      /// \brief Forges the table, allocating space to hold measurement values.
      void Forge() {
         if( !IsForged() ) {
            dip::uint n = values_.size() * objects_.size();
            DIP_THROW_IF( n == 0, "Attempting to forge a zero-sized table" );
            data_.resize( n );
         }
      }

      /// \brief Tests if the object is forged (has data segment allocated)
      bool IsForged() const { return !data_.empty(); }

      /// \brief Creates an iterator (view) to the first object
      IteratorObject FirstObject() const { return IteratorObject( *this, 0 ); }

      /// \brief Creates and iterator (view) to the given object
      IteratorObject operator[]( dip::uint objectID ) const { return IteratorObject( *this, ObjectIndex( objectID )); }

      /// \brief Creates and iterator (view) to the first feature
      IteratorFeature FirstFeature() const { return IteratorFeature( *this, 0 ); }

      /// \brief Creates and iterator (view) to the given feature
      IteratorFeature operator[]( String const& name ) const { return IteratorFeature( *this, FeatureIndex( name )); }

      /// \brief Creates and iterator (view) to a subset of feature values
      ///
      /// Example:
      ///
      /// ```cpp
      ///     dip::Measurement msr = measureTool.Measure( label, grey, {"Feret"}, {} );
      ///     auto featureValues = msr.FeatureValuesView( 1, 1 ); // Select the "FeretMin" column only
      /// ```
      IteratorFeature FeatureValuesView( dip::uint startValue, dip::uint numberValues = 1 ) const {
         DIP_THROW_IF( startValue + numberValues > NumberOfValues(), "Subset out of range" );
         return IteratorFeature( *this, startValue, numberValues );
      }

      /// \brief A raw pointer to the data of the table. All values for one object are contiguous.
      ValueType* Data() const {
         DIP_THROW_IF( !IsForged(), "Measurement object not forged" );
         return data_.data();
      }

      /// \brief The stride to use to access the next row of data in the table (next object).
      dip::sint Stride() const {
         return static_cast< dip::sint >( values_.size() );
      }

      /// \brief True if the feature is available in `this`.
      bool FeatureExists( String const& name ) const {
         return featureIndices_.count( name ) != 0;
      }

      /// \brief Finds the column index for the first value of the given feature.
      dip::uint FeatureIndex( String const& name ) const {
         auto it = featureIndices_.find( name );
         DIP_THROW_IF( it == featureIndices_.end(), String( "Feature not present: " ) + name );
         return it->second;
      }

      /// \brief Returns an array of feature names
      std::vector< FeatureInformation > const& Features() const {
         return features_;
      }

      /// \brief Returns the number of features
      dip::uint NumberOfFeatures() const {
         return features_.size();
      }

      /// \brief Finds the index into the `dip::Measurement::Values()` array for the first value of the given feature.
      dip::uint ValueIndex( String const& name ) const {
         return features_[ FeatureIndex( name ) ].startColumn;
      }

      /// \brief Returns an array with names and units for each of the values for the feature.
      /// (Note: data is copied to output array, not a trivial function).
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
         DIP_THROW_IF( it == objectIndices_.end(), String( "Object not present: " ) + std::to_string( objectID ));
         return it->second;
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

      // TODO: port dipio_MeasurementWriteCSV as a method to dip::Measurement

   private:

      void AddFeature_( String const& name, Feature::ValueInformationArray const& values ) {
         dip::uint startIndex = values_.size();
         values_.resize( startIndex + values.size() );
         for( dip::uint ii = 0; ii < values.size(); ++ii ) {
            values_[ startIndex + ii ] = values[ ii ];
         }
         dip::uint index = features_.size();
         features_.emplace_back( name, startIndex, values.size() );
         featureIndices_.emplace( name, index );
      }
      UnsignedArray objects_;                         // the rows of the table (maps row indices to objectIDs)
      ObjectIdToIndexMap objectIndices_;              // maps object IDs to row indices
      std::vector< FeatureInformation > features_;    // the column groups of the table (maps column group indices to feature names and contains other info also)
      Feature::ValueInformationArray values_;         // the columns of the table
      std::map< String, dip::uint > featureIndices_;  // maps feature names to column group indices
      std::vector< ValueType > mutable data_;         // this is mutable so that a const object doesn't have const data -- the only reason for this is to avoid making const versions of the iterators, which seems pointless
      // `data` has a row for each objectID, and a column for each feature value. The rows are stored contiguous.
      // `data[ features[ ii ].offset + jj * numberValues ]` gives the first value for feature `ii` for object with
      // index `jj`. `jj = objectIndices_[ id ]`. `ii = features_[ featureIndices_[ name ]].startColumn`.
};

//
// Overloaded operators
//

/// \brief You can output a `dip::Measurement` to `std::cout` or any other stream to produce a human-readable
/// representation of the tabular data in it.
DIP_EXPORT std::ostream& operator<<( std::ostream& os, Measurement const& msr );


namespace Feature {

/// \brief The pure virtual base class for all measurement features.
class DIP_EXPORT Base {
   public:
      Information const information; ///< Information on the feature
      Type const type; ///< The type of the measurement

      Base( Information const& information, Type const type ) : information( information ), type( type ) {};

      /// \brief A feature can have configurable parameters. Such a feature can define a `%Configure` method
      /// that the user can access through `dip::MeasurementTool::Configure`.
      virtual void Configure( String const& /*parameter*/, dfloat /*value*/ ) {
         DIP_THROW( "Feature not configurable" );
      };

      /// \brief All measurement features define an `%Initialize` method that prepares the feature class
      /// to perform measurements on the image. It also gives information on the feature as applied to that image.
      ///
      /// This function should check image properties and throw an exception if the measurement
      /// cannot be made. The `dip::MeasurementTool` will not catch this exception, please provide a
      /// meaningful error message for the user. `label` will always be a scalar, unsigned integer image, and
      /// `grey` will always be of a real type. But `grey` can be a tensor image, so do check for that. For
      /// chain-code--based and convex-hull--based measurements, the images will always have exactly two
      /// dimensions; for other measurement types, the images will have at least one dimension, check the
      /// image dimensionality if there are other constraints. `grey` will always have the same dimensionality
      /// and sizes as `label` if the measurement requires a grey-value image; it will be a raw image otherwise.
      ///
      /// %Information returned includes the number of output values it will generate per object, what
      /// their name and units will be, and how many intermediate values it will need to store (for
      /// line-based functions only).
      ///
      /// Note that this function can store information about the images in private data members of the
      /// class, so that it is available when performing measurements. For example, it can store the
      /// pixel size to inform the measurement.
      ///
      /// Note that this function is not expected to perform any major amount of work.
      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) = 0;

      /// \brief All measurement features define a `%Cleanup` method that is called after finishing the measurement
      /// process for one image.
      virtual void Cleanup() {};

      virtual ~Base() {};
};

/// \brief A pointer to a measurement feature of any type
using Pointer = std::unique_ptr< Base >;

/// \brief The pure virtual base class for all line-based measurement features.
class DIP_EXPORT LineBased : public Base {
   public:
      LineBased( Information const& information ) : Base( information, Type::LINE_BASED ) {};

      /// \brief Called once for each image line, to accumulate information about each object.
      /// This function is not called in parallel, and hence does not need to be thread-safe.
      ///
      /// The two line iterators can always be incremented exactly the same number of times.
      /// `coordinates[ dimension ]` should be incremented at the same time, if coordinate
      /// information is required by the algorithm. `label` is non-zero where there is an
      /// object pixel. Look up the `label` value in `objectIndices` to obtain the index for
      /// the object. Object indices are always between 0 and number of objects - 1. The
      /// `dip::Feature::Base::Initialize` function should allocate an array with `nObjects`
      /// elements, where measurements are accumulated. The `dip::Feature::LineBased::Finish`
      /// function is called after the whole image has been scanned, and should provide the
      /// final measurement result for one object given its index (not object ID).
      virtual void ScanLine(
            LineIterator< uint32 > label, ///< Pointer to the line in the labeled image (always scalar)
            LineIterator< dfloat > grey, ///< Pointer to the line in the grey-value image (if given, invalid otherwise)
            UnsignedArray coordinates, ///< Coordinates of the first pixel on the line (by copy, so it can be modified)
            dip::uint dimension, ///< Along which dimension the line runs
            ObjectIdToIndexMap const& objectIndices ///< A map from objectID (label) to index
      ) = 0;

      /// \brief Called once for each object, to finalize the measurement
      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all image-based measurement features.
class DIP_EXPORT ImageBased : public Base {
   public:
      ImageBased( Information const& information ) : Base( information, Type::IMAGE_BASED ) {};

      /// \brief Called once to compute measurements for all objects
      virtual void Measure( Image const& label, Image const& grey, Measurement::IteratorFeature& output ) = 0;
};

/// \brief The pure virtual base class for all chain-code--based measurement features.
class DIP_EXPORT ChainCodeBased : public Base {
   public:
      ChainCodeBased( Information const& information ) : Base( information, Type::CHAINCODE_BASED ) {};

      /// \brief Called once for each object
      virtual void Measure( ChainCode const& chainCode, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all polygon-based measurement features.
class DIP_EXPORT PolygonBased : public Base {
   public:
      PolygonBased( Information const& information ) : Base( information, Type::POLYGON_BASED ) {};

      /// \brief Called once for each object
      virtual void Measure( Polygon const& polygon, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all convex-hull--based measurement features.
class DIP_EXPORT ConvexHullBased : public Base {
   public:
      ConvexHullBased( Information const& information ) : Base( information, Type::CONVEXHULL_BASED ) {};

      /// \brief Called once for each object
      virtual void Measure( ConvexHull const& convexHull, Measurement::ValueIterator output ) = 0;
};

/// \brief The pure virtual base class for all composite measurement features.
class DIP_EXPORT Composite : public Base {
   public:
      Composite( Information const& information ) : Base( information, Type::COMPOSITE ) {};

      /// \brief Lists the features that the measurement depends on. These features will be computed and made
      /// available to the `Measure` method. This function is always called after `dip::Feature::Base::Initialize`.
      virtual StringArray Dependencies() = 0;

      /// \brief Called once for each object, the input `dependencies` object contains the measurements
      /// for the object from all the features in the `dip::Composite::Dependencies` list.
      virtual void Compose( Measurement::IteratorObject& dependencies, Measurement::ValueIterator output ) = 0;
};

} // namespace Feature


/// \brief Performs measurements on images.
///
/// \ingroup measurement
///
/// The %MeasurementTool class knows about defined measurement features, and can apply them to an
/// image through its `dip::MeasurementTool::Measure` method.
///
/// ```cpp
///     dip::MeasurementTool tool;
///     dip::Image img = ...
///     dip::Image label = Label( Threshold( img ), 2 );
///     dip::Measurement msr = tool.Measure( label, img, { "Size", "Perimeter" }, {}, 2 );
///     std::cout << "Size of object with label 1 is " << msr[ "Size" ][ 1 ][ 0 ] << std::endl;
/// ```
///
/// By default, the features in the following table are defined:
///
/// <table>
/// <tr><th> %Measurement name <th> Description <th> Limitations
/// <tr><td colspan="3"> **Size features**
/// <tr><td> "Size"                    <td> Number of object pixels <td>
/// <tr><td> "CartesianBox"            <td> Cartesian box size of the object in all dimensions <td>
/// <tr><td> "Minimum"                 <td> Minimum coordinates of the object <td>
/// <tr><td> "Maximum"                 <td> Maximum coordinates of the object <td>
/// <tr><td> "Perimeter"               <td> Length of the object perimeter <td> 2D (CC)
/// <tr><td> "SurfaceArea"             <td> Surface area of object <td> 3D
/// <tr><td> "Feret"                   <td> Maximum and minimum object diameters <td> 2D (CC)
/// <tr><td> "ConvexArea"              <td> Area of the convex hull <td> 2D (CC)
/// <tr><td> "ConvexPerimeter"         <td> Perimeter of the convex hull <td> 2D (CC)
/// <tr><td colspan="3"> **Shape features**
/// <tr><td> "AspectRatioFeret"        <td> Feret-based aspect ratio <td> 2D (CC)
/// <tr><td> "Radius"                  <td> Statistics on radius of object <td> 2D (CC)
/// <tr><td> "EllipseVariance"         <td> Ellipse variance <td> 2D (CC)
/// <tr><td> "P2A"                     <td> Circularity of the object <td> 2D (CC) & 3D
/// <tr><td> "PodczeckShapes"          <td> Podczeck shape descriptors <td> 2D (CC)
/// <tr><td> "Convexity"               <td> Area fraction of convex hull covered by object <td> 2D (CC)
/// <tr><td> "BendingEnergy"           <td> Bending energy of object perimeter <td> 2D (CC)
/// <tr><td colspan="3"> **Intensity features**
/// <tr><td> "Mass"                    <td> Mass of object (sum of object intensity) <td> Scalar grey
/// <tr><td> "Mean"                    <td> Mean object intensity <td> Scalar grey
/// <tr><td> "StandardDeviation"       <td> Standard deviation of object intensity <td> Scalar grey
/// <tr><td> "Statistics"              <td> Mean, standard deviation, skewness and excess kurtosis of object intensity <td> Scalar grey
/// <tr><td> "MaxVal"                  <td> Maximum object intensity <td> Scalar grey
/// <tr><td> "MinVal"                  <td> Minimum object intensity <td> Scalar grey
/// <tr><td colspan="3"> **Moments of binary object**
/// <tr><td> "Center"                  <td> Coordinates of the geometric mean of the object <td>
/// <tr><td> "Mu"                      <td> Elements of the inertia tensor <td>
/// <tr><td> "Inertia"                 <td> Moments of inertia of the binary object <td>
/// <tr><td> "MajorAxes"               <td> Principal axes of the binary object <td>
/// <tr><td> "DimensionsCube"          <td> Extent along the principal axes of a cube <td> 2D & 3D
/// <tr><td> "DimensionsEllipsoid"     <td> Extent along the principal axes of an ellipsoid <td> 2D & 3D
/// <tr><td colspan="3"> **Moments of grey-value object**
/// <tr><td> "Gravity"                 <td> Coordinates of the center of mass of the object <td> Scalar grey
/// <tr><td> "GreyMu"                  <td> Elements of the grey-weighted inertia tensor <td> scalar grey
/// <tr><td> "GreyInertia"             <td> Grey-weighted moments of inertia of the object <td> scalar grey
/// <tr><td> "GreyMajorAxes"           <td> Grey-weighted principal axes of the object <td> scalar grey
/// <tr><td> "GreyDimensionsCube"      <td> Extent along the principal axes of a cube (grey-weighted) <td> 2D & 3D, scalar grey
/// <tr><td> "GreyDimensionsEllipsoid" <td> Extent along the principal axes of an ellipsoid (grey-weighted) <td> 2D & 3D, scalar grey
/// </table>
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
/// into the `dip::MeasurementTool::Measure` method together with the label image.
///
/// See \ref features for more information on each of these features.
///
/// Note that you can define new measurement features, and register them with the `%MeasurementTool` through the
/// `dip::MeasurementTool::Register` method. The new feature then becomes available in the `dip::MeasurementTool::Measure`
/// method just like any of the default features.
class DIP_NO_EXPORT MeasurementTool {
   public:

      /// \brief Constructor.
      DIP_EXPORT MeasurementTool();

      /// \brief Registers a feature with this `%MeasurementTool`. The feature object becomes property of the tool.
      ///
      /// Create an instance of the feature class on the heap using `new`. The feature class must be
      /// derived from one of the five classes derived from `dip::Feature::Base` (thus not directly from `Base`).
      /// Note that the pointer returned by `new` must be explicitly converted to a `dip::Feature::Pointer`:
      ///
      /// ```cpp
      ///     class MyFeature : public dip::Feature::ChainCodeBased {
      ///        // define constructor and override virtual functions
      ///     }
      ///     MeasurementTool measurementTool;
      ///     measurementTool.Register( dip::Feature::Pointer( new MyFeature ));
      /// ```
      ///
      /// See the source files for exsiting features for examples (and a starting point) on how to write your
      /// own feature.
      void Register(
            Feature::Pointer feature
      ) {
         String const& name = feature->information.name;
         if( !Exists( name )) {
            dip::uint index = features_.size();
            features_.emplace_back( std::move( feature ));
            featureIndices_.emplace( name, index );
         } else {
            feature = nullptr; // deallocates the feature we received, we don't need it, we already have one with that name
         }
      }

      /// \brief Sets a parameter of a feature registered with this `%MeasurementTool`.
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
      ///
      /// `features` is an array with feature names. See the `dip::MeasurementTool::Features` method for
      /// information on how to obtain those names. Some features are composite features, they compute
      /// values based on other features. Thus, it is possible that the output `dip::Measurement` object
      /// contains features not directly requested, but needed to compute another feature.
      ///
      /// `objectIDs` is an array with the IDs of objects to measure, If any of the IDs is not a label
      /// in the `label` image, the resulting measures will be zero or otherwise marked as invalid. If
      /// an empty array is given, all objects in the labeled image are measured.
      ///
      /// `connectivity` should match the value used when creating the labeled image `label`.
      ///
      /// The output `dip::Measurement` structure contains measurements that take the pixel size of
      /// the `label` image into account. Those of `grey` are ignored. Some measurements require
      /// isotropic pixel sizes, if `label` is not isotropic, the pixel size is ignored and these
      /// measures will return values in pixels instead.
      DIP_EXPORT Measurement Measure(
            Image const& label,
            Image const& grey,
            StringArray features, // we take a copy of this array
            UnsignedArray const& objectIDs = {},
            dip::uint connectivity = 2
      ) const;

      /// \brief Returns a table with known feature names and descriptions, which can directly be shown to the user.
      /// (Note: data is copied to output array, not a trivial function).
      Feature::InformationArray Features() const {
         Feature::InformationArray out;
         for( auto const& feature : features_ ) {
            out.push_back( feature->information );
         }
         return out;
      }

   private:

      std::vector< Feature::Pointer > features_;
      std::map< String, dip::uint > featureIndices_;

      bool Exists( String const& name ) const {
         return featureIndices_.count( name ) != 0;
      }

      dip::uint Index( String const& name ) const {
         auto it = featureIndices_.find( name );
         DIP_THROW_IF( it == featureIndices_.end(), String( "Feature name not known: " ) + name );
         return it->second;
      }
};


/// \brief Paints each object with the selected measurement feature values.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object.
/// It is assumed that that measurement object was obtained through measurement of the input `label` image.
/// To obtain such a view, use the measurement's `[]` indexing with a feature name. Alternatively, use the
/// `dip::Measurement::FeatureValuesView` method to select an arbitrary subset of feature value columns.
/// The `dip::Measurement::IteratorFeature::Subset` method can be used for the same purpose.
///
/// If the selected feature has more than one value, then `out` will be a vector image with as many tensor elements
/// as values are in the feature.
///
/// `out` will be of type `DT_SFLOAT`. To change the data type, set the data type of `out` and set its protect
/// flag before calling this function:
///
/// ```cpp
///     dip::Image out;
///     out.SetDataType( dip::DT_UINT32 );
///     out.Protect();
///     ObjectToMeasurement( label, out, featureValues );
/// ```
DIP_EXPORT void ObjectToMeasurement(
      Image const& label,
      Image& out,
      Measurement::IteratorFeature const& featureValues
);
inline Image ObjectToMeasurement(
      Image const& label,
      Measurement::IteratorFeature const& featureValues
) {
   Image out;
   ObjectToMeasurement( label, out, featureValues );
   return out;
}


/// \brief Returns the smallest feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
DIP_EXPORT dfloat Minimum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the largest feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
DIP_EXPORT dfloat Maximum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the `percentile` feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
DIP_EXPORT dfloat Percentile( Measurement::IteratorFeature const& featureValues, dfloat percentile );

/// \brief Returns the median feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
inline dfloat Median( Measurement::IteratorFeature const& featureValues ) {
   return Percentile( featureValues, 50.0 );
}

/// \brief Returns the mean feature value in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
DIP_EXPORT dfloat Mean( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the maximum and minimum feature values in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
DIP_EXPORT MinMaxAccumulator MaximumAndMinimum( Measurement::IteratorFeature const& featureValues );

/// \brief Returns the first four central moments of the feature values in the first column of `featureValues`.
///
/// The input `featureValues` is a view over a specific feature in a `dip::Measurement` object. Only the
/// first value of the feature is used. For features with multiple values, select a value using the
/// `dip::Measurement::IteratorFeature::Subset` method, or pick a column in the `dip::Measurement` object
/// directly using `dip::Measurement::FeatureValuesView`.
DIP_EXPORT StatisticsAccumulator SampleStatistics( Measurement::IteratorFeature const& featureValues );

/// \}

} // namespace dip

#endif // DIP_MEASUREMENT_H
