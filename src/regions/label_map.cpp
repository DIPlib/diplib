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

#include "diplib/label_map.h"

#include <vector>

#include "diplib.h"
#include "diplib/framework.h"
#include "diplib/measurement.h"
#include "diplib/overload.h"
#include "diplib/private/robin_map.h"
#include "diplib/private/robin_set.h"

namespace dip {

namespace {

template< typename TPI >
class LabelMapApplyLineFilter: public Framework::ScanLineFilter {
   public:
      LabelMapApplyLineFilter( LabelMap const& labelMap ) : labelMap_( labelMap ) {}
      dip::uint GetNumberOfOperations( dip::uint nInput, dip::uint /**/, dip::uint /**/ ) override {
         return nInput * 10;  // TODO: this is a wild guess as to the cost...
      }
      void Filter( Framework::ScanLineFilterParameters const& params ) override {
         TPI const* in = static_cast< TPI const* >( params.inBuffer[ 0 ].buffer );
         LabelType* out = static_cast< LabelType* >( params.outBuffer[ 0 ].buffer );
         dip::sint inStride = params.inBuffer[ 0 ].stride;
         dip::sint outStride = params.outBuffer[ 0 ].stride;
         dip::uint bufferLength = params.bufferLength;
         LabelType inLabel = 0;       // last label seen, initialize to background label
         LabelType outLabel = 0;      // new label assigned to prevID
         for( dip::uint ii = 0; ii < bufferLength; ++ii ) {
            if( *in == 0 ) {
               // The background label is processed differently
               *out = 0;
            } else {
               LabelType lab = clamp_cast< LabelType >( *in ); // If TPI is uint64, we clamp, otherwise it's a plain cast.
               if( lab != inLabel ) {
                  inLabel = lab;
                  outLabel = labelMap_[ inLabel ];
               }
               *out = outLabel;
            }
            in += inStride;
            out += outStride;
         }
      }
   private:
      LabelMap const& labelMap_;
};

} // namespace

void LabelMap::Apply( Image const& in, Image& out ) const {
   // This function is very similar to dip::Relabel, though from the outside it looks more like dip::LookupTable::Apply.
   DIP_THROW_IF( !in.IsForged(), E::IMAGE_NOT_FORGED );
   DIP_THROW_IF( !in.IsScalar(), E::IMAGE_NOT_SCALAR );
   DIP_THROW_IF( !in.DataType().IsUInt(), E::DATA_TYPE_NOT_SUPPORTED );
   std::unique_ptr< Framework::ScanLineFilter >scanLineFilter;
   DIP_OVL_NEW_UINT( scanLineFilter, LabelMapApplyLineFilter, ( *this ), in.DataType() );
   ImageRefArray outar{ out };
   Framework::Scan( { in }, outar, { in.DataType() }, { DT_LABEL }, { DT_LABEL }, { 1 }, *scanLineFilter );
}

Measurement LabelMap::Apply( Measurement const& in ) const {
   DIP_THROW_IF( !in.IsForged(), E::MEASUREMENT_NOT_FORGED );
   // Code here to create a new measurement object is similar to that in dip::MeasurementTool::Measure().
   Measurement out;
   // Fill out the object IDs
   tsl::robin_set< dip::uint > ids; // Using a set to avoid any duplicate IDs
   ids.reserve( in.NumberOfObjects() * 2 );
   for( auto id : in.Objects() ) {
      auto out_id = ( *this )[ CastLabelType( id ) ];
      if( out_id != 0 ) {
         ids.insert(  out_id );
      }
   }
   UnsignedArray objects( ids.size() );
   std::copy( ids.begin(), ids.end(), objects.begin() );
   std::sort( objects.begin(), objects.end() ); // not important, but nice
   out.SetObjectIDs( objects );
   // Copy over all the feature information
   std::vector< Measurement::FeatureInformation > const& features = in.Features();
   for( dip::uint ii = 0; ii < features.size(); ++ii ) {
      out.AddFeature( features[ ii ].name, in.Values( features[ ii ].name ) );
   }
   // Allocate memory for all features and objects
   out.Forge();
   if( out.NumberOfObjects() == 0 ) {
      return out;
   }
   // Copy over the data
   dip::uint nValues = out.NumberOfValues();
   auto in_it = in.FirstObject();
   do {
      auto out_id = ( *this )[ CastLabelType( in_it.ObjectID() ) ];
      if( out_id != 0 ) {
         // If multiple objects map to the same ID, this overwrites previous data, only the last object mapped to this ID remains.
         std::copy_n( in_it.Data(), nValues, out[ out_id ].Data() );
      }
   } while( ++in_it );
   return out;
}

LabelMap& LabelMap::operator&=( LabelMap const& rhs ) {
   // Add all labels from `rhs` here, mapping to 0
   for( auto const& pair : rhs.map_ ) {
      map_.insert( { pair.first, 0 } ); // Does nothing if the label is already present
   }
   // Iterate over all labels, updating map_
   for( auto it = map_.begin(); it != map_.end(); ++it ) {
      auto rhs_it = rhs.map_.find( it->first );
      LabelType rhs_target = rhs_it == rhs.map_.end() ? 0 : rhs_it.value();
      if( rhs_target == 0 ) {
         it.value() = 0;
      }
   }
   return *this;
}

LabelMap& LabelMap::operator|=( LabelMap const& rhs ) {
   // Add all labels from `rhs` here, mapping to 0
   for( auto const& pair : rhs.map_ ) {
      map_.insert( { pair.first, 0 } ); // Does nothing if the label is already present
   }
   // Iterate over all labels, updating map_
   for( auto it = map_.begin(); it != map_.end(); ++it ) {
      if( it.value() == 0 ) {
         auto rhs_it = rhs.map_.find( it->first );
         LabelType rhs_target = rhs_it == rhs.map_.end() ? 0 : rhs_it.value();
         it.value() = rhs_target;
      }
   }
   return *this;
}

LabelMap& LabelMap::operator^=( LabelMap const& rhs ) {
   // Add all labels from `rhs` here, mapping to 0
   for( auto const& pair : rhs.map_ ) {
      map_.insert( { pair.first, 0 } );
   }
   // Iterate over all labels, updating map_
   for( auto it = map_.begin(); it != map_.end(); ++it ) {
      auto rhs_it = rhs.map_.find( it->first );
      LabelType lhs_target = it.value();
      LabelType rhs_target = rhs_it == rhs.map_.end() ? 0 : rhs_it.value();
      if( rhs_target != 0 ) {
         if( lhs_target == 0 ) {
            it.value() = rhs_target;
         } else {
            it.value() = 0;
         }
      }
   }
   return *this;
}

void LabelMap::Negate() {
   for( auto it = map_.begin(); it != map_.end(); ++it ) {
      if( it.value() == 0 ) {
         it.value() = it->first;
      } else {
         it.value() = 0;
      }
   }
}

void LabelMap::Relabel() {
   // Put all non-zero target labels in a sorted set
   std::set< LabelType > targets;
   for( auto const& pair : map_ ) {
      if( pair.second != 0 ) {
         targets.insert( pair.second );
      }
   }
   // Create a lookup table for these target values, mapping to the new target values
   tsl::robin_map< LabelType, LabelType > new_targets;
   new_targets.reserve( targets.size() * 2 );
   LabelType v = 1;
   for( LabelType t : targets ) {
      new_targets[ t ] = v;
      ++v;
   }
   // Update map_
   for( auto it = map_.begin(); it != map_.end(); ++it ) {
      if( it.value() != 0 ) {
         it.value() = new_targets[ it.value() ];
      }
   }
}

} // namespace dip

#ifdef DIP_CONFIG_ENABLE_DOCTEST
#include "doctest.h"
#include "diplib/regions.h"
#include "diplib/testing.h"

DOCTEST_TEST_CASE("[DIPlib] testing dip::LabelMap logical operations") {
   // This also tests constructors
   dip::LabelMap labels( 6 );
   dip::LabelMap moreLabels( std::vector< dip::LabelType >{ 3, 4, 5, 6, 7, 8 } );
   labels[ 2 ] = 0;
   labels[ 4 ] = 0;
   labels[ 6 ] = 0;
   moreLabels[ 3 ] = 0;
   moreLabels[ 6 ] = 0;
   moreLabels[ 8 ] = 0;
   // labels =     [ 1 0 3 0 5 0 ]
   // moreLabels = [     0 4 5 0 7 0 ]
   // AND =        [ 0 0 0 0 5 0 0 0 ]
   // OR =         [ 1 0 3 4 5 0 7 0 ]
   // XOR =        [ 1 0 3 4 0 0 7 0 ]
   // NOT =        [ 0 2 0 4 0 6 ]
   DOCTEST_CHECK( labels.Size() == 6 );
   DOCTEST_CHECK( labels.Count() == 3 );
   DOCTEST_CHECK( moreLabels.Size() == 6 );
   DOCTEST_CHECK( moreLabels.Count() == 3 );

   auto AND = labels & moreLabels;
   DOCTEST_CHECK( AND[ 1 ] == 0 );
   DOCTEST_CHECK( AND[ 2 ] == 0 );
   DOCTEST_CHECK( AND[ 3 ] == 0 );
   DOCTEST_CHECK( AND[ 4 ] == 0 );
   DOCTEST_CHECK( AND[ 5 ] == 5 );
   DOCTEST_CHECK( AND[ 6 ] == 0 );
   DOCTEST_CHECK( AND[ 7 ] == 0 );
   DOCTEST_CHECK( AND[ 8 ] == 0 );
   DOCTEST_CHECK( AND.Size() == 8 );
   DOCTEST_CHECK( AND.Count() == 1 );

   auto OR = labels | moreLabels;
   DOCTEST_CHECK( OR[ 1 ] == 1 );
   DOCTEST_CHECK( OR[ 2 ] == 0 );
   DOCTEST_CHECK( OR[ 3 ] == 3 );
   DOCTEST_CHECK( OR[ 4 ] == 4 );
   DOCTEST_CHECK( OR[ 5 ] == 5 );
   DOCTEST_CHECK( OR[ 6 ] == 0 );
   DOCTEST_CHECK( OR[ 7 ] == 7 );
   DOCTEST_CHECK( OR[ 8 ] == 0 );

   auto XOR = labels ^ moreLabels;
   DOCTEST_CHECK( XOR[ 1 ] == 1 );
   DOCTEST_CHECK( XOR[ 2 ] == 0 );
   DOCTEST_CHECK( XOR[ 3 ] == 3 );
   DOCTEST_CHECK( XOR[ 4 ] == 4 );
   DOCTEST_CHECK( XOR[ 5 ] == 0 );
   DOCTEST_CHECK( XOR[ 6 ] == 0 );
   DOCTEST_CHECK( XOR[ 7 ] == 7 );
   DOCTEST_CHECK( XOR[ 8 ] == 0 );

   auto NOT = ~labels;
   DOCTEST_CHECK( NOT[ 1 ] == 0 );
   DOCTEST_CHECK( NOT[ 2 ] == 2 );
   DOCTEST_CHECK( NOT[ 3 ] == 0 );
   DOCTEST_CHECK( NOT[ 4 ] == 4 );
   DOCTEST_CHECK( NOT[ 5 ] == 0 );
   DOCTEST_CHECK( NOT[ 6 ] == 6 );

   labels.Negate();
   DOCTEST_CHECK( labels[ 1 ] == 0 );
   DOCTEST_CHECK( labels[ 2 ] == 2 );
   DOCTEST_CHECK( labels[ 3 ] == 0 );
   DOCTEST_CHECK( labels[ 4 ] == 4 );
   DOCTEST_CHECK( labels[ 5 ] == 0 );
   DOCTEST_CHECK( labels[ 6 ] == 6 );

   labels.Relabel();
   DOCTEST_CHECK( labels[ 1 ] == 0 );
   DOCTEST_CHECK( labels[ 2 ] == 1 );
   DOCTEST_CHECK( labels[ 3 ] == 0 );
   DOCTEST_CHECK( labels[ 4 ] == 2 );
   DOCTEST_CHECK( labels[ 5 ] == 0 );
   DOCTEST_CHECK( labels[ 6 ] == 3 );

   DOCTEST_CHECK( labels.Contains( 1 ));
   DOCTEST_CHECK( labels.Contains( 6 ));
   DOCTEST_CHECK( !labels.Contains( 7 ));
   DOCTEST_CHECK( labels[ 7 ] == 7 );
   DOCTEST_CHECK( labels.Contains( 7 ));
   auto const& ref = labels;
   DOCTEST_CHECK( ref[ 9 ] == 9 );
   labels.DestroyUnknownLabels();
   DOCTEST_CHECK( !labels.Contains( 8 ));
   DOCTEST_CHECK( labels[ 8 ] == 0 );
   DOCTEST_CHECK( labels.Contains( 8 ));
   auto const& ref2 = labels;
   DOCTEST_CHECK( ref2[ 9 ] == 0 );
}

DOCTEST_TEST_CASE("[DIPlib] testing dip::LabelMap::Apply for images") {
   dip::Image labels( { 10 }, 1, dip::DT_UINT8 );
   dip::uint8* ptr = static_cast< dip::uint8* >( labels.Origin() );
   for( dip::uint8 ii = 0; ii < 10; ++ii, ++ptr ) {
      *ptr = ii;
   }
   auto list = dip::ListObjectLabels( labels );
   dip::LabelMap map( list );
   map[ 1 ] = 0;
   map.Relabel();
   dip::Image modified = map.Apply( labels );
   DOCTEST_CHECK( modified.DataType() == dip::DT_LABEL );
   labels -= 1;  // Note that 0 maps to 0 here
   DOCTEST_CHECK( dip::testing::CompareImages( modified, labels ));

   // Idem, but `list2` is a vector of dip::uint instead of LabelType
   auto list2 = std::vector< dip::uint >{ 1, 2, 3, 4, 5, 6, 7, 8 };
   dip::LabelMap map2( list2 );
   map2[ 1 ] = 0;
   map2.Relabel();
   dip::Image modified2 = map2.Apply( labels );
   DOCTEST_CHECK( modified2.DataType() == dip::DT_LABEL );
   labels -= 1;  // Note that 0 maps to 0 here
   DOCTEST_CHECK( dip::testing::CompareImages( modified2, labels ));
}

#endif // DIP_CONFIG_ENABLE_DOCTEST
