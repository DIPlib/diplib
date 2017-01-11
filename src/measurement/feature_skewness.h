/*
 * DIPlib 3.0
 * This file defines the "Skewness" measurement feature
 *
 * (c)2016-2017, Cris Luengo.
 * Based on original DIPlib code: (c)1995-2014, Delft University of Technology.
 */


#include <array>

#include "diplib.h"
#include "diplib/measurement.h"


namespace dip {
namespace Feature {


class FeatureSkewness : public LineBased {
   public:
      FeatureSkewness() : LineBased( { "Skewness", "Skewness (gamma_1) of object intensity", true } ) {};

      virtual ValueInformationArray Initialize( Image const& label, Image const& grey, dip::uint nObjects ) override {
         DIP_THROW_IF( !grey.IsScalar(), E::NOT_SCALAR );
         nD_ = label.Dimensionality();
         data_.clear();
         data_.resize( nObjects );
         ValueInformationArray out( 1 );
         out[ 0 ].name = String( "Skewness" );
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
                  dfloat gsq = *grey * *grey;
                  data->sum2 += gsq;
                  data->sum3 += gsq * *grey;
                  ++( data->number );
               }
            }
            ++grey;
         } while( ++label );
      }

      virtual void Finish( dip::uint objectIndex, Measurement::ValueIterator output ) override {
         Data data = data_[ objectIndex ];
         if( data.number > 1 ) {
            dfloat h1 = data.sum / data.number;
            dfloat h2 = data.sum2 / data.number;
            dfloat h3 = data.sum3 / data.number;
            dfloat m2 = h2 - h1 * h1;
            dfloat m3 = h3 - 3 * h1 * h2 + 2 * h1 * h1 * h1;
            *output = ( m2 != 0.0 ) ? ( m3 / std::pow( m2, 3.0 / 2.0 )) : ( 0.0 );
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
         dfloat sum3 = 0;
         dip::uint number = 0;
      };
      dip::uint nD_;
      std::vector< Data > data_;
};


} // namespace feature
} // namespace dip
