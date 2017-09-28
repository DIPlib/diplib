/*
 * DIPlib 3.0
 * This file contains the declaration for a union-find data structure
 *
 * (c)2017, Cris Luengo.
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

#ifndef DIP_UNION_FIND_H
#define DIP_UNION_FIND_H

#include "diplib/library/types.h"


/// \file
/// \brief A data structure for the union-find algorithm.
/// \see infrastructure


namespace dip {


/// \addtogroup infrastructure
/// \{


/// \brief An STL-like data structure for the union-find algorithm.
///
/// Operations such as the watershed, connected component labeling, and the area opening, as implemented in
/// *DIPlib*, use the union-find algorithm. It provides an efficient method to set equivalences in labels.
/// That is, one can re-assign one label to be equivalent to another in O(1) time. Typically, one pass through
/// the image assigns a label to each pixel (`Create`), and determines which labels should be equivalent (`Union`);
/// a second pass changes the label for each pixel to that of the representative label for the corresponding set of
/// equivalent labels (`FindRoot`).
///
/// To stream-line the second pass, we provide here a `Relabel` method that assigns a unique, consecutive
/// label to each of the correspondence sets. **Note:** After this function has been called, the union-find data
/// structure is destroyed, and the only valid method that can still be called is `Label`.
///
/// Each tree element has a value associated to it. This must be a type that is copy-constructible and
/// default-initializable. Ideally, it's small. The value associated to any tree element that is not a root is
/// ignored. The `unionFunction` that the constructor takes is used to compute the value associated to the merged
/// tree when two trees are merged. It should have the following signature:
/// ```cpp
///     ValueType_ unionFunction( ValueType_ const& value1, ValueType_ const& value2 );
/// ```
///
/// The `IndexType_` template parameter should be an integer, and probably unsigned.
///
/// See the code to any of the algorithms that use this class for an example usage.
template< typename IndexType_, typename ValueType_, typename UnionFunction_ >
class UnionFind {
   public:
      using IndexType = IndexType_; ///< The type of the index (or label) that identifies each tree element
      using ValueType = ValueType_; ///< The type of the additional data stored for each tree element

      /// \brief Default constructor, creates an empty structure
      explicit UnionFind( UnionFunction_ const& unionFunction ) : unionFunction_( unionFunction ) {
         list.reserve( 1000 );
         list.push_back( ListElement{ 0, {} } ); // This element will not be used.
      }

      /// \brief Alternate constructor, creates `n` trees initialized to `value`.
      UnionFind( dip::uint n, ValueType value, UnionFunction_ const& unionFunction  ) : unionFunction_( unionFunction ) {
         list.resize( n + 1, { 0, value } );
         for( IndexType ii = 1; ii <= static_cast< IndexType >( n ); ++ii ) {
            list[ ii ].parent = ii;
         }
      }

      /// \brief Returns the index (label) for the root of the tree that contains `index`.
      IndexType FindRoot( IndexType index ) const {
         if( list[ index ].parent == index ) {
            return index;
         } else {
            // Note we update the parent node here to point directly to the root.
            return list[ index ].parent = FindRoot( list[ index ].parent );
         }
      }

      /// \brief Creates a new element, and places it in its own tree.
      IndexType Create( ValueType const& value ) {
         if( list.size() > std::numeric_limits< IndexType >::max() ) {
            DIP_THROW( "Cannot create more regions!" );
         }
         IndexType index = static_cast< IndexType >( list.size() );
         list.push_back( ListElement{ index, value } );
         return index;
      }

      /// \brief Merges two trees. Returns the index of the new root.
      IndexType Union( IndexType index1, IndexType index2 ) {
         index1 = FindRoot( index1 );
         index2 = FindRoot( index2 );
         if( index1 == index2 ) {
            return index1;
         } else {
            // We take the lower of the two labels as root.
            IndexType root = std::min( index1, index2 );
            IndexType leaf = std::max( index1, index2 );
            list[ root ].value = unionFunction_( list[ root ].value, list[ leaf ].value );
            list[ leaf ].parent = root;
            return root;
         }
      }

      /// \brief Returns a reference to the value associated to the tree that contains `index`.
      ValueType& Value( IndexType index ) { return list[ FindRoot( index ) ].value; }

      /// \brief Returns a reference to the value associated to the tree that contains `index`.
      ValueType const& Value( IndexType index ) const { return list[ FindRoot( index ) ].value; }

      /// \brief Assigns a new label to each of the trees, destroying the tree structure. Returns the number of
      /// unique labels. After this call, only use `Label`!
      dip::uint Relabel() {
         std::vector< IndexType > newLabels( list.size(), 0 );
         IndexType lab = 0;
         IndexType maxLab = static_cast< IndexType >( list.size() );
         // Assign a new, unique and consecutive label to each tree.
         for( IndexType ii = 1; ii < maxLab; ++ii ) {
            IndexType index = FindRoot( ii );
            if(( index > 0 ) && ( newLabels[ index ] == 0 )) {
               newLabels[ index ] = ++lab;
            }
         }
         // Write the new labels to the list.
         for( IndexType ii = 1; ii < maxLab; ++ii ) {
            // Note that we've called FindRoot on each list element, so they all directly point at their root now.
            list[ ii ].parent = newLabels[ list[ ii ].parent ];
         }
         return lab;
      }

      /// \brief Assigns a new label to the trees that satisfy `constraint`, and 0 to the remainder.
      ///
      /// This function destroys the tree structure. After this call, only use `Label`!
      ///
      /// `constraint` is a function or function object that takes the `ValueType` associated to a tree,
      /// and returns `true` if the tree is to be kept.
      ///
      /// Returns the number of unique labels.
      template< typename Constraint >
      dip::uint Relabel( Constraint constraint ) {
         std::vector< IndexType > newLabels( list.size(), 0 );
         IndexType lab = 0;
         IndexType maxLab = static_cast< IndexType >( list.size() );
         // Assign a new, unique and consecutive label to each tree.
         for( IndexType ii = 1; ii < maxLab; ++ii ) {
            IndexType index = FindRoot( ii );
            if(( index > 0 ) && ( newLabels[ index ] == 0 ) && constraint( list[ index ].value )) {
               newLabels[ index ] = ++lab;
            }
         }
         // Write the new labels to the list.
         for( IndexType ii = 1; ii < maxLab; ++ii ) {
            // Note that we've called FindRoot on each list element, so they all directly point at their root now.
            list[ ii ].parent = newLabels[ list[ ii ].parent ];
         }
         return lab;
      }

      /// \brief Returns the new label associated to the tree that contains `index`. Only useful after calling `Relabel`.
      IndexType Label( IndexType index ) const {
         return list[ index ].parent;
      }

   private:
      // The union-find algorithm stores a set of trees in an array (`list`). Each element of the array is part
      // of a tree. They have a `parent` element that contains the index to their parent element. Following parent
      // links leads to the root of the tree, which points at itself (it's its own parent). The union of two trees
      // simply implies making one of the roots a child of the other.
      // Parent pointers are mutable. Every time we traverse a tree looking for the root, we modify each parent
      // pointer to point directly to the root. This keeps the trees maximally flat, meaning that looking for the
      // parent can be done in constant time.
      struct ListElement {
         IndexType mutable parent; // Index to the parent in the tree. The root of a tree points to self.
         ValueType value;
      };
      std::vector< ListElement > list;
      UnionFunction_ const& unionFunction_;
};


/// \}

} // namespace dip

#endif // DIP_UNION_FIND_H
