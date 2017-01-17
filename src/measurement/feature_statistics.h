/*
 * DIPlib 3.0
 * This file defines the "Statistics" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <cmath>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureStatistics : public LineBased {
   public:
      FeatureStatistics() : LineBased( { "Statistics", "Mean, standard deviation, skewness and excess kurtosis of object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects );
         ValueInformationArray out( 4 );
         out[ 0 ].name = String( "Mean" );
         out[ 1 ].name = String( "StandardDeviation" );
         out[ 2 ].name = String( "Skewness" );
         out[ 3 ].name = String( "ExcessKurtosis" );
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
         StatisticsAccumulator* data = nullptr;
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
         StatisticsAccumulator data = data_[ objectIndex ];
         output[ 0 ] = data.Mean();
         output[ 1 ] = data.StandardDeviation();
         output[ 2 ] = data.Skewness();
         output[ 3 ] = data.ExcessKurtosis();
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      dip::uint nD_;
      std::vector< StatisticsAccumulator > data_;
};


} // namespace feature
} // namespace dip
