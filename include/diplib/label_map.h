/*
 * (c)2024, Cris Luengo.
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

#ifndef DIP_LABEL_MAP_H
#define DIP_LABEL_MAP_H

#include <vector>

#include "diplib.h"
#include "diplib/union_find.h"
#include "diplib/private/robin_map.h"

/// \file
/// \brief Label maps and related functionality.
/// See \ref mapping.


namespace dip {

class DIP_NO_EXPORT Measurement; // Defined in "diplib/measurement.h"

/// \addtogroup regions

/// \brief Represents a set of labels (object IDs), and maps them to new ones.
///
/// The object contains a set of known labels. Each of these known labels will be associated
/// to a target label. If the target label is 0 (background), we refer to it as a zero mapping.
/// If the target is any other value, we refer to it as a non-zero mapping.
///
/// The function \ref Apply, when given a label image as input, will look up each pixel's value
/// in the set of known labels. If it is known, the corresponding pixel in the output image will
/// be given the target label as value. If it is not known, it is left unchanged by default.
/// After calling \ref DestroyUnknownLabels, the unknown labels will map to 0 instead.
class DIP_NO_EXPORT LabelMap {
   public:

      LabelMap() = default;
      // We relay on default copy and move constructor and assignment, it's all good.

      /// \brief Construct a map that maps `objectIDs` to themselves.
      template< typename UnsignedIntegerType, std::enable_if_t< std::numeric_limits< UnsignedIntegerType >::is_integer &&
                                                                !std::numeric_limits< UnsignedIntegerType >::is_signed, int > = 0 >
      explicit LabelMap( std::vector< UnsignedIntegerType > const& labels ) {
         map_.reserve( static_cast< dip::uint >( labels.size() ) * 2 );  // Most efficient when load factor is < 0.5
         for( UnsignedIntegerType lab : labels ) {
            LabelType l = CastLabelType( lab );
            map_.insert( { l, l } );
         }
      }

      /// \brief Construct a map that maps `objectIDs` to themselves.
      explicit LabelMap( UnsignedArray const& labels ) {
         map_.reserve( labels.size() * 2 );  // Most efficient when load factor is < 0.5
         for( dip::uint lab : labels ) {
            LabelType l = CastLabelType( lab );
            map_.insert( { l, l } );
         }
      }

      /// \brief Construct a map from the Union-Find data structure. Must call `labels.Relabel()`
      /// before converting to a `LabelMap`.
      template< typename IndexType_, typename ValueType_, typename UnionFunction_ >
      explicit LabelMap( UnionFind< IndexType_, ValueType_, UnionFunction_ > const& labels ) {
         LabelType nLabels = CastLabelType( labels.Size() );
         map_.reserve( static_cast< dip::uint >( nLabels ) * 2 );  // Most efficient when load factor is < 0.5
         for( LabelType lab = 1; lab < nLabels; ++lab ) {
            map_.insert( { lab, CastLabelType( labels.Label( lab )) } );
         }
      }

      /// \brief Construct a map that maps objectIDs 1 to `maxLabel` (inclusive) to themselves.
      explicit LabelMap( LabelType maxLabel ) {
         map_.reserve( static_cast< dip::uint >( maxLabel ) * 2 );  // Most efficient when load factor is < 0.5
         for( LabelType lab = 1; lab <= maxLabel; ++lab ) {
            map_.insert( { lab, lab } );
         }
      }

      /// \brief Causes the map, when applied, to map unknown labels to 0 (background).
      void DestroyUnknownLabels() {
         preserveUnknownLabels_ = false;
      }

      /// \brief Causes the map, when applied, to keep unknown labels unchanged. This is the default.
      void PreserveUnknownLabels() {
         preserveUnknownLabels_ = true;
      }

      /// \brief Returns the number of labels known (i.e. explicitly listed in the mapping). See also \ref Count.
      dip::uint Size() const {
         return map_.size();
      }

      /// \brief Applies the label map to a label image.
      ///
      /// `in` must be a label image (scalar, of an unsigned integer type). Out will be
      /// identical, but of type \ref DT_LABEL.
      DIP_EXPORT void Apply( Image const& in, Image& out ) const;
      DIP_NODISCARD Image Apply( Image const& in ) const {
         Image out;
         Apply( in, out );
         return out;
      }

      /// \brief Applies the label map to measurement data.
      ///
      /// Filters out objects (rows), and changes the object IDs for the remaining objects.
      ///
      /// Note that the mapping can map multiple objects to the same ID. In this case, the output
      /// measurement data will contain only the measurements for the last object that mapped to
      /// any given ID.
      DIP_NODISCARD DIP_EXPORT Measurement Apply( Measurement const& in ) const;

      /// \brief Combines `*this` and `other` using logical AND.
      ///
      /// The resulting map will contain the union of all the labels in the two maps.
      /// Non-zero mappings that exist in both of the maps will be kept in the output map.
      /// The target value of `*this` is used.
      /// The remainder will map to 0.
      DIP_EXPORT LabelMap& operator&=( LabelMap const& rhs );

      /// \brief Combines `*this` and `other` using logical OR.
      ///
      /// The resulting map will contain the union of all the labels in the two maps.
      /// Non-zero mappings that exist in either of the two maps will be kept in the output map.
      /// Those that map to a non-zero value in both maps will map to 0.
      /// If both label maps have a non-zero mapping for a given label, the one in `*this` is kept.
      DIP_EXPORT LabelMap& operator|=( LabelMap const& rhs );

      /// \brief Combines `*this` and `other` using logical XOR.
      ///
      /// The resulting map will contain the union of all the labels in the two maps.
      /// Non-zero mappings that exist in only one of the two maps will be kept in the output map.
      /// The remainder will map to 0.
      DIP_EXPORT LabelMap& operator^=( LabelMap const& rhs );

      /// \brief Modifies the map such that labels mapped to 0 instead map to themselves,
      /// and those mapped to any non-zero label instead map to 0.
      DIP_EXPORT void Negate();

      /// \brief Updates all target labels to be consecutive integers starting at 1. Zero mappings are not affected.
      DIP_EXPORT void Relabel();

      /// \brief Looks up a label in the map and returns the target label by reference.
      ///
      /// You can assign a new target label by updating the referenced label.
      ///
      /// If the label is not present, it will be added. The newly added label will map to itself (by default)
      /// or to 0 (if \ref DestroyUnknownLabels was called previously).
      LabelType& operator[]( LabelType label ) {
         MapType::iterator it{};
         bool success{};
         std::tie( it, success ) = map_.insert( { label, preserveUnknownLabels_ ? label : 0 } );
         // If `success`, then we just created a new entry. Either way, `it` points to `label`.
         return it.value();
      }

      /// \brief Looks up a label in the map and returns the target label. If the label is not present,
      /// instead returns the label (by default) or 0 (if \ref DestroyUnknownLabels was called previously).
      LabelType operator[]( LabelType label ) const {
         auto it = map_.find( label );
         if( it != map_.end() ) {
            return it.value();
         }
         return preserveUnknownLabels_ ? label : 0;
      }

      /// \brief Checks to see if `label` is known (i.e. explicitly listed with a mapping).
      bool Contains( LabelType label ) const {
         return map_.count( label ) > 0;
      }

      /// \brief Counts how many labels have a non-zero mapping (i.e. how many objects are selected). See also \ref Size.
      dip::uint Count() const {
         dip::uint count = 0;
         for( auto const& pair : map_ ) {
            if( pair.second != 0 ) {
               ++count;
            }
         }
         return count;
      }

   private:
      using MapType = tsl::robin_map< LabelType, LabelType >;
      MapType map_;
      bool preserveUnknownLabels_ = true;
};

/// \brief Combines two maps using logical AND. See \ref dip-LabelMap-operator%26%3D-LabelMap-CL.
/// \relates LabelMap
inline LabelMap operator&( LabelMap lhs, LabelMap const& rhs ) {
   lhs &=  rhs;
   return lhs;
}

/// \brief Combines two maps using logical OR. See \ref dip-LabelMap-operator%7C%3D-LabelMap-CL.
/// \relates LabelMap
inline LabelMap operator|( LabelMap lhs, LabelMap const& rhs ) {
   lhs |= rhs;
   return lhs;
}

/// \brief Combines two maps using logical XOR. See \ref dip-LabelMap-operator%5E%3D-LabelMap-CL.
/// \relates LabelMap
inline LabelMap operator^( LabelMap lhs, LabelMap const& rhs ) {
   lhs ^= rhs;
   return lhs;
}

/// \brief Applies logical NOT to the map. See \ref LabelMap::Negate.
/// \relates LabelMap
inline LabelMap operator~( LabelMap rhs ) {
   rhs.Negate();
   return rhs;
}


/// \endgroup

} // namespace dip

#endif // DIP_LABEL_MAP_H
