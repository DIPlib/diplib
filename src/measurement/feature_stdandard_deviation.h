/*
 * DIPlib 3.0
 * This file defines the "StandardDeviation" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureStandardDeviation : public LineBased {
   public:
      FeatureStandardDeviation() : LineBased( { "StandardDeviation", "Standard deviation of object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects );
         ValueInformationArray out( 1 );
         out[ 0 ].name = String( "StandardDeviation" );
         return out;
      }

      virtual void ScanLine(
            LineIterator <uint32> label,
            LineIterator <dfloat> grey,
            UnsignedArray /*coordinates*/,
            dip::uint /*dimension*/,
            ObjectIdToIndexMap const& objectIndices
      ) override {
         // If new objectID is equal to previous one, we don't to fetch the data pointer again
         uint32 objectID = 0;
         VarianceAccumulator* data = nullptr;
         do {
            if( *label > 0 ) {
               if( *label != objectID ) {
                  objectID = *label;
                  auto it = objectIndices.find( objectID );
                  if( it == objectIndices.end() ) {
                     data = nullptr;
                  } else {
                     data = &( data_[ it->second ] );
                  }
               }
               if( data ) {
                  data->Push( *grey );
               }
            }
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         VarianceAccumulator data = data_[ objectIndex ];
         *output = data.StandardDeviation();
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dip::uint nD_;
      std::vector< VarianceAccumulator > data_;
};


} // namespace feature
} // namespace dip
