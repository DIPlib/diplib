/*
 * DIPlib 3.0
 * This file defines the "StdDev" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureStdDev : public LineBased {
   public:
      FeatureStdDev() : LineBased( { "StdDev", "Standard deviation of object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects );
         ValueInformationArray out( 1 );
         out[ 0 ].name = String( "StdDev" );
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
         Data* data = nullptr;
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
                  data->sum += *grey;
                  data->sum2 += *grey * *grey;
                  ++( data->number );
               }
            }
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         Data data = data_[ objectIndex ];
         if( data.number > 1 ) {
            dfloat stddev = ( data.sum2 - (( data.sum * data.sum ) / data.number )) / ( data.number - 1 );
            *output = std::sqrt( std::max( 0.0, stddev ));
         } else {
            *output = 0;
         }
      }

      virtual void Cleanup() override {
         data_.clear();
         data_.shrink_to_fit();
      }

   private:
      struct Data {
         dfloat sum = 0;
         dfloat sum2 = 0;
         dip::uint number = 0;
      };
      dip::uint nD_;
      std::vector< Data > data_;
};


} // namespace feature
} // namespace dip
